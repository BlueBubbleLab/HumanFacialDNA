#include <iostream>
#include <iomanip>
#include <fstream>
#include <time.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "crowdsight.h"
#include "demo_version.h"
#include "os/ossupport.h"

#include "util/settings.h"

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
  std::string cameraName, instanceName;
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
    if ( file_name != "server")
    {
      cap.open(file_name);
    }
    break;
  }
  case 5:
  {
    file_name = argv[1];
    if ( file_name.find( "--capture" ) != std::string::npos )
    {
      if ( file_name != "server")
      {
        cap.open( atoi( argv[2] ) );
      }
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

  if ( file_name != "server")
  {
    if ( !cap.isOpened() )  // check if we can capture from frame
    {
      std::cerr
          << "Couldn't capture video from input "
          << input
          << std::endl;
      return -1;
    }
  }


  // **************** construct instance ID ********************

  // This should be replaced by the instances registering at the server and the server will
  // assign them a unique instanceID.

  if (!is_webcam_input)
  {
      cameraName = "NC";
  }
  else
  {
      cameraName = input;
  }
  instanceName = cameraName;

  // ***********************************************************




  // ********** CrowdSightServer ****************
  bool runCrowdSightServer = false;
  if ( file_name == "server")
  {
//    if ( Settings::getInstance()->getUseServer() )
//    {
//      runCrowdSightServer = true;
//    }
//    else
//    {
//      //TODO: Nimrod: Diego? How to output this in the right way?
//      std::cout << "ERROR: Server requested, but server settings are turned off" << std::endl;
//    }
    runCrowdSightServer = true;
  }

  if (runCrowdSightServer)
  {
    CrowdSightServer crowdsightserver( data_dir );
    crowdsightserver.runCrowdSightServer();
  }
  // *******************************************
  else
  {
    //Create crowdsight instance
    CrowdSight crowdsight( data_dir, instanceName );




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

  #if RECORDING
    std::ofstream storeFile;
    std::ostringstream fileName;
    fileName << "outputData_" << time( NULL ) << ".csv";
    storeFile.open( fileName.str().c_str(), std::ios::out | std::ios::app );

    std::ofstream storeFileBinned;
    std::ostringstream fileNameBinned;
    fileNameBinned << "outputData_" << time( NULL ) << "_binned.csv";
    storeFileBinned.open( fileNameBinned.str().c_str(), std::ios::out | std::ios::app );

    int outputBinSizeInMiliSec = 10000;
    int endTimePreviousBin     = 0;
    int binCounter             = 0;
    int agesBinSize            = 20;
    int lastBinsPeopleCounter  = 0;

    std::vector<int> ages   ( 4, 0 );
    std::vector<int> genders( 2, 0 );
    std::vector<int> yaws   ( 3, 0 );
    std::vector<int> pitches( 3, 0 );
    std::vector<std::string> uniqueIDsInOutput;
    std::vector<std::string> newIDsThisBin;
  #endif


    //If desirable skips of a video can be skipped.
    int processEveryNthFrame = 1;
    // keep track of how many frames are processed
    int frameCount = 0;

    cap >> frame;

  #if RECORDING
    std::ostringstream videoName;
    videoName << "outputVideo_" << time( NULL ) << ".avi"; //videoName.str().c_str()
    cv::VideoWriter videoOutput( videoName.str().c_str(), CV_FOURCC('M','J','P','G'), 5, frame.size() );
    bool firstFrameToBeWritten = true;
  #endif

    // Define the region of interest here
    int marginTop     = 0;
    int marginBottom  = 0;
    int marginLeft    = 0;
    int marginRight   = 0;
    cv::Rect roi;

//    crowdsight.setMatchingLevel( 0.15 );
//    crowdsight.setFaceConfidence( 0.1 );

    crowdsight.setMinFaceSize( 100 );
    crowdsight.setMaxFaceSize( 120 );


    bool useClassifiers[] = { true, true, true, true };
    bool useScaleFactor   = false;


    //Start main processing loop
    while( true )
    {
      frameCount++;

      //Grab a frame
      cap >> frame;

      //If frame is empty break or reload video
      if ( frame.empty() )
      {
        cap.open(file_name);
        cap >> frame;
  //      break;
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
        std::ostringstream rc_string;
        std::vector< std::vector<int> > clothingColors;

        //Get person at current index
        Person person = people.at(i);

        /*********************************** RETRIEVE PERSON INFO ***********************************/

        cv::Rect    face              = person.getFaceRect();      // Retrieve the person's face
        cv::Point   right_eye         = person.getRightEye();      // Retrieve left and right eye locations of the person. Eye location is relative to the face rectangle.
        cv::Point   left_eye          = person.getLeftEye();
        std::string id                = person.getID();
        float       ageValue          = person.getAge();
        float       genderValue       = ( person.getGender() + 1. ) / 2.;
        float       moodValue         = ( person.getMood() + 1. ) / 2.;
        int64_t     attentionSpan     = person.getAttentionSpan(); // Get person's attention span. This value is returned in milliseconds
        bool        hasClothingColors = person.getClothingColors( 3, clothingColors );
        bool        returningCustomer = person.isReturningCustomer();

        /************************************* DRAW PERSON INFO *************************************/

        // Offset eye position with face position, to get frame coordinates.
        right_eye.x += face.x;
        right_eye.y += face.y;
        left_eye.x  += face.x;
        left_eye.y  += face.y;

        // Drawing positions for the components we want to display
        int moodPos       = -11;
        int genderPos     = 1;
        int agePos        = 12;
        int idPos         = face.y + 10;
        int rcPos         = idPos + 12;
        int attentionPos  = face.y + face.height-3;

        //Draw circles in the center of left and right eyes
        cv::circle(frame, right_eye, 3, cv::Scalar(0,255,0));
        cv::circle(frame, left_eye,  3, cv::Scalar(0,255,0));

        id_string << "ID #" << id; //<< "/" << person.getPredatorID();
        rc_string << "RC " << returningCustomer;

        // Display attention span in minutes:seconds
        int minutes = ( attentionSpan / 60000 );
        int seconds = ( attentionSpan / 1000 ) % 60;
        attention_string << minutes;
        if (seconds < 10) { attention_string << ":0" << seconds; }
        else              { attention_string << ":"  << seconds; }

        cv::putText( frame, id_string.str(),        cv::Point( face.x+3, idPos),        cv::FONT_HERSHEY_SIMPLEX, 0.35, colors[1] );
        cv::putText( frame, rc_string.str(),        cv::Point( face.x+3, rcPos),        cv::FONT_HERSHEY_SIMPLEX, 0.35, cv::Scalar(255, 255, 255) );
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
         cv::Rect genderBorder = cv::Rect( face.x, face.y + face.height + genderPos, face.width, 10 );
        cv::Rect genderPink   = cv::Rect( face.x + floor((1-genderValue)*face.width) + 1, face.y + face.height + genderPos + 1, floor(genderValue*face.width) - 1, 8 );
        cv::Rect genderBlue   = cv::Rect( face.x + 1, face.y + face.height + genderPos + 1, (1-genderValue) * face.width -1, 8 );

        cv::rectangle( frame, genderBlue,cv::Scalar(230,54,0), CV_FILLED);
        cv::rectangle( frame, genderPink, cv::Scalar(147,20,255), CV_FILLED );
        cv::rectangle( frame, genderBorder,cv::Scalar(255,255,255), 1 );

        cv::putText( frame, "GENDER", cv::Point( face.x+3, face.y+face.height+genderPos+7 ), cv::FONT_HERSHEY_PLAIN, 0.5, cv::Scalar(255, 255, 255));

        // SHOW AGE BAR
        if (ageValue > 80) {ageValue=80;}

        double agePerc     = static_cast<double>( ageValue / 80.0 );
        int    ageLocInBar = static_cast<int>( face.width * agePerc );
        int    ageBlock    = static_cast<int>( face.width / 4 );

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

        // Draw the top dominant clothing colors
        if ( hasClothingColors )
        {
          for ( unsigned int cl = 0; cl < clothingColors.size(); ++cl )
          {
            cv::Rect clothColorRect = cv::Rect( face.x + face.width, face.y + ( 30 * cl ), 10, 30 );
            std::vector<int> clColor = clothingColors.at( cl );
            cv::rectangle( frame, clothColorRect,
                          cv::Scalar( clColor.at(2), clColor.at(1), clColor.at(0) ),
                          CV_FILLED );
          }
        }


        // Draw a rectangle around person's face on the current frame
        cv::rectangle(frame, face, cv::Scalar(255,255,255), 1);

        // visualize head pose; for this purpose, yaw and pitch are normalized in [-1...1] by HeadPose
        float yawValue   = 1 - ( ( person.getHeadYaw() + 1. ) / 2. );
        float pitchValue = ( person.getHeadPitch() + 1. ) / 2.;
        cv::line( frame, cv::Point(face.x+face.width/2,face.y+face.height/2), cv::Point(face.x+yawValue*face.width,face.y+pitchValue*face.height), cv::Scalar(255,255,255), 2 );

  #if RECORDING
        //store to csv...
        storeFile << frameCount << "," << person.getID() << "," << person.getTime() << "," << person.getAge() << "," << person.getGender() << ","
                  << person.getFaceRect().x << "," << person.getFaceRect().y << "," << person.getFaceRect().width << ","
                  << person.getHeadYaw() << "," << person.getHeadPitch() << "\n";


        if (person.getTime()-endTimePreviousBin > outputBinSizeInMiliSec)
        {
          binCounter        += 1;
          endTimePreviousBin = person.getTime();

          //store to csv...
          storeFileBinned << binCounter    << "," << uniqueIDsInOutput.size() << "," << newIDsThisBin.size() << "," << crowdsight.getPeopleCount() << "," << crowdsight.getPeopleCount()-lastBinsPeopleCounter << ","
                          << ages.at(0)    << "," << ages.at(1)    << "," << ages.at(2)    << "," << ages.at(3) << ","
                          << genders.at(0) << "," << genders.at(1) << ","
                          << yaws.at(0)    << "," << yaws.at(1)    << "," << yaws.at(2)    << ","
                          << pitches.at(0) << "," << pitches.at(1) << "," << pitches.at(2) << ","  << "\n";

          ages.at(0)    = 0;
          ages.at(1)    = 0;
          ages.at(2)    = 0;
          ages.at(3)    = 0;

          genders.at(0) = 0;
          genders.at(1) = 0;

          yaws.at(0)    = 0;
          yaws.at(1)    = 0;
          yaws.at(2)    = 0;

          pitches.at(0) = 0;
          pitches.at(1) = 0;
          pitches.at(2) = 0;

          newIDsThisBin.clear();

          lastBinsPeopleCounter = crowdsight.getPeopleCount();

        }

        int ageBin = cv::min(3,person.getAge()/agesBinSize);
        ages.at(ageBin) += 1;

        if      (person.getGender() <=0)   { genders.at(0) += 1; }
        else                               { genders.at(1) += 1; }

        if      (person.getHeadYaw() <=0.38)   { yaws.at(0) += 1; }
        else if (person.getHeadYaw() <=0.54)   { yaws.at(1) += 1; }
        else                               { yaws.at(2) += 1; }

        if      (person.getHeadPitch() <=0.38) { pitches.at(0) += 1; }
        else if (person.getHeadPitch() <=0.54) { pitches.at(1) += 1; }
        else                               { pitches.at(2) += 1; }

        std::vector<std::string>::iterator itor = std::find(uniqueIDsInOutput.begin(), uniqueIDsInOutput.end(), person.getID());
        if (itor == uniqueIDsInOutput.end())
        {
          uniqueIDsInOutput.push_back(person.getID());
          newIDsThisBin.push_back(person.getID());
        }
  #endif
      }

  #if RECORDING
      if (people.size()==0)
      {
        // when no people are recognized in a frame, store zeros so the csv file will be synchronous with the video
        storeFile << frameCount << "," << "0" << "," << "0" << "," << "0" << "," << "0" << ","
                  << "0" << "," << "0" << "," << "0" << "," << "0" << "," << "0" << "\n";
      }
  #endif

      std::ostringstream peopleCounter_string;
      std::ostringstream peopleCounterNR_string;
      peopleCounter_string   << "People counter: ";
      peopleCounterNR_string << crowdsight.getPeopleCount();

      std::ostringstream age_string, gender_string, mood_string, headPose_string, scaleFactor_string;

      age_string         << "Age "         << useClassifiers[0];
      gender_string      << "Gender "      << useClassifiers[1];
      mood_string        << "Mood "        << useClassifiers[2];
      headPose_string    << "HeadPose "    << useClassifiers[3];
      scaleFactor_string << "ScaleFactor " << useScaleFactor;

      cv::putText( frame, age_string.str(),         cv::Point(3, 10), cv::FONT_HERSHEY_PLAIN, 0.8, cv::Scalar(0,0,0) );
      cv::putText( frame, gender_string.str(),      cv::Point(3, 25), cv::FONT_HERSHEY_PLAIN, 0.8, cv::Scalar(0,0,0) );
      cv::putText( frame, mood_string.str(),        cv::Point(3, 40), cv::FONT_HERSHEY_PLAIN, 0.8, cv::Scalar(0,0,0) );
      cv::putText( frame, headPose_string.str(),    cv::Point(3, 55), cv::FONT_HERSHEY_PLAIN, 0.8, cv::Scalar(0,0,0) );
      cv::putText( frame, scaleFactor_string.str(), cv::Point(3, 70), cv::FONT_HERSHEY_PLAIN, 0.8, cv::Scalar(0,0,0) );

      cv::putText( frame, peopleCounter_string.str(),   cv::Point(500, 13),  cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0,0,0), 2 );
      cv::putText( frame, peopleCounterNR_string.str(), cv::Point(550, 40), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0,0,0), 2 );

      cv::rectangle( frame, roi, cv::Scalar(255,255,255), 1 );
      cv::imshow( HUMAN_NAME, frame );
      //Show processed frame

  //        cv::Mat bigframe;
  //        cv::resize(frame,bigframe,cv::Size(1280,1024));
  //        cv::imshow(HUMAN_NAME, bigframe);

  #if RECORDING
      if(!firstFrameToBeWritten)
        videoOutput << frame;
      else
        firstFrameToBeWritten = false;
  #endif
      //Press 'q' to quit the program
      char key = cv::waitKey(1);
      if (key == 'q')
      {
  #if RECORDING
        storeFile.close();
  #endif
        // uncomment line below if you want to store changes made run-time in the settings.ini file
//        crowdsight.saveSettings();
        break;
      }

      if (key == '1')
      {
        if ( useClassifiers[0] ) { crowdsight.useAge( false ); useClassifiers[0] = false; }
        else                     { crowdsight.useAge( true );  useClassifiers[0] = true; }
      }

      if (key == '2')
      {
        if ( useClassifiers[1] ) { crowdsight.useGender( false ); useClassifiers[1] = false; }
        else                     { crowdsight.useGender( true );  useClassifiers[1] = true; }
      }

      if (key == '3')
      {
        if ( useClassifiers[2] ) { crowdsight.useMood( false ); useClassifiers[2] = false; }
        else                     { crowdsight.useMood( true );  useClassifiers[2] = true; }
      }

      if (key == '4')
      {
        if ( useClassifiers[3] ) { crowdsight.useHeadPose( false ); useClassifiers[3] = false; }
        else                     { crowdsight.useHeadPose( true );  useClassifiers[3] = true; }
      }

      if (key == 's')
      {
        if ( useScaleFactor ) { crowdsight.useScaleFactor( false ); useScaleFactor = false; }
        else                  { crowdsight.useScaleFactor( true );  useScaleFactor = true; }
      }
    }

  }

  return 0;
}
