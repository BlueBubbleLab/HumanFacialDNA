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

#define W 12
#define H 9
#define BORDER 2
#define HARDCODED_PARAMS 1
#define CAMVIEW_WIDTH 320
#define CAMVIEW_HEIGHT 240
#define NUM_CALIBRATION_POINTS 5

enum FSM { ERASE=0, DRAW=1, IDLE=2, ADD=3 };

int main (int argc, char *argv[])
{
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
    int calibration_points = 0;
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
                if (calibration_points < NUM_CALIBRATION_POINTS)
                {
                    switch (state)
                    {
                    case ERASE: {
                        cv::circle(view, calib_point, 1, cv::Scalar(255,255,255), 2);
                        cv::circle(view, calib_point, 5, cv::Scalar(255,255,255), 2);
                        cv::circle(view, calib_point, 10, cv::Scalar(255,255,255), 2);
                        state = DRAW;
                    } break;
                    case DRAW: {
                        calib_point.x = (rand() % W) * width + 15 + rand() % int(width-30);
                        calib_point.y = (rand() % H) * height + 15 + rand() % int(height-30);
                        cv::circle(view, calib_point, 1, cv::Scalar(0,0,255), 2);
                        cv::circle(view, calib_point, 5, cv::Scalar(0,0,255), 2);
                        cv::circle(view, calib_point, 10, cv::Scalar(0,0,255), 2);
                        timer = time + 10;
                        state = IDLE;
                    } break;
                    case IDLE: {
                        if (time > timer)
                        {
                            state = ADD;
                            timer = time + 5;
                        }
                    } break;
                    case ADD: {
                        if (time < timer)
                            insight.addCalibrationPoint(calib_point);
                        else
                        {
                            state = ERASE;
                            calibration_points++;
                        }
                    } break;
                    default: return -1;
                    }
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
                        std::cout << "EYE_GAZE (" << eye_gaze.x << ", " << eye_gaze.y << ")" << std::endl;
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
    	{
            std::cout << "Wrote output to " << file_name << std::endl;
    	    break;
    	}
        time++;
    }
    return 0;
}
