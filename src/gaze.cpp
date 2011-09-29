#include <iostream>
#include <iomanip>
#include <fstream>

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
#define CAMVIEW_WIDTH 640
#define CAMVIEW_HEIGHT 480

int main (void)
{
    std::vector<cv::Point2f> in, out;
    
    for (int i=0; i < 15; i++) {
        float x = i*2.0f;
        float y = i*3.0f;
        float x_prime = 2.0f*x+3.0f*y+0.75f;
        float y_prime = 1.75f*x+5.0f*y+0.5f;
        in.push_back(cv::Point2f(x,y));
        out.push_back(cv::Point2f(x_prime,y_prime));
        std::cout << x_prime << " " << y_prime << std::endl;
    }
    in.push_back(cv::Point2f(28,42));
    out.push_back(cv::Point2f(10000,30));
    cv::Mat a = cv::Mat(in,1),
            b = cv::Mat(out,1),
            mask;
    cv::Mat c = cv::fitPolynomial(a,b,1,CV_FM_LMEDS,0.00001, 0.99,mask);
    std::cout << c << std::endl << mask << std::endl;
    for (int i=0; i < 15; i++) {
        float x = i*2.0f;
        float y = i*3.0f;
        float x_prime = c.at<double>(0,0) + c.at<double>(1,0)*x + c.at<double>(2,0)*y;
        float y_prime = c.at<double>(0,1) + c.at<double>(1,1)*x + c.at<double>(2,1)*y;
        std::cout << x_prime << " " << y_prime << std::endl;
    }
    float x_prime = c.at<double>(0,0) + c.at<double>(1,0)*28 + c.at<double>(2,0)*42;
    float y_prime = c.at<double>(0,1) + c.at<double>(1,1)*28 + c.at<double>(2,1)*42;
    std::cout << x_prime << " " << y_prime << std::endl;
    return 0;
}
