#include <iostream>
#include <string>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "kcftracker.hpp"
#include "getFirstBox.h"
//#include <dirent.h>
#include "flyControl.h"

//#define  FLIP
//#define  SAVE

using std::string;
using namespace std;
using namespace cv;

//#define FROMECAMERA   
#define PSR_Threshold 10

int main(int argc, char* argv[]){

	int B = 0;
	int P = 1500;
	int R = 1500;

	bool HOG = true;
	bool FIXEDWINDOW = false;
	bool MULTISCALE = true;
	bool SILENT = true;  //display tracking result
	bool LAB = true;
	bool pause = false;

	//string fileName = argv[1];
	string fileName = "C:\\Users\\Aiwei\\Desktop\\Hover.mp4";
    string windowName = "tracking out";
	// Create KCFTracker object
	KCFTracker tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);
	flyControl controller;

	// Frame readed
	cv::Mat frame;
    cv::Mat displayImage;
	// Tracker results
	cv::Rect result;
    cv::Rect firstBox;

#ifdef FROMECAMERA
	cv::VideoCapture video(0);///from camera
	if (!video.isOpened())
	{
		printf("input video error\n");
		return -1;
	}
	bool getInitbox = false;///add variable
	while (!getInitbox)///add function
	{
		video >> frame;
		cv::resize(frame, frame, cv::Size(640, 360));
		getInitbox = getFirstBoxFromDetection(frame, firstBox, windowName);
	}
#endif

#ifndef FROMECAMERA
	cv::VideoCapture video(fileName); ///from video
	if (!video.isOpened())
	{
		cout << "S" << endl;
	}
	while (1){
		video >> frame;
		cv::resize(frame, frame, cv::Size(640, 360));
		if (getFirstBoxFromDetection(frame, firstBox, windowName))
			break;
	}
#endif // !FROMECAMERA

#ifdef SAVE
	cv::VideoWriter saveVideo;
	//string outName = "/media/ise/myfiles/xingtu/"+std::to_string(1)+".avi";

	saveVideo.open("/media/cyz/myfiles/xingtu/100.avi", CV_FOURCC('M', 'J', 'P', 'G'), 50,
		cv::Size(frame.cols, frame.rows));
#endif

    tracker.init( firstBox, frame );    
	controller.init(firstBox, frame.cols,frame.rows);

	float peak_value, PSR;
    cv::HOGDescriptor peopleDetect;
    peopleDetect.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
    std::vector<cv::Rect> people,peopleFiltered;
    int fameNum = -1;
    cv::Rect displayRect = firstBox;
    while (video.read(frame)){
        fameNum++;
        cv::resize(frame,frame,cv::Size(640,360));
        //result = tracker.update(frame, peak_vaule); 
		result = tracker.update(frame, peak_value, PSR);
		cout << "PSR = " << PSR << endl;
		if (PSR < PSR_Threshold)
		{
			B = 1;
			break;
		}			

		controller.update(result);
		P = controller.getPitch();
		R = controller.getRoll();
		

// 一直加检测
//        people.clear();
//        peopleFiltered.clear();
//        peopleDetect.detectMultiScale(frame,people,0,cv::Size(8,8),cv::Size(32,32),1.05,2);
//        for( int i = 0; i < people.size(); i++ )
//        {
//            cv::Rect r = people[i];
//            int j = 0;
//            for(  j = 0; j < people.size(); j++ )
//                if( j != i && (r & people[j]) == r)
//                    break;
//            if( j == people.size() )
//                peopleFiltered.push_back(r);
//        }
//        for(cv::Rect r : peopleFiltered){
//            r.x += cvRound(r.width*0.1);
//            r.width = cvRound(r.width*0.8);
//            r.y += cvRound(r.height*0.07);
//            r.height = cvRound(r.height*0.8);
//            std::cout<<"people: "<<r<<std::endl;
//
//        }

      //  if(fameNum%10 == 5)       
            displayRect = result;

        frame.copyTo(displayImage);
        cv::rectangle(displayImage,displayRect,CV_RGB(0,255,0),3);  
        cv::imshow(windowName,displayImage);  
        if(cv::waitKey(1) == 27)
            return 0;
#ifdef SAVE
        saveVideo << displayImage;
#endif

		if (pause)
		{
			switch (cv::waitKey(0))
			{
			case     ' ':  pause = !pause; break; //space
			case 2555904:  break;             //arrow right 
			case      27:  return 0;				//esc
			default:
				break;
			}
		}
		else{
			switch (waitKey(1))
			{
			case    32:  pause = !pause; break; //space
			case    27:  return 0;              //esc
			default:
				break;
			}
		}
    }

	

    return 0;
}
