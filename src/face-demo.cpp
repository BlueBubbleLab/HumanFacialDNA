#include <iostream>
#include <iomanip>
#include <fstream>
#include <time.h>
#include <stdlib.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "../inSight/src/insight.h"
#include "demo_version.h"

int main (int argc, char *argv[])
{
  (void) argc;
  (void) argv;
  InSight insight("./data/");
  if(!insight.authenticate("medusa123"))
  {
    std::cerr << insight.getError() << std::endl;
    return -1;
  }
  cv::VideoCapture capture;
  cv::namedWindow(HUMAN_NAME, CV_WINDOW_NORMAL);

  int device = 0;
  if (!capture.open(device))
  {
    std::cerr << "Unable to capture device " << device << std::endl;
    return -1;
  }
  capture.set(CV_CAP_PROP_FRAME_WIDTH, insight.getCamWidthRes());
  capture.set(CV_CAP_PROP_FRAME_HEIGHT, insight.getCamHeightRes());

  cv::Mat camera;
  for (;;)
  {
    capture >> camera;
    cv::flip(camera, camera, 1);
    std::vector<cv::Rect> faces = insight.getFaces(camera);
    for (int i = 0, n = faces.size(); i < n; i++)
    {
      cv::rectangle(camera, faces[i], cv::Scalar(0,0,255),2);
    }
    cv::imshow(HUMAN_NAME, camera);
    char key = cv::waitKey(1);
    switch (key)
    {
    case 'q': return 0;
    default: break;
    }
  }

  return 0;
}
