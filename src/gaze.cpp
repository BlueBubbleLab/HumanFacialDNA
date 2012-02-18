#include <iostream>
#include <iomanip>
#include <fstream>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "../Perseus/src/perseus.h"
#include "demo_version.h"

#define W 12
#define H 9
#define BORDER 2
#define CAMVIEW_WIDTH 640
#define CAMVIEW_HEIGHT 480

int main (int argc, char *argv[])
{
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

  if(!cap.isOpened())  // check if we can capture from frame
  {
    std::cerr
        << "Couldn't capture video from input "
        << input
        << std::endl;
    return -1;
  }

  Perseus perseus;

  if(!perseus.authenticate(auth_key))
  {
    std::cerr << perseus.getError() << std::endl;
    return -1;
  }

  cap.set(CV_CAP_PROP_FRAME_WIDTH,
          perseus.getCamWidthRes());
  cap.set(CV_CAP_PROP_FRAME_HEIGHT,
          perseus.getCamWidthRes());

  cv::namedWindow(HUMAN_NAME);

  cv::Mat frame;

  for(;;)
  {
    cap >> frame;

    if(frame.data)
    {
      if (is_capturing)
      {
        cv::flip(frame, frame, 1);
      }

      std::vector<bool> emotions;
      if(!perseus.getEmotions(frame, emotions))
      {
        std::cerr << perseus.getError() << std::endl;
      }
      else
      {
        for(size_t i = 0; i < emotions.size(); i++)
        {
          std::cout << emotions.at(i) << " ";
        }
        std::cout << std::endl;
      }
    }
    else
    {
      break;
    }

    cv::imshow(HUMAN_NAME, frame);
    char key = cv::waitKey(1);
    if(key == 'q')
    {
      std::cout << "Wrote output to " << file_name << std::endl;
      break;
    }
  }
  return 0;
}
