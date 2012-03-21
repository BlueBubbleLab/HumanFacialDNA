#include <iostream>
#include <iomanip>
#include <fstream>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "../Perseus/src/perseus.h"


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

    cv::Mat tits = cv::imread("tits.jpg");
    cv::Mat pony = cv::imread("pony.jpg");

    Perseus perseus(data_dir);

    if(!perseus.authenticate(auth_key))
    {
        std::cerr << perseus.getErrorDescription() << std::endl;
        return -1;
    }

    cap.set(CV_CAP_PROP_FRAME_WIDTH,
            640 /*perseus.getCamWidthRes()*/);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,
            480 /*perseus.getCamWidthRes()*/);

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

    for(;;)
    {
        cap >> frame;

        if(frame.data)
        {
            if (is_capturing)
            {
                cv::flip(frame, frame, 1);
            }



            // NOTE: HERE IS A TYPICAL USAGE SCENARIO PSEUDISHCODE: ===================================================
            if(!perseus.process(frame))
            {
                std::cerr << perseus.getErrorDescription() << std::endl;
            }

            std::vector<Person> people;
            perseus.getCurrentPeople(people);

            for (unsigned int i=0; i<people.size();i++)
            {
                cv::Rect face = (people.at(i)).getFaceRect();

                cv::rectangle(frame,face,colors[people.at(i).getBestMatch()%8],3);

                cv::Point rightEye = (people.at(i)).getRightEye();
                cv::Point leftEye  = (people.at(i)).getLeftEye();

                rightEye.x += face.x;
                rightEye.y += face.y;
                leftEye.x += face.x;
                leftEye.y += face.y;

                cv::circle(frame,rightEye,3,cv::Scalar(0,255,0));
                cv::circle(frame,leftEye,3,cv::Scalar(0,255,0));

                std::ostringstream idString;
                idString << "ID #" << people.at(i).getBestMatch();
                std::ostringstream genderString;
                genderString << "Gender: " << ((people.at(i).getGender())==-1?"male":"female");
                std::ostringstream ageString;
                ageString << "Age: " << people.at(i).getAge();
                std::ostringstream moodString;
                moodString << "Mood" << people.at(i).getMood();
                cv::putText(frame, idString.str(),cv::Point(face.x+10,face.y+20),cv::FONT_HERSHEY_SIMPLEX, 0.5, colors[people.at(i).getBestMatch()%8]);
                cv::putText(frame, genderString.str(), cv::Point(face.x+10,face.y+40),cv::FONT_HERSHEY_SIMPLEX, 0.5, colors[people.at(i).getBestMatch()%8]);
                cv::putText(frame, ageString.str(), cv::Point(face.x+10,face.y+60),cv::FONT_HERSHEY_SIMPLEX, 0.5, colors[people.at(i).getBestMatch()%8]);
                cv::putText(frame, ageString.str(), cv::Point(face.x+10,face.y+60),cv::FONT_HERSHEY_SIMPLEX, 0.5, colors[people.at(i).getBestMatch()%8]);
                cv::putText(frame, moodString.str(), cv::Point(face.x+10,face.y+80),cv::FONT_HERSHEY_SIMPLEX, 0.5, colors[people.at(i).getBestMatch()%8]);

            }

            //
            // ******************************************************************** //
        }
        else
        {
            break;
        }
//        cv::Mat bigframe;
//        cv::resize(frame,bigframe,cv::Size(1280,1024));
//        cv::imshow(HUMAN_NAME, bigframe);
        cv::imshow("Perseus example", frame);
        char key = cv::waitKey(1);
        if(key == 'q')
        {
            std::cout << "Wrote output to " << file_name << std::endl;
            break;
        }
    }
    return 0;
}
