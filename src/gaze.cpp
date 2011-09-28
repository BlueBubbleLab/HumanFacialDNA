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
    
    for (int i=0; i < 5; i++) {
        float x = i*2.0f;
        float y = i*3.0f;
        float x_prime = 2.0f*x-3.0f*y+1.0f;
        float y_prime = 2.0f*x-3.0f*y+1.0f;
        in.push_back(cv::Point2f(x,y));
        out.push_back(cv::Point2f(x_prime,y_prime));
    }
    cv::Mat a = cv::Mat(in,1),
            b = cv::Mat(out,1);
    cv::Mat output = cv::fitPolynomial(a, b, NULL);
    std::cout << a << b << output << std::endl;
    return 0;
}
