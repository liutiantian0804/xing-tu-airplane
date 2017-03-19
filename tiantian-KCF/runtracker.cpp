#include <iostream>
#include <string>
#include <algorithm>

	   //#include <opencv2/core/core.hpp>
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

int main(int argc, char* argv[]){


	int GP = 1500;
	int GR = 1500;

	string fileName = "C:\\Users\\Aiwei\\Desktop\\2.avi";
	string windowName = "tracking out";

	// Frame readed
	cv::Mat frame;
	cv::Mat displayImage;
	// Tracker results
	cv::Rect result, displayRect;
	cv::Rect firstBox;
	bool getInitbox = false;///add variable
	bool pause = false;

#ifdef SAVE
	cv::VideoWriter saveVideo;
	//string outName = "/media/ise/myfiles/xingtu/"+std::to_string(1)+".avi";

	saveVideo.open("C:\\Users\\Aiwei\\Desktop\\100.avi", CV_FOURCC('M', 'J', 'P', 'G'), 50,
		cv::Size(frame.cols, frame.rows));
#endif

	float peak_value, PSR = 20;
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
		getInitbox = getFirstBoxFromDetection(frame, firstBox, windowName); 
		//getInitbox = getFirstBoxFromDetectionSmallSize(frame, firstBox, windowName);
	}

	KCFTracker *tracker = new KCFTracker;
	flyControl *controller = new flyControl;

	bool GgetInitObject = false;
	int GcenterObjectArea = firstBox.area();

	tracker->init(firstBox, frame/*, G_tmpl, Ghann, G_alphaf, G_prob, Gsize_patch, G_roi, G_tmpl_sz, G_scale*/);

	//C++需要保存的变量
	cv::Mat G_tmpl = tracker->_tmpl;
	cv::Mat Ghann = tracker->hann;
	cv::Mat G_alphaf = tracker->_alphaf;
	cv::Mat G_prob = tracker->_prob;
	int Gsize_patch[3];
	Gsize_patch[0] = tracker->size_patch[0];
	Gsize_patch[1] = tracker->size_patch[1];
	Gsize_patch[2] = tracker->size_patch[2];
	cv::Rect_<float> G_roi = tracker->_roi;
	cv::Size G_tmpl_sz = tracker->_tmpl_sz;
	float G_scale = tracker->_scale;

	cv::Rect newResult;
	float newPSR;
	bool getRedectBox = false;

	while (video.read(frame)){   //detect every 10 frames
		/*video >> frame;
		if (frame.data == NULL)
		{
		end = true;
		break;
		}*/
		fameNum++;
		//  cv::resize(frame,frame,cv::Size(640,360));
		//result = tracker.update(frame, peak_vaule); 
		//newResult = tracker->update(frame, peak_value, PSR);
		frame.copyTo(displayImage);

		delete tracker;
		KCFTracker *tracker = new KCFTracker;
		//cv::Rect KCFTracker::update(cv::Mat image, float &peak_value,
		//cv::Mat G_tmpl, cv::Mat Ghann, cv::Mat G_alphaf, cv::Mat G_prob, int Gsize_patch[3], cv::Rect_<float> G_roi, cv::Size G_tmpl_sz, float G_scale)

		newResult = tracker->update(frame, G_tmpl, Ghann, G_alphaf, G_prob, Gsize_patch, G_roi, G_tmpl_sz, G_scale);

		cv::rectangle(displayImage, newResult, CV_RGB(0, 255, 0), 3);


		G_tmpl = tracker->_tmpl;
		Ghann = tracker->hann;
		G_alphaf = tracker->_alphaf;
		G_prob = tracker->_prob;
		Gsize_patch[0] = tracker->size_patch[0];
		Gsize_patch[1] = tracker->size_patch[1];
		Gsize_patch[2] = tracker->size_patch[2];
		G_roi = tracker->_roi;
		G_tmpl_sz = tracker->_tmpl_sz;
		G_scale = tracker->_scale;


		controller->update(newResult, displayImage.cols, displayImage.rows, GgetInitObject, GcenterObjectArea);
		GP = controller->P;
		GR = controller->R;
		GgetInitObject = controller->getInitObject;
		GcenterObjectArea = controller->centerObjectArea;

		char text[30];
		sprintf_s(text, "P = %d,R = %d", GP / 10 * 10 + 1500, GR / 10 * 10 + 1500);
		cv::putText(displayImage, text, cv::Point(30, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255));

		cv::imshow(windowName, displayImage);
		waitKey(1);


		/*//if (getRedectBox == true)
		//{
		//	newResult = tracker->detectWithBox(frame, peak_value, PSR, tracker->_roi);
		//	//cout << "PSR = " << PSR << endl;
		//}

		//if (PSR >= 10)
		//{
		//	newResult = tracker->update(frame, peak_value, PSR);

		//	cv::rectangle(displayImage, newResult, CV_RGB(0, 255, 0), 3);

		//	controller->update(newResult);
		//	P = controller->getPitch();
		//	R = controller->getRoll();
		//	char text[30];
		//	sprintf_s(text, "P = %d,R = %d", P / 10 * 10 + 1500, R / 10 * 10 + 1500);
		//	cv::putText(displayImage, text, cv::Point(30, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255));

		//	cv::imshow(windowName, displayImage);
		//	waitKey(1);
		//}
		//else
		//{
		//	getRedectBox = false;
		//	if (fameNum % 15 == 0)
		//	{
		//		for (int i = displayImage.rows / 3; i < displayImage.rows / 3 * 2; i += tracker->_roi.height)
		//		{
		//			for (int j = displayImage.cols / 4; j < displayImage.cols / 4 * 3; j += tracker->_roi.width)
		//			{
		//				cv::Rect box = cv::Rect(j, i, tracker->_roi.width, tracker->_roi.height);
		//				//cv::rectangle(displayImage, box, CV_RGB(0, 255, 0));
		//				//cv::imshow("haha", displayImage);
		//				//waitKey(1);
		//				tracker->detect(tracker->_tmpl, tracker->getFeatures2(displayImage, 0, 1.0f, box), peak_value, newPSR);
		//				if (newPSR > 30)
		//				{
		//					PSR = newPSR;//
		//					newResult = box;
		//					tracker->_roi = box;
		//					cv::Mat x = tracker->getFeatures(frame, 0);
		//					tracker->train(x, tracker->interp_factor);
		//					cv::rectangle(displayImage, newResult, CV_RGB(0, 255, 0), 3);

		//					i = displayImage.rows;
		//					j = displayImage.cols;
		//					getRedectBox = true;

		//					controller->update(newResult);
		//					P = controller->getPitch();
		//					R = controller->getRoll();
		//					char text[30];
		//					sprintf_s(text, "P = %d,R = %d", P / 10 * 10 + 1500, R / 10 * 10 + 1500);
		//					//sprintf_s(text, "P = %d,R = %d", P, R);	
		//					cv::putText(displayImage, text, cv::Point(30, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255));

		//					cv::imshow(windowName, displayImage);
		//					waitKey(1);
		//				}
		//			}
		//		}
		//	}

		//	if (getRedectBox == false)
		//	{
		//		P = 1500;
		//		R = 1500;
		//		char text[30];
		//		sprintf_s(text, "P = %d,R = %d", P, R);
		//		cv::putText(displayImage, text, cv::Point(30, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255));
		//		cv::imshow(windowName, displayImage);
		//		waitKey(1);
		//	}

		//}*/

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
