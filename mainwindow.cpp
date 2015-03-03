#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCore>
#include <QDebug>
#include <ctime>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/contrib/contrib.hpp>
#include <opencv2/objdetect/objdetect.hpp>

/** Global variables **/
std::string faceCascadeName = "/home/josh/projects/uWho/lbpcascade_frontalface.xml";
std::string eyesCascadeName = "/home/josh/projects/uWho/haarcascade_eye_tree_eyeglasses.xml";
cv::CascadeClassifier faceCascade;
cv::CascadeClassifier eyesCascade;
std::string face_file = "/home/josh/projects/uWho/face.xml";
QFile face("/home/josh/projects/uWho/face.xml");
/** end of global variables **/

using namespace std;
using namespace cv;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QPixmap webcam("/home/josh/projects/uWho/webcam.png");
    QPixmap videofile("/home/josh/projects/uWho/videofile.png");
    ui->webcamButton->setIcon(webcam);
    ui->videofileButton->setIcon(videofile);


}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_videofileButton_clicked()
{

}

void MainWindow::on_webcamButton_clicked()
{
    std::srand(std::time(NULL));
    Ptr<cv::FaceRecognizer> model = cv::createLBPHFaceRecognizer(1,8,8,8, 100);
    if (face.exists()){
        model->load(face_file);
        qDebug() << "Loaded model." ;
    }else{
        qDebug() << "Generating starting model..." ;
        vector<cv::Mat> images (10);
        vector<int> labels (10);
        images[0] = (imread("/home/josh/projects/uWho/startingfaces/josh1.png", CV_LOAD_IMAGE_GRAYSCALE));
        labels[0] = 0;
        images[1] = (imread("/home/josh/projects/uWho/startingfaces/josh2.png", CV_LOAD_IMAGE_GRAYSCALE));
        labels[1] = 0;
        images[2] = (imread("/home/josh/projects/uWho/startingfaces/josh3.png", CV_LOAD_IMAGE_GRAYSCALE));
        labels[2] = 0;
        images[3] = (imread("/home/josh/projects/uWho/startingfaces/josh4.png", CV_LOAD_IMAGE_GRAYSCALE));
        labels[3] = 0;
        images[4] = (imread("/home/josh/projects/uWho/startingfaces/josh5.png", CV_LOAD_IMAGE_GRAYSCALE));
        labels[4] = 0;
        images[5] = (imread("/home/josh/projects/uWho/startingfaces/josh6.png", CV_LOAD_IMAGE_GRAYSCALE));
        labels[5] = 0;
        images[6] = (imread("/home/josh/projects/uWho/startingfaces/josh7.png", CV_LOAD_IMAGE_GRAYSCALE));
        labels[6] = 0;
        images[7] = (imread("/home/josh/projects/uWho/startingfaces/josh8.png", CV_LOAD_IMAGE_GRAYSCALE));
        labels[7] = 0;
        images[8] = (imread("/home/josh/projects/uWho/startingfaces/josh9.png", CV_LOAD_IMAGE_GRAYSCALE));
        labels[8] = 0;
        images[9] = (imread("/home/josh/projects/uWho/startingfaces/josh10.png", CV_LOAD_IMAGE_GRAYSCALE));
        labels[9] = 0;
        model->train(images, labels);

        cv::Mat testingImage = (imread("/home/josh/projects/uWho/josh11.png", CV_LOAD_IMAGE_GRAYSCALE));
        int predicted = -1;  // Sanity check. We throw a face I know is mine to the predictor.
        double confidence ;
        model->predict(testingImage, predicted, confidence);
        qDebug() << "Testing predicted/confidence: " << predicted << confidence ;
    }

    cv::namedWindow("VidWindow");
    cv::VideoCapture cap = cv::VideoCapture(0);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 800 );
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 600 );
    cv::Mat frame;
    do{
        cap >> frame;
        if(!frame.empty()){
            cv::blur(frame, frame, cv::Size(3,3));
            faceCascade.load(faceCascadeName);
            eyesCascade.load(eyesCascadeName);
            std::vector<cv::Rect> faces;
            cv::Mat frame_gray;
            cv::cvtColor(frame, frame_gray, CV_BGR2GRAY);
            faceCascade.detectMultiScale(frame_gray, faces, 1.1, 3, CV_HAAR_SCALE_IMAGE, cv::Size(50,50));
            for(int i = 0; i < faces.size(); i++){
                std::vector<cv::Mat> facePicture (1);
                std::vector<int> faceIndex (1);
                std::vector<Rect> eyes;
                facePicture[0] = frame(faces[i]);   // Gets the face only as the variable facePicture

                cv::rectangle(frame, faces[i], cv::Scalar(255,0,255), 1, 8, 0); // Draws rectangles on webcam video
                string faceString = static_cast<ostringstream*>( &(ostringstream() << i) )->str();
                cv::putText(frame, faceString, cv::Point(faces[i].x, (faces[i].y+30)),FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1,8, false);

                eyesCascade.detectMultiScale( facePicture[0], eyes, 1.1, 2, 0 |CV_HAAR_SCALE_IMAGE, Size(30, 30) );
                if (eyes.size() != 0){
                    cv::cvtColor(facePicture[0], facePicture[0], CV_BGR2GRAY); // colorspace change to gray
                    int predicted = -1;
                    double confidence ;
                    model->predict(facePicture[0], predicted, confidence); // Check the machine learner and ask if it's seen this face before
                    string predictString = static_cast<ostringstream*>( &(ostringstream() << predicted) )->str();
                    cv::putText(frame, predictString, cv::Point((faces[i].x + faces[i].width - 40), (faces[i].y + 30)),FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255), 1,8, false);
                    if (predicted == -1){
                        faceIndex[0] = std::rand()%30000;
                        model->update(facePicture,faceIndex); // If its not in the FaceRecognizer, add it
                    }else{
                        faceIndex[0] = predicted;
                        model->update(facePicture,faceIndex);  // if the face is already in, add this as another data point
                    }
                    qDebug() << "face # " << predicted << confidence;
                }
            }


            imshow("VidWindow" ,frame);}
    }while(cv::waitKey(30)<30);
    model->save(face_file);
    cv::destroyWindow("VidWindow");
}
