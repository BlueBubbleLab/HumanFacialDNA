#include <iostream>
#include <iomanip>
#include <fstream>
#include <time.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "../CrowdSight/src/crowdsight.h"
#include "demo_version.h"

#ifndef HUMAN_NAME
#define HUMAN_NAME "CrowdSight Demo"
#endif

#define RECORDING 0

int main ( int argc, char *argv[] )
{
  cv::VideoCapture cap;
  std::string auth_key;
  std::string file_name;
  std::string input;
  std::string data_dir = "../data/";
  bool is_webcam_input = false;

  switch ( argc )
  {
  case 4:
  {
    file_name = argv[1];
    data_dir  = argv[2];
    auth_key  = argv[3];
    input     = argv[1];
    cap.open(file_name);
    break;
  }
  case 5:
  {
    file_name = argv[1];
    if ( file_name.find( "--capture" ) != std::string::npos )
    {
      cap.open( atoi( argv[2] ) );
      is_webcam_input = true;
      input = argv[2];
    }
    data_dir = argv[3];
    auth_key = argv[4];
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

  if ( !cap.isOpened() )  // check if we can capture from frame
  {
    std::cerr
        << "Couldn't capture video from input "
        << input
        << std::endl;
    return -1;
  }

  //Create crowdsight instance
  CrowdSight crowdsight( data_dir );

  //Authenticate crowdsight instance
  if ( !crowdsight.authenticate( auth_key ) )
  {
    std::cerr << crowdsight.getErrorDescription() << std::endl;
    return -1;
  }

  //Setup video frame resolution
  cap.set( CV_CAP_PROP_FRAME_WIDTH,  640 );
  cap.set( CV_CAP_PROP_FRAME_HEIGHT, 480 );

  //Create an OpenCV output display window
  cv::namedWindow( HUMAN_NAME );

  cv::Mat frame;

  static CvScalar colors[] =
  {
    {{0,   0,   255}},
    {{0,   128, 255}},
    {{0,   255, 255}},
    {{0,   255, 0  }},
    {{255, 128, 0  }},
    {{255, 255, 0  }},
    {{255, 0,   0  }},
    {{255, 0,   255}}
  };

  //If desirable skips of a video can be skipped.
  int processEveryNthFrame = 1;
  // keep track of how many frames are processed
  int frameCount = 0;

  cap >> frame;

  // Define the region of interest here
  int marginTop     = 0;
  int marginBottom  = 0;
  int marginLeft    = 0;
  int marginRight   = 0;
  cv::Rect roi;

  //Start main processing loop
  while( true )
  {
    frameCount++;

    //Grab a frame
    cap >> frame;

    //If frame is empty break
    if ( frame.empty() )
    {
      cap.open(file_name);
      cap >> frame;
    }

    //Flip frame if capturing from webcam
    if ( is_webcam_input )
    {
      cv::flip(frame, frame, 1);
    }

    //specify the roi that will be extracted from the frame
    roi.x      = marginLeft;                    // pixels to skip from the left
    roi.y      = marginTop;                     // pixels to skip from the top
    roi.width  = frame.cols-roi.x-marginRight;  // width of roi
    roi.height = frame.rows-roi.y-marginBottom; // height of roi

    //Use CrowdSight instance to proces current frame.
    //Process function evaluates the frames contents and
    //must be called before getCurrentPeople();
    if (frameCount % processEveryNthFrame == 0 && frameCount>0)
    {
      if (!crowdsight.process(frame, roi))
      {
        std::cerr << crowdsight.getErrorDescription() << std::endl;
      }
    }

    //Get the list of people in the current frame
    std::vector<Person> people;
    if (!crowdsight.getCurrentPeople(people))
    {
      std::cerr << crowdsight.getErrorDescription() << std::endl;
    }

    //For each person in the frame, do:
    for (unsigned int i=0; i<people.size();i++)
    {
      std::ostringstream id_string;
      std::ostringstream attention_string;

      //Get person at current index
      Person person = people.at(i);

      //Retrieve the person's face
      cv::Rect face = person.getFaceRect();

      //Retrieve left and right eye locations of the person. Eye location is relative to the face rectangle.
      cv::Point right_eye = person.getRightEye();
      cv::Point left_eye  = person.getLeftEye();

      //Offset eye position with face position, to get frame coordinates.
      right_eye.x += face.x;
      right_eye.y += face.y;
      left_eye.x  += face.x;
      left_eye.y  += face.y;

      // Drawing positions for the components we want to display
      int moodPos       = -11;
      int genderPos     = 1;
      int agePos        = 12;
      int idPos         = face.y + 10;
      int attentionPos  = face.y + face.height-3;

      //Draw circles in the center of left and right eyes
      cv::circle(frame, right_eye, 3, cv::Scalar(0,255,0));
      cv::circle(frame, left_eye,  3, cv::Scalar(0,255,0));

      //Get the features of a person
      int   id       = person.getID();
      float ageValue = person.getAge();
      float moodValue = (person.getMood()+1.)/2.;
      //Get person's attention span. This value is returned in milliseconds
      int64_t attentionSpan = person.getAttentionSpan();

      id_string        << "ID #"        << id << "/" << person.getPredatorID();
      // Display attention span in minutes:seconds
      int minutes = (attentionSpan / 60000 );
      int seconds = ( attentionSpan / 1000 ) % 60;
      attention_string << minutes;
      if (seconds < 10) { attention_string << ":0" << seconds; }
      else              { attention_string << ":"  << seconds; }

      cv::putText( frame, id_string.str(),        cv::Point( face.x+3, idPos),        cv::FONT_HERSHEY_SIMPLEX, 0.35, colors[id % 8] );
      cv::putText( frame, attention_string.str(), cv::Point( face.x+3, attentionPos), cv::FONT_HERSHEY_SIMPLEX, 0.35, cv::Scalar(255, 255, 255) );

      // SHOW MOOD BAR
      cv::Rect moodBorder = cv::Rect( face.x, face.y + moodPos, face.width, 10 );
      cv::Rect moodRed    = cv::Rect( face.x + floor(moodValue*face.width) + 1, face.y + moodPos + 1, face.width - floor(moodValue*face.width) - 1, 8 );
      cv::Rect moodGreen  = cv::Rect( face.x + 1, face.y + moodPos + 1, moodValue * face.width -1, 8 );

      cv::rectangle( frame, moodRed   , cv::Scalar(0,0,200)    , CV_FILLED );
      cv::rectangle( frame, moodGreen , cv::Scalar(0,200,0)    , CV_FILLED );
      cv::rectangle( frame, moodBorder, cv::Scalar(255,255,255), 1 );

      cv::putText( frame, "MOOD", cv::Point( face.x+3, face.y + moodPos + 7 ), cv::FONT_HERSHEY_PLAIN, 0.5, cv::Scalar(255, 255, 255) );

      // SHOW GENDER BAR
      float genderValue = ( person.getGender() + 1 ) / 2.;

      cv::Rect genderBorder = cv::Rect( face.x, face.y + face.height + genderPos, face.width, 10 );
      cv::Rect genderPink   = cv::Rect( face.x + floor((1-genderValue)*face.width) + 1, face.y + face.height + genderPos + 1, floor(genderValue*face.width) - 1, 8 );
      cv::Rect genderBlue   = cv::Rect( face.x + 1, face.y + face.height + genderPos + 1, (1-genderValue) * face.width -1, 8 );

      cv::rectangle( frame, genderBlue,cv::Scalar(230,54,0), CV_FILLED);
      cv::rectangle( frame, genderPink, cv::Scalar(147,20,255), CV_FILLED );
      cv::rectangle( frame, genderBorder,cv::Scalar(255,255,255), 1 );

      cv::putText( frame, "GENDER", cv::Point( face.x+3, face.y+face.height+genderPos+7 ), cv::FONT_HERSHEY_PLAIN, 0.5, cv::Scalar(255, 255, 255));

      // SHOW AGE BAR
      if (ageValue > 80) {ageValue=80;}
      double agePerc = ((double)ageValue/80.0);
      int ageLocInBar = (int)(face.width*agePerc);
      int ageBlock = (int)face.width/4;

      cv::Rect ageBorder     = cv::Rect( face.x                   , face.y+face.height+agePos  , round(face.width/2) + round(face.width/2), 10 );
      cv::Rect ageGroupA     = cv::Rect( face.x + 1               , face.y+face.height+agePos+1, ageBlock, 8 );
      cv::Rect ageGroupB     = cv::Rect( face.x + 1 +    ageBlock , face.y+face.height+agePos+1, ageBlock, 8 );
      cv::Rect ageGroupC     = cv::Rect( face.x + 1 + (2*ageBlock), face.y+face.height+agePos+1, ageBlock, 8 );
      cv::Rect ageGroupD     = cv::Rect( face.x + 1 + (3*ageBlock), face.y+face.height+agePos+1, ageBlock, 8 );
      cv::Rect ageIndicatorB = cv::Rect( face.x + 1 + ageLocInBar - 1, face.y+face.height+agePos+1, 4, 8 );
      cv::Rect ageIndicatorW = cv::Rect( face.x + 1 + ageLocInBar    , face.y+face.height+agePos+1, 2, 8 );

      cv::rectangle( frame, ageGroupA    , cv::Scalar(255,130,92)  , CV_FILLED );
      cv::rectangle( frame, ageGroupB    , cv::Scalar(255,104,57)  , CV_FILLED );
      cv::rectangle( frame, ageGroupC    , cv::Scalar(230,54,0)    , CV_FILLED );
      cv::rectangle( frame, ageGroupD    , cv::Scalar(99,23,0)     , CV_FILLED );
      cv::rectangle( frame, ageIndicatorB, cv::Scalar(0,0,0)       , CV_FILLED );
      cv::rectangle( frame, ageIndicatorW, cv::Scalar(255,255,255) , CV_FILLED );
      cv::rectangle( frame, ageBorder    , cv::Scalar(255,255,255) , 1 );

      cv::putText( frame, "AGE", cv::Point(face.x+3, face.y+face.height+agePos+7 ), cv::FONT_HERSHEY_PLAIN, 0.5, cv::Scalar(255, 255, 255) );

      // Draw a rectangle around person's face on the current frame
      cv::rectangle(frame, face, cv::Scalar(255,255,255), 1);

      // visualize head pose
      // for this purpose, yaw and pitch are normalized in [-1...1] by HeadPose
      float yawValue = 1 - ((person.getHeadYaw()+1.)/2.);
      float pitchValue = (person.getHeadPitch()+1.)/2.;
      cv::line( frame, cv::Point(face.x+face.width/2,face.y+face.height/2), cv::Point(face.x+yawValue*face.width,face.y+pitchValue*face.height), cv::Scalar(255,255,255), 2 );

    }

    std::ostringstream peopleCounter_string;
    std::ostringstream peopleCounterNR_string;
    peopleCounter_string   << "People counter: ";
    peopleCounterNR_string << crowdsight.getPeopleCount();

    cv::putText( frame, peopleCounter_string.str(),   cv::Point(3, 13),  cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0,0,0), 2 );
    cv::putText( frame, peopleCounterNR_string.str(), cv::Point(50, 40), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0,0,0), 2 );

    cv::rectangle( frame, roi, cv::Scalar(255,255,255), 1 );
    cv::imshow( HUMAN_NAME, frame );
    //Show processed frame

    //Press 'q' to quit the program
    char key = cv::waitKey(1);
    if (key == 'q')
    {
      break;
    }
  }
  return 0;
}
