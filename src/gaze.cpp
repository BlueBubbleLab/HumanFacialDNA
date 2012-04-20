#include <iostream>
#include <iomanip>
#include <fstream>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "../Perseus/src/perseus.h"
#include "demo_version.h"

#ifndef HUMAN_NAME
    #define HUMAN_NAME "Perseus Demo"
#endif

int main (int argc, char *argv[])
{
  cv::VideoCapture cap;
  std::string auth_key;
  std::string file_name;
  std::string data_dir = "../data/";
  std::string input;
  bool is_webcam_input = false;

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

  if (!cap.isOpened())  // check if we can capture from frame
  {
    std::cerr
        << "Couldn't capture video from input "
        << input
        << std::endl;
    return -1;
  }

  //Create perseus instance
  Perseus perseus(data_dir);

  //Authenticate perseus instance
  if (!perseus.authenticate(auth_key))
  {
    std::cerr << perseus.getErrorDescription() << std::endl;
    return -1;
  }

  //Setup video frame resolution
  cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

  //Create an OpenCV output display window
  cv::namedWindow(HUMAN_NAME);

  cv::Mat frame;

  static CvScalar colors[] =
  {
    {{0, 0, 255}},
    {{0, 128, 255}},
    {{0, 255, 255}},
    {{0, 255, 0}},
    {{255, 128, 0}},
    {{255, 255, 0}},
    {{255, 0, 0}},
    {{255, 0, 255}}
  };

  //If desirable skips of a video can be skipped.
  int processEveryNthFrame = 3;
  // keep track of how many frames are processed
  int frameCount           = 0;

  //Start main processing loop
  while(true)
  {
    frameCount++;

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

    cv::Rect roi;
    //specify the roi that will be extracted from the frame
    roi.x      = 0;                // pixels to skip from the left
    roi.y      = 220;                // pixels to skip from the top
    roi.width  = frame.cols;       // width of roi
    roi.height = frame.rows-roi.y; // height of roi
    // extract roi from frame and continue with that part of the frame only
    frame = frame(roi);


    //Use perseus instance to procees current frame.
    //Process function evaluates the frames contents and
    //must be called before getCurrentPeople();

    if (frameCount % processEveryNthFrame == 0)
    {



        if (!perseus.process(frame))
        {
          std::cerr << perseus.getErrorDescription() << std::endl;
        }
    }

    //Get the list of people in the current frame
    std::vector<Person> people;
    if (!perseus.getCurrentPeople(people))
    {
      std::cerr << perseus.getErrorDescription() << std::endl;
    }

    //For each person in the frame, do:
    for (unsigned int i=0; i<people.size();i++)
    {
      //Get person at current index
      Person person = people.at(i);

      //Retrieve the person's face
      cv::Rect face = person.getFaceRect();

      //Retrieve left and right eye locations of the person.
      //Eye location is relative to the face rectangle.
      cv::Point right_eye = person.getRightEye();
      cv::Point left_eye  = person.getLeftEye();

      //Offset eye position with face position, to get frame coordinates.
      right_eye.x += face.x;
      right_eye.y += face.y;
      left_eye.x += face.x;
      left_eye.y += face.y;

      //Draw circles in the center of left and right eyes
      cv::circle(frame, right_eye, 3, cv::Scalar(0,255,0));
      cv::circle(frame, left_eye, 3, cv::Scalar(0,255,0));

      //Get person's ID and other features and draw it in the face rectangle
      std::ostringstream id_string;
      id_string << "ID #" << person.getID();
      std::ostringstream age_string;
      age_string << "Age: " << person.getAge();

      cv::putText(frame, id_string.str(), cv::Point(face.x+10, face.y+30),
                  cv::FONT_HERSHEY_SIMPLEX, 0.5, colors[person.getID()%8]);
      cv::putText(frame, age_string.str(), cv::Point(face.x+10, face.y+50),
                  cv::FONT_HERSHEY_SIMPLEX, 0.5, colors[person.getID()%8]);

      // SHOW MOOD BAR
      cv::Rect moodRed = cv::Rect(face.x+3,face.y+3,round(face.width/2),10);
      cv::rectangle(frame,moodRed,cv::Scalar(0,0,255),CV_FILLED);
      cv::Rect moodGreen = cv::Rect(face.x+round(face.width/2),face.y+3,round(face.width/2)-1,10);
      cv::rectangle(frame,moodGreen,cv::Scalar(0,255,0),CV_FILLED);

      cv::putText(frame, "MOOD", cv::Point(face.x+3, face.y+10),
                  cv::FONT_HERSHEY_SIMPLEX, 0.3, cv::Scalar(0,0,0));

      float moodValue = person.getMood();
      moodValue+=2; //make non negative
      if (moodValue<0.) moodValue=0.;
      if (moodValue>4.) moodValue=4.;
      moodValue /=4; // normalize between 0 and 1

      // SHOW GENDER BAR
      cv::Rect genderBlue = cv::Rect(face.x+3,face.y+face.height-13,round(face.width/2),10);
      cv::rectangle(frame,genderBlue,cv::Scalar(255,55,55),CV_FILLED);
      cv::Rect genderPink = cv::Rect(face.x+round(face.width/2),face.y+face.height-13,round(face.width/2)-1,10);
      cv::rectangle(frame,genderPink,cv::Scalar(147,20,255),CV_FILLED);

      cv::putText(frame, "GENDER", cv::Point(face.x+3, face.y+face.height-6),
                  cv::FONT_HERSHEY_SIMPLEX, 0.3, cv::Scalar(0,0,0));


      //Draw a rectangle around person's face on the current frame
      cv::rectangle(frame, face, colors[person.getID()%8], 3);


      cv::Rect theMood = cv::Rect(face.x+round(face.width*moodValue),face.y+3,3,10);
      cv::rectangle(frame,theMood,cv::Scalar(0,0,0),CV_FILLED);
      cv::Rect theMoodInnerWhite = cv::Rect(face.x+round(face.width*moodValue)+1,face.y+4,1,8);
      cv::rectangle(frame,theMoodInnerWhite,cv::Scalar(255,255,255),CV_FILLED);

      float genderValue = (person.getGender() + 1) / 2.;
      cv::Rect theGender = cv::Rect(face.x+round(face.width*genderValue),face.y+face.height-13,3,10);
      cv::rectangle(frame,theGender,cv::Scalar(0,0,0),CV_FILLED);
      cv::Rect theGenderInnerWhite = cv::Rect(face.x+round(face.width*genderValue)+1,face.y+face.height-12,1,8);
      cv::rectangle(frame,theGenderInnerWhite,cv::Scalar(255,255,255),CV_FILLED);

      // visualize head pose
      // for this purpose, yaw and pitch are normalized in [0...1] by HeadPose
      float yawValue = 1 - person.getYaw();
      float pitchValue = person.getPitch();
      cv::line(frame, cv::Point(face.x+face.width/2,face.y+face.height/2), cv::Point(face.x+yawValue*face.width,face.y+pitchValue*face.height), colors[person.getID()%8],2);
    }

    //Show processed frame

//    cv::Mat bigframe;
//    cv::resize(frame,bigframe,cv::Size(1280,1024));
//    cv::imshow(HUMAN_NAME, bigframe);

    cv::imshow(HUMAN_NAME, frame);

    //Press 'q' to quit the program
    char key = cv::waitKey(1);
    if (key == 'q')
    {
      break;
    }
  }
  return 0;
}
