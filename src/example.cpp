#include <iostream>
#include <iomanip>
#include <fstream>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "insight.h"

#ifndef HUMAN_NAME
    #define HUMAN_NAME "inSight Demo"
#endif

int main(int argc, char *argv[])
{
  cv::VideoCapture cap;
  std::string auth_key;
  std::string file_name;
  std::string data_dir = "../data/";
  std::string input;
  bool is_webcam_input = false;

  switch( 5 )
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
    file_name = "--capture"; //argv[1];
    if (file_name.find("--capture") != std::string::npos)
    {
      cap.open( 0 /*atoi(argv[2])*/ );
      is_webcam_input = true;
      input = "0"; //argv[2];
    }
    data_dir = "./data/"; //argv[3];
    auth_key = "123"; //argv[4];
    break;
  }
  default:
  {
    std::cout
        << "Usage: "
        << argv[0]
        << " <videofile> <data dir> <auth key>"
        << std::endl;
    std::cout
        << "       "
        << argv[0]
        << " --capture <frame-id> <data dir> <auth key>"
        << std::endl;
    return -1;
  }
  }

  if (!cap.isOpened())  // check if we can capture from frame
  {
    std::cerr
        << "Couldn't capture video from input "
        << input
        << std::endl;
    return -1;
  }

  //Create an inSight instance
  InSight insight(data_dir);

  cv::Mat background = cv::imread("./resources/demo_dashboard.png");

  if (!insight.authenticate(auth_key))
  {
    std::cerr << insight.getError() << std::endl;
    return -1;
  }

  cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

  //Create an OpenCV output display window
  cv::namedWindow( HUMAN_NAME );
  cv::moveWindow( HUMAN_NAME, 100, 100 );
  cv::Mat camera_roi = background( cv::Rect(10,10,640,480) );
  cv::Rect roi;

  cv::Mat frame;

  cv::Rect neutralBkgr   = cv::Rect( 800,  80, 180, 20 );
  cv::Rect happyBkgr     = cv::Rect( 800, 105, 180, 20 );
  cv::Rect surprisedBkgr = cv::Rect( 800, 130, 180, 20 );
  cv::Rect angryBkgr     = cv::Rect( 800, 155, 180, 20 );
  cv::Rect disgustedBkgr = cv::Rect( 800, 180, 180, 20 );
  cv::Rect afraidBkgr    = cv::Rect( 800, 205, 180, 20 );
  cv::Rect sadBkgr       = cv::Rect( 800, 230, 180, 20 );

  cv::Rect neutralCurrent   = cv::Rect( 800,  80, 0, 20 );
  cv::Rect happyCurrent     = cv::Rect( 800, 105, 0, 20 );
  cv::Rect surprisedCurrent = cv::Rect( 800, 130, 0, 20 );
  cv::Rect angryCurrent     = cv::Rect( 800, 155, 0, 20 );
  cv::Rect disgustedCurrent = cv::Rect( 800, 180, 0, 20 );
  cv::Rect afraidCurrent    = cv::Rect( 800, 205, 0, 20 );
  cv::Rect sadCurrent       = cv::Rect( 800, 230, 0, 20 );

  cv::Scalar bkgrColor      = cv::Scalar( 20, 20, 20 );

  cv::Scalar neutralColor   = cv::Scalar( 127, 127, 127 );
  cv::Scalar happyColor     = cv::Scalar( 0, 255, 0 );
  cv::Scalar surprisedColor = cv::Scalar( 255, 0, 0 );
  cv::Scalar angryColor     = cv::Scalar( 0, 0, 255 );
  cv::Scalar disgustedColor = cv::Scalar( 0, 255, 255 );
  cv::Scalar afraidColor    = cv::Scalar( 0, 0, 127 );
  cv::Scalar sadColor       = cv::Scalar( 200, 90, 200 );


  std::vector<float> outHeadpose;
  cv::Point  rotationCenterX   = cv::Point( 825, 305 );
  cv::Point  rotationCenterY   = cv::Point( 890, 305 );
  cv::Point  rotationCenterZ   = cv::Point( 955, 305 );
  cv::Size   rotationSize      = cv::Size( 20, 20 );
  cv::Rect   rotationXBkgr     = cv::Rect( 800, 280, 50, 50 );
  cv::Rect   rotationYBkgr     = cv::Rect( 865, 280, 50, 50 );
  cv::Rect   rotationZBkgr     = cv::Rect( 930, 280, 50, 50 );
  cv::Scalar rotationBkgrColor = cv::Scalar( 40, 40, 40 );

  cv::Point  headGaze;
  cv::Point  projectedHeadGaze;
  cv::Point  headGazeBkgrOffset = cv::Point( 800, 340 );
  const int  screenWidth        = InSight::getScreenWidthRes();
  const int  screenHeight       = InSight::getScreenHeightRes();
  const int  projectedWidth     = 180;
  const int  projectedHeight    = ( 180 * screenHeight / screenWidth );
  cv::Rect   headGazeBkgr       = cv::Rect( headGazeBkgrOffset.x, headGazeBkgrOffset.y, projectedWidth, projectedHeight );
  cv::Scalar headGazeColor      = cv::Scalar( 0, 0, 255 );

  while(true)
  {
	//Grab a frame
    cap >> frame;

    //If frame is empty break
    if (frame.empty())
    {
      break;
    }

	//Flip frame if capturing from webcam
    if (is_webcam_input)
    {
      cv::flip(frame, frame, 1);
    }

	//Initialize insight on first frame,
	//process the rest of the frames

    //////
    // Draw meters backgrounds
    cv::rectangle( background, neutralBkgr,   bkgrColor, CV_FILLED );
    cv::rectangle( background, happyBkgr,     bkgrColor, CV_FILLED );
    cv::rectangle( background, surprisedBkgr, bkgrColor, CV_FILLED );
    cv::rectangle( background, angryBkgr,     bkgrColor, CV_FILLED );
    cv::rectangle( background, disgustedBkgr, bkgrColor, CV_FILLED );
    cv::rectangle( background, afraidBkgr,    bkgrColor, CV_FILLED );
    cv::rectangle( background, sadBkgr,       bkgrColor, CV_FILLED );

    cv::rectangle( background, rotationXBkgr, bkgrColor, CV_FILLED );
    cv::rectangle( background, rotationYBkgr, bkgrColor, CV_FILLED );
    cv::rectangle( background, rotationZBkgr, bkgrColor, CV_FILLED );
    cv::circle( background, rotationCenterX, 20, rotationBkgrColor, 1, CV_AA );
    cv::circle( background, rotationCenterY, 20, rotationBkgrColor, 1, CV_AA );
    cv::circle( background, rotationCenterZ, 20, rotationBkgrColor, 1, CV_AA );

    cv::rectangle( background, headGazeBkgr, bkgrColor, CV_FILLED );

    char key = cv::waitKey( 1 );
    if( key == 27 || key == 'q' || !cvGetWindowHandle( HUMAN_NAME ) )
    {
      break;
    }
    // Init or process
    if (!insight.isInit())
    {
      if( key == ' ' )
      {
        if( !insight.init( frame ) )
        {
          std::cerr << insight.getError() << std::endl;
        }
      }
      else
      {
        cv::rectangle( frame, cv::Rect( 145, 10, 350, 70 ), bkgrColor, CV_FILLED );
        cv::putText( frame, "Place your face inside the red circle,", cv::Point( 155, 30 ), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar( 255, 255, 255 ), 1,  CV_AA );
        cv::putText( frame, "look toward the camera and press the",  cv::Point( 155, 50 ), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar( 255, 255, 255 ), 1,  CV_AA );
        cv::putText( frame, "spacebar to initialize",                cv::Point( 155, 70 ), cv::FONT_HERSHEY_DUPLEX, 0.5, cv::Scalar( 255, 255, 255 ), 1,  CV_AA );
        cv::ellipse( frame, cv::Point( 320, 240 ), cv::Size( 100, 75 ), -90.0, 00.0, 360.0, cv::Scalar( 0, 0, 255 ), 1, CV_AA );
      }
    }
    else
    {
      if (!insight.process(frame))
      {
        std::cerr << insight.getError() << std::endl;
      }

      std::vector<float> emotions;
      if (insight.getEmotions(emotions))
      {
        std::vector<cv::Point> points;
        insight.getMaskPoints(points);
        for (int i = 0, n = points.size(); i < n; i++)
        {
          cv::circle(frame, points[i], 1, cv::Scalar(0,255,0), 1, CV_AA );
        }

        //////
        // Draw emotions
        neutralCurrent.width   = emotions[0] * 180;
        happyCurrent.width     = emotions[1] * 180;
        surprisedCurrent.width = emotions[2] * 180;
        angryCurrent.width     = emotions[3] * 180;
        disgustedCurrent.width = emotions[4] * 180;
        afraidCurrent.width    = emotions[5] * 180;
        sadCurrent.width       = emotions[6] * 180;

        cv::rectangle( background, neutralCurrent,   neutralColor,   CV_FILLED );
        cv::rectangle( background, happyCurrent,     happyColor,     CV_FILLED );
        cv::rectangle( background, surprisedCurrent, surprisedColor, CV_FILLED );
        cv::rectangle( background, angryCurrent,     angryColor,     CV_FILLED );
        cv::rectangle( background, disgustedCurrent, disgustedColor, CV_FILLED );
        cv::rectangle( background, afraidCurrent,    afraidColor,    CV_FILLED );
        cv::rectangle( background, sadCurrent,       sadColor,       CV_FILLED );

        //////
        // Draw head pose
        if( insight.getHeadPose( outHeadpose ) )
        {
          cv::ellipse( background, rotationCenterX, rotationSize, -90.0, 00.0, (  360.0 * outHeadpose[3] ) / ( 2.0 * CV_PI ), cv::Scalar( 255, 255, 255 ), 1, CV_AA );
          cv::ellipse( background, rotationCenterY, rotationSize, -90.0, 00.0, (  360.0 * outHeadpose[4] ) / ( 2.0 * CV_PI ), cv::Scalar( 255, 255, 255 ), 1, CV_AA );
          cv::ellipse( background, rotationCenterZ, rotationSize, -90.0, 00.0, ( -360.0 * outHeadpose[5] ) / ( 2.0 * CV_PI ), cv::Scalar( 255, 255, 255 ), 1, CV_AA );
        }

        //////
        // Draw head gaze
        if( insight.getHeadGaze( headGaze ) )
        {
          projectedHeadGaze.x = ( headGaze.x * projectedWidth ) / screenWidth;
          projectedHeadGaze.y = ( headGaze.y * projectedHeight ) / screenHeight;
          // Clip by one pixel
          if( projectedHeadGaze.x <= 2 ) projectedHeadGaze.x += 2;
          if( projectedHeadGaze.x >= projectedWidth - 2 ) projectedHeadGaze.x -= 2;
          if( projectedHeadGaze.y <= 2 ) projectedHeadGaze.y += 2;
          if( projectedHeadGaze.y >= projectedHeight - 2 ) projectedHeadGaze.y -= 2;
          cv::circle( background, projectedHeadGaze + headGazeBkgrOffset, 2, headGazeColor );
        }
      }
      else
      {
        std::cerr << insight.getError() << std::endl;
      }
    }

    //Show processed frame
    cv::rectangle( frame, roi, cv::Scalar(255,255,255), 1 );
    cv::Mat smallframe;
    cv::resize(frame,smallframe,cv::Size(640, 480));
    smallframe.copyTo(camera_roi);
    cv::imshow( HUMAN_NAME, background );
  }
  return 0;
}
