#include <iostream>
#include <iomanip>
#include <fstream>
#include <time.h>
#include <stdlib.h>

#include <QSettings>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "../inSight/src/insight.h"
#include "demo_version.h"

#ifdef __APPLE__ 
//Needed to resolve data dir on osx
#include <CoreFoundation/CoreFoundation.h>
#endif

#define W 11
#define H 7
#define BORDER 2
#define HARDCODED_PARAMS 1
#define CAMVIEW_WIDTH 320
#define CAMVIEW_HEIGHT 240

enum FSM { ERASE=0, DRAW=1, IDLE=2, ADD=3 };

void drawPoint(cv::Mat& inCanvas, const cv::Point2i& inPos, const cv::Scalar& inColor)
{
    cv::circle(inCanvas, inPos, 2, inColor, 2);
    cv::circle(inCanvas, inPos, 7, inColor, 2);
    cv::circle(inCanvas, inPos, 12, inColor, 2);
}

int main (int argc, char *argv[])
{
    std::vector<cv::Point2i> calibration_points;
    calibration_points.push_back(cv::Point2i(0, 0));
    calibration_points.push_back(cv::Point2i(W/2, 0));
    calibration_points.push_back(cv::Point2i(W-1, 0));
    calibration_points.push_back(cv::Point2i(W-1, H/2));
    calibration_points.push_back(cv::Point2i(W-1, H-1));
    calibration_points.push_back(cv::Point2i(W/2, H-1));
    calibration_points.push_back(cv::Point2i(0, H-1));
    calibration_points.push_back(cv::Point2i(0, H/2));
    calibration_points.push_back(cv::Point2i(W/2, H/2));


    srand( time(NULL) );
    cv::VideoCapture cap;
    std::string auth_key;
    std::string file_name;
    std::string data_dir = "../data/";
    std::string input;
    bool is_capturing = false;
    switch (argc)
    {
        case 4:
        {
            file_name = argv[1];
            data_dir  = argv[2];
            auth_key = argv[3];
            input = argv[1];
            cap.open(file_name);
            break;
        }
        case 5:
        {
            file_name = argv[1];
            if (file_name.find("--capture") != std::string::npos)
            {
                cap.open(atoi(argv[2]));
                is_capturing = true;
                file_name = std::string("capture-") + argv[2] + ".csv";
                input = argv[2];
            }
            data_dir = argv[3];
            auth_key = argv[4];
            break;
        }
        default:
        {

#ifdef HARDCODED_PARAMS
        cap.open(1);
        is_capturing = true;
  #ifdef __APPLE__
        //get path to Resource directory in OSX app bundle
        CFBundleRef mainBundle;
        mainBundle = CFBundleGetMainBundle();
        CFURLRef resourceURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
        CFURLRef dataURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault,
                                 resourceURL,
                                   CFSTR("data"),
                                   true);
        char buffer[1024];
        if(CFURLGetFileSystemRepresentation(dataURL, true, (UInt8*)buffer, 1024)) {
            //TODO: fix trailing slash not being appended automatically
            data_dir = std::string(buffer) + "/";
        }
  #else
        //same place as executable
        data_dir = "./data/";
  #endif
        auth_key = "medusa123";
#else
         std::cout
         << "Usage: "
         << argv[0]
         << " <videofile> <data dir> <auth key>"
         << std::endl;
         std::cout
         << "       "
         << argv[0]
         << " --capture <camera-id> <data dir> <auth key>"
         << std::endl;
         return -1;
#endif
        }
    }

    if(!cap.isOpened())  // check if we can capture from camera
    {
    	std::cerr 
            << "Couldn't capture video from input "
            << input
            << std::endl;
    	return -1;
    }
    
    QSettings *settings = InSight::settingsInstance();
    cap.set(CV_CAP_PROP_FRAME_WIDTH,
            settings->value("camera/reswidth").toInt());
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,
            settings->value("camera/resheight").toInt());
    
    InSight insight;
    if(!insight.authenticate(auth_key))
    {
    	std::cerr << insight.getError() << std::endl;
    	return -1;
    }
    
    cv::namedWindow(HUMAN_NAME, CV_WINDOW_NORMAL);
    cv::setWindowProperty(HUMAN_NAME, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
    // Initialize gaze-grid
    cv::Mat camera, view, temp;
    int pixel_width = settings->value("screen/reswidth").toInt(),
        pixel_height = settings->value("screen/resheight").toInt();
    view.create(cv::Size(pixel_width, pixel_height), CV_8UC3);
    view.setTo(cv::Scalar(0,0,0));
    cv::Rect roi(pixel_width-CAMVIEW_WIDTH,0,CAMVIEW_WIDTH,CAMVIEW_HEIGHT);
    std::vector<cv::Rect> sections(W*H);
    float width = pixel_width/float(W);
    float height = pixel_height/float(H);
    for (int i = 0; i < H; i++)
    {
        for (int j = 0; j < W; j++)
        {
            sections[i*W+j] =
                    cv::Rect(j*width+BORDER,i*height+BORDER,width-BORDER,height-BORDER);
            temp = view(sections[i*W+j]);
            temp.setTo(cv::Scalar(255,255,255));
        }
    }
    cv::Point2i prev_head_gaze(0);
    cv::Point2i prev_eye_gaze(0);
    int time = 0;
    int timer = 0;
    unsigned int num_point = 0;
    FSM state = ERASE;
    cv::Point2i calib_point(0);
    // Start indefinite loop and track eye gaze
    for(;;)
    {
        cap >> camera;
        if (is_capturing)
        {
            cv::flip(camera, camera, 1);
            cv::imwrite("test.jpg", camera);
        }

    	if (!insight.isInit())
    	{
            if(!insight.init(camera, data_dir))
            {
                std::cerr << insight.getError() << std::endl;
            }
            cvMoveWindow(HUMAN_NAME,0,0);
    	}
    	else
    	{
            if(!insight.process(camera))
            {
                std::cerr << insight.getError() << std::endl;
            }
            else
            {
                if (num_point < calibration_points.size())
                {
                    switch (state)
                    {
                    case ERASE: {
                        drawPoint(view, calib_point, cv::Scalar(255,255,255));
                        state = DRAW;
                    } break;
                    case DRAW: {
                        calib_point.x = calibration_points[num_point].x * width + width/2;
                        calib_point.y = calibration_points[num_point].y * height + height/2;
                        drawPoint(view, calib_point, cv::Scalar(0,0,255));
                        timer = time + 10;
                        state = IDLE;
                    } break;
                    case IDLE: {
                        if (time > timer)
                        {
                            state = ADD;
                            timer = time + 2;
                        }
                    } break;
                    case ADD: {
                        if (time < timer)
                            insight.addCalibrationPoint(calib_point);
                        else
                        {
                            state = ERASE;
                            num_point++;
                        }
                    } break;
                    default: return -1;
                    }
                    if (num_point == calibration_points.size())
                        insight.calibrate();
                }
                else
                {
                    cv::Point2i head_gaze, eye_gaze;
                    if (insight.getHeadGaze(head_gaze))
                    {
                        head_gaze = cv::Point2i(head_gaze.x / width, head_gaze.y / height);
                        if (prev_head_gaze != head_gaze)
                        {
                            temp = view(sections[prev_head_gaze.y*W+prev_head_gaze.x]);
                            temp.setTo(cv::Scalar(255,255,255));
                        }
                        temp = view(sections.at(head_gaze.y*W+head_gaze.x));
                        temp.setTo(cv::Scalar(255,0,0));
                        prev_head_gaze = head_gaze;
                    }
                    if (insight.getEyeGaze(eye_gaze))
                    {
                        eye_gaze = cv::Point2i(eye_gaze.x / width, eye_gaze.y / height);
                        if (prev_eye_gaze != eye_gaze && prev_eye_gaze != head_gaze)
                        {
                            temp = view(sections[prev_eye_gaze.y*W+prev_eye_gaze.x]);
                            temp.setTo(cv::Scalar(255,255,255));
                        }

                        temp = view(sections.at(eye_gaze.y*W+eye_gaze.x));
                        cv::Scalar color(0,0,255);
                        if (eye_gaze == head_gaze)
                        {
                            color = cv::Scalar(128,0,128);
                        }
                        temp.setTo(color);
                        prev_eye_gaze = eye_gaze;
                    }
                    insight.drawWireFace(camera);
                    cv::resize(camera, temp, cv::Size(CAMVIEW_WIDTH,CAMVIEW_HEIGHT));
                    camera = view(roi);
                    temp.copyTo(camera);
                }
            }
    	}
        imshow(HUMAN_NAME, view);
        char key = cv::waitKey(1);
        if(key == 'q')
    	    break;
        time++;
    }
    return 0;
}
