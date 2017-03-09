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
	int S = 1500;//旋转

	bool HOG = true;
	bool FIXEDWINDOW = false;
	bool MULTISCALE = true;
	bool SILENT = true;  //display tracking result
	bool LAB = true;
	bool pause = false;

	//string fileName = argv[1];
	//string fileName = "C:\\Users\\Aiwei\\Desktop\\Hover.mp4";
	string fileName = "C:\\Users\\Aiwei\\Desktop\\2.avi";
	string windowName = "tracking out";
	// Create KCFTracker object
	//KCFTracker tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);
	//flyControl controller;

	// Frame readed
	cv::Mat frame;
	cv::Mat displayImage;
	// Tracker results
	cv::Rect result, displayRect;
	cv::Rect firstBox;
	bool getInitbox = false;///add variable
	std::vector<int> p_vec;
	std::vector<int> r_vec;
	int p_mean = 0, p_sum = 0;
	int r_mean = 0, r_sum = 0;

#ifdef SAVE
	cv::VideoWriter saveVideo;
	//string outName = "/media/ise/myfiles/xingtu/"+std::to_string(1)+".avi";

	saveVideo.open("C:\\Users\\Aiwei\\Desktop\\100.avi", CV_FOURCC('M', 'J', 'P', 'G'), 50,
		cv::Size(frame.cols, frame.rows));
#endif

	float peak_value, PSR;
	cv::HOGDescriptor peopleDetect;
	peopleDetect.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
	std::vector<cv::Rect> people, peopleFiltered;
	int fameNum = -1;

#ifdef FROMECAMERA
	cv::VideoCapture video(0);///from camera
	if (!video.isOpened())
	{
		printf("input video error\n");
		return -1;
	}
#endif

#ifndef FROMECAMERA
	cv::VideoCapture video(fileName); ///from video
	if (!video.isOpened())
	{
		cout << "camera open error" << endl;
	}
#endif // !FROMECAMERA



	while (!getInitbox)///add function
	{
		video >> frame;
		//	cv::resize(frame, frame, cv::Size(640, 360));
		getInitbox = getFirstBoxFromDetection(frame, firstBox, windowName);
	}
	KCFTracker *tracker = new KCFTracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);
	flyControl *controller = new flyControl;
	tracker->init(firstBox, frame);
	controller->init(firstBox, frame.cols, frame.rows);
	displayRect = firstBox;

	cv::Rect newResult;
	float newPSR;
		while (video.read(frame)){
			/*video >> frame;
			if (frame.data == NULL)
			{
				end = true;
				break;
			}*/
			fameNum++;
			//  cv::resize(frame,frame,cv::Size(640,360));
			//result = tracker.update(frame, peak_vaule); 
			newResult = tracker->update(frame, peak_value, PSR);
			cout << "PSR = " << PSR << endl;
			if (PSR < 10)
			{
				//waitKey(0);
				P = 1500;
				R = 1500;//悬停 旋转
				S = 1500;
				bool out = false;
				while (video.read(frame) && !out)
				{
					frame.copyTo(displayImage);
					cv::imshow(windowName, displayImage);
					waitKey(1);
					for (int i = displayImage.rows / 4; i < displayImage.rows / 4 * 3; i += result.height)
					{
						for (int j = displayImage.cols / 4; j < displayImage.cols / 4 * 3; j += result.width)
						{
							cv::Rect box = cv::Rect(j, i, result.width, result.height);
						//	cv::rectangle(displayImage, box, CV_RGB(0, 255, 0));
						//	cv::imshow("haha", displayImage);
						//	waitKey(1);
							tracker->detect(tracker->_tmpl, tracker->getFeatures2(displayImage, 0, 1.0f, box), peak_value, newPSR);
						//	cout << "newPSR = " << newPSR << endl;
							if (newPSR > 30)
							{
								PSR = newPSR;
								//waitKey(0);
								result = box;
								tracker->_roi = box;
								cv::Mat x = tracker->getFeatures(frame, 0);
								tracker->train(x, tracker->interp_factor);

							/*	delete tracker;
								KCFTracker *tracker = new KCFTracker(HOG, FIXEDWINDOW, MULTISCALE, LAB);
								tracker->init(box, frame);*/
								//displayRect = box;
								i = displayImage.rows;
								j = displayImage.cols;
								out = true;
							}
						}
					}
				}			
			}
			else
			{
				result = newResult;
				//PSR = newPSR ;
			}
			controller->update(result);
			P = controller->getPitch();
			R = controller->getRoll();
			
			/*if (p_vec.size() < 9)
			{
				p_vec.push_back(P);
				p_sum += P;
				p_mean = P;

				r_vec.push_back(R);			
				r_sum += R;				
				r_mean = R;
			}
			else
			{
				p_vec.push_back(P);
				p_sum = p_sum + P;
				p_mean = p_sum / 10;
				p_vec.;
				p_sum = p_sum - p_vec[0];

				r_vec.push_back(R);			
				r_sum += R;			
				r_mean = r_sum / 10;			
				r_vec.pop_back();			
				r_sum -= r_vec.begin;
			}*/

			//  if(fameNum%10 == 5)       
			displayRect = result;
			char text[30];
			sprintf_s(text, "p_mean = %d,r_mean = %d", P/10*10, R/10*10);
			//sprintf_s(text, "P = %d,R = %d", P, R);
			frame.copyTo(displayImage);
			cv::rectangle(displayImage, displayRect, CV_RGB(0, 255, 0), 3);
			cv::putText(displayImage, text, cv::Point(30, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255));

			cv::imshow(windowName, displayImage);
			if (cv::waitKey(1) == 27)
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