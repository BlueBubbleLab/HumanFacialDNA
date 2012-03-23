#include <iostream>
#include <iomanip>
#include <fstream>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "../Perseus/src/perseus.h"


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
  cv::namedWindow("Perseus Example");

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

  //Start main processing loop
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

    //Use perseus instance to procees current frame.
    //Process function evaluates the frames contents and
    //must be called before getCurrentPeople();
    if (!perseus.process(frame))
    {
      std::cerr << perseus.getErrorDescription() << std::endl;
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

      //Draw a rectangle around person's face on the current frame
      cv::rectangle(frame, face, colors[person.getID()%8], 3);

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
      std::ostringstream gender_string;
      gender_string << "Gender: " << (person.getGender() == -1 ? "male" : "female");
      std::ostringstream age_string;
      age_string << "Age: " << person.getAge();
      std::ostringstream mood_string;
      mood_string << "Mood" << person.getMood();

      cv::putText(frame, id_string.str(), cv::Point(face.x+10, face.y+20),
                  cv::FONT_HERSHEY_SIMPLEX, 0.5, colors[people.at(i).getID()%8]);
      cv::putText(frame, gender_string.str(), cv::Point(face.x+10, face.y+40),
                  cv::FONT_HERSHEY_SIMPLEX, 0.5, colors[people.at(i).getID()%8]);
      cv::putText(frame, age_string.str(), cv::Point(face.x+10, face.y+60),
                  cv::FONT_HERSHEY_SIMPLEX, 0.5, colors[people.at(i).getID()%8]);
      cv::putText(frame, age_string.str(), cv::Point(face.x+10, face.y+60),
                  cv::FONT_HERSHEY_SIMPLEX, 0.5, colors[people.at(i).getID()%8]);
      cv::putText(frame, mood_string.str(), cv::Point(face.x+10, face.y+80),
                  cv::FONT_HERSHEY_SIMPLEX, 0.5, colors[people.at(i).getID()%8]);

    }

    //Show processed frame
    cv::imshow("Perseus example", frame);

    //Press 'q' to quit the program
    char key = cv::waitKey(1);
    if (key == 'q')
    {
      break;
    }
  }
  return 0;
}
