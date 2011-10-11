#include <iostream>
#include <iomanip>
#include <fstream>

#include <QSettings>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "../inSight/src/insight.h"
#include "demo_version.h"



int main (int argc, char *argv[])
{
    cv::VideoCapture cap;
    std::string auth_key;
    std::string file_prefix;
    std::string file_suffix;
    std::string file_name;
    std::vector<std::string> batch_file;
    int file_number;
    int num_files;
    int frame_number;
    bool batch_processing = false;
    std::string data_dir = "../data/";
    switch (argc)
    {
        case 5:
        {
            std::string param = argv[1];
            if (param.find("--batch") != std::string::npos)
    	    {
                std::string file = argv[2];
                std::string line;
                std::ifstream myfile (file.c_str());
                if (myfile.is_open())
                {
                    while ( myfile.good() )
                    {
                        getline (myfile,line);
                        if (myfile.good())
                        {
                            batch_file.push_back(line);
                        }
                    }
                    myfile.close();
                }
                else
                {
                    std::cerr
                    << "Could not open file "
                    << file
                    << std::endl;
                    return -1;
                }
                batch_processing = true;
            }
            data_dir = argv[3];
            auth_key = argv[4];
            file_number = 0;
            num_files = (int)batch_file.size();
            file_name = batch_file.at(file_number);
            cap.open(file_name);
            std::cout << std::endl << " Files in batch: " << 
            num_files << std::endl << std::endl;
            break;
        }
        case 7:
    	{
            file_prefix = argv[1];
            file_number = atoi(argv[2]);
            num_files = atoi(argv[3]);
            file_suffix = argv[4];
            data_dir  = argv[5];
            auth_key = argv[6];
            std::string file_number_str = argv[2];
            file_name = file_prefix + file_number_str + file_suffix;
            cap.open(file_name);
    	    break;
    	}

    	default:
    	{
         std::cout
         << "Usage: "
         << argv[0]
         << " <videofileprefix> <firstfilenumber> <lastfilenumber> <videofilesuffix> <data dir> <auth key>"
         << std::endl;
         std::cout 
         << "       " 
         << argv[0]
         << " --batch <batchfile> <data dir> <auth key>"
         << std::endl;
         return -1;
    	}
    }

    if(!cap.isOpened())  // check if we can capture from camera
    {
    	std::cerr 
            << "Could not open file "
            << file_name
    	    << std::endl;
    	return -1;
    }
    else{
        std::cout << "Processing file: " << file_name << std::endl;
    }
    
    InSight insight;
    if(!insight.authenticate(auth_key))
    {
    	std::cerr << insight.getError() << std::endl;
    	return -1;
    }
    
    cv::namedWindow(HUMAN_NAME, CV_WINDOW_NORMAL);
    cv::Mat frame, full_frame;
    
    // Start indefinite loop and process video
    for(;;)
    {
        if(!cap.read(full_frame)){
            //next movie
            insight.reset();
            frame_number = 0;
            file_number++;
            if(file_number < num_files)
            {
                if (batch_processing) {
                    file_name = batch_file.at(file_number);
                }   
                else
                {
                    std::ostringstream o;
                    o << file_number;
                    file_name = file_prefix + o.str() + file_suffix;
                }
                cap.open(file_name);
                if(!cap.isOpened() || !cap.read(full_frame))
                {
                    std::cerr 
                    << "Could not open or read from file "
                    << file_name
                    << std::endl;
                    return -1;
                }
                else
                {
                    std::cout << std::endl << "Processing file: " << file_name 
                    << std::endl << std::endl;
                }
            }
            else
            {
                break;
            }
        }
        
      full_frame.copyTo(frame); //use this in case you want to crop or resize
    	if (!insight.isInit())
    	{
            if(!insight.init(frame, data_dir))
    	    {
    	    	std::cerr << insight.getError() << std::endl;
    	    }
            else {
                frame_number = 0;
            }
    	    cvMoveWindow(HUMAN_NAME,0,0);
    	}
    	else
    	{
            if(!insight.process(frame))
    	    {
    	    	std::cerr << insight.getError() << std::endl;
    	    }
            else
            {
                //2D mask points
                std::vector<cv::Point> mask_points;
                if (!insight.getMaskPoints(mask_points))
                {
                    std::cerr << insight.getError() << std::endl;
                }
                else
                {
                    //Draw mask points and write to file
                    std::ofstream myfile;
                    std::ostringstream o;
                    o << frame_number;
                    std::string outfile = "./" + file_name + "_2d_coord_fr_" + \
                                          o.str() + ".txt";
                    myfile.open (outfile.c_str());
                    for (int i=0; i < (int)mask_points.size(); i++) 
                    {
                        cv::Point p = mask_points.at(i);
                        cv::circle(frame, p, 1, cv::Scalar(0,255,0));
                        //std::ostringstream o;
                        //o << p.x << "," << p.y << std::endl;
                        myfile << p.x << "," << p.y << std::endl;//o.str();
                    }
                    myfile.close();
                }
                
                //Motion units
                std::vector<float> motion_units;
                if (!insight.getMotionUnits(motion_units))
                {
                    std::cerr << insight.getError() << std::endl;
                }
                else
                {
                    std::ofstream myfile;
                    std::ostringstream o;
                    o << frame_number;
                    std::string outfile = "./" + file_name + "_m_units_fr_" + \
                    o.str() + ".txt";
                    myfile.open (outfile.c_str());
                    for (int i=0; i < (int)motion_units.size(); i++) 
                    {
                        //std::ostringstream o;
                        //o << motion_units.at(i);
                        myfile << motion_units.at(i) << std::endl;//<< o.str() << "\n";
                    }
                    myfile.close();
                }
                    
                //3D mask points
                std::vector<cv::Point3f> mask_points_3d;
                if (!insight.getMaskPoints3D(mask_points_3d))
                {
                    std::cerr << insight.getError() << std::endl;
                }
                else
                {
                    std::ofstream myfile;
                    std::ostringstream o;
                    o << frame_number;
                    std::string outfile = "./" + file_name + "_3d_coord_fr_" + \
                    o.str() + ".txt";
                    myfile.open (outfile.c_str());
                    for (int i=0; i < (int)mask_points_3d.size(); i++) 
                    {
                        //std::ostringstream o;
                        //o << motion_units.at(i);
                        cv::Point3f p = mask_points_3d.at(i);
                        myfile << p.x << "," << p.y << "," << p.z << std::endl;//<< o.str() << "\n";
                    }
                    myfile.close();
                }
                frame_number++;
            }
    	}

        imshow(HUMAN_NAME, frame);
    }
    return 0;
}
