//
// Created by ise on 16-10-28.
//

#include "getFirstBox.h"
#include <iostream>
#include<vector>
#include "cascadedetect.hpp"
#include "time.h"

using namespace std;
using namespace cv;

bool getBox = false;
bool getPoint = false;
cv::Point peoplePoint;

void MYdetectMultiScale(
	const Mat& img, vector<Rect>& foundLocations,
	double hitThreshold, Size winStride, Size padding,
	double scale0, double finalThreshold, bool useMeanshiftGrouping)
{
	double scale = 1.;
	int levels = 0;
	vector<int> foundWeights;
	/*vector<double> levelScale;
	for (levels = 0; levels < 64; levels++)
	{
		levelScale.push_back(scale);
		if (cvRound(img.cols / scale) < winSize.width ||
			cvRound(img.rows / scale) < winSize.height ||
			scale0 <= 1)
			break;
		scale *= scale0;
	}
	levels = std::max(levels, 1);
	levelScale.resize(levels);*/

	std::vector<Rect> allCandidates;
	//std::vector<double> tempScales;
	std::vector<double> tempWeights;
	std::vector<double> foundScales;
	Mutex mtx;

	/*parallel_for_(Range(0, (int)levelScale.size()),
		HOGInvoker(this, img, hitThreshold, winStride, padding, &levelScale[0], &allCandidates, &mtx, &tempWeights, &tempScales));*/

	//std::copy(tempScales.begin(), tempScales.end(), back_inserter(foundScales));
	foundLocations.clear();
	std::copy(allCandidates.begin(), allCandidates.end(), back_inserter(foundLocations));
	foundWeights.clear();
	std::copy(tempWeights.begin(), tempWeights.end(), back_inserter(foundWeights));

	if (useMeanshiftGrouping)
	{
		//groupRectangles_meanshift(foundLocations, foundWeights, foundScales, finalThreshold, winSize);
	}
	else
	{
		groupRectangles(foundLocations, foundWeights, (int)finalThreshold, 0.2);
	}
}

//void MYdetectMultiScale(const Mat& img, vector<Rect>& foundLocations,
//	double hitThreshold, Size winStride, Size padding,
//	double scale0, double finalThreshold, bool useMeanshiftGrouping)
//{
//	vector<double> foundWeights;
//	MYdetectMultiScale(img, foundLocations, foundWeights, hitThreshold, winStride,
//		padding, scale0, finalThreshold, useMeanshiftGrouping);
//}

void getFirstBoxFromMouse(const cv::Mat &firstFrame, cv::Rect &firstBox, std::string windowName){

	cv::Mat frameCopy;
	cv::Rect boxTemp;
	cv::namedWindow(windowName);
	cv::setMouseCallback(windowName, mouseHandlerForMouse, &boxTemp);

	while (!getBox){
		firstFrame.copyTo(frameCopy);
		cv::rectangle(frameCopy, boxTemp, CV_RGB(255, 0, 0), 2);
		cv::imshow(windowName, frameCopy);
		cv::waitKey(1);
	}
	cv::setMouseCallback(windowName, NULL);
	cv::waitKey(0);
	firstBox = boxTemp;
	std::cout << firstBox << std::endl;

	getBox = false;
	return;
}

bool getFirstBoxFromDetectionSmallSize(const cv::Mat &secondFrame, cv::Rect &firstBox, std::string windName){
	double t = 0, fps;//secondFrame 原始图像   firstFrame：640*480  smallFrame：以点为中心的320*240
	t = (double)cv::getTickCount();
	cv::Mat frameCopy;
	cv::Mat firstFrame;
	cv::Mat smallFrame;
	//cout << secondFrame.cols << " " << secondFrame.rows << endl;
	cv::resize(secondFrame, firstFrame, cv::Size(640, 480));
	cv::HOGDescriptor peopleHog;
	peopleHog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

	cv::namedWindow(windName);
	cv::setMouseCallback(windName, mouseHandlerForDetection, &peoplePoint);

	if (!getPoint){
		firstFrame.copyTo(frameCopy);
		// cv::circle(frameCopy,peoplePoint,5,CV_RGB(255,0,0),3);
		cv::imshow(windName, frameCopy);
		cv::waitKey(1);
		cv::setMouseCallback(windName, NULL);
		return false;
	}
	getPoint = false;
	std::cout << "peoplePoint: " << peoplePoint << std::endl;
	cv::setMouseCallback(windName, NULL);
	firstFrame.copyTo(frameCopy);
	std::vector<cv::Rect> people, peopleFiltered;

	smallFrame = firstFrame(cv::Rect(max(0, peoplePoint.x - firstFrame.cols / 4), max(0, peoplePoint.y - firstFrame.rows / 4), firstFrame.cols / 2, firstFrame.rows / 2));
	imshow("s", smallFrame);
	//int xbias = max(0, peoplePoint.x - firstFrame.cols / 4);

	peopleHog.detectMultiScale(smallFrame, people, 0, cv::Size(16, 16), cv::Size(0, 0), 1.05, 2);
	//MYdetectMultiScale(firstFrame, people, 0, cv::Size(8, 8), cv::Size(32, 32), 1.05, 2,0);

	for (int i = 0; i < people.size(); i++)
	{
		cv::Rect r = people[i]; //r xiaokuang
		int j = 0;
		for (j = 0; j < people.size(); j++)
			if (j != i && (r & people[j]) == r)
				break;
		if (j == people.size())
			peopleFiltered.push_back(r);
	}
	if (peopleFiltered.empty()) //没有检测到人，直接返回
	{
		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
		fps = /*1.0 /*/ t;
		cout << "no peopleo in this frame and fps1 = " << fps << endl;
		cv::waitKey(0);
		return false;
	}
		
	for (cv::Rect r : peopleFiltered){
		r.x += cvRound(r.width*0.1);
		r.width = cvRound(r.width*0.8);
		r.y += cvRound(r.height*0.07);
		r.height = cvRound(r.height*0.8);
		std::cout << "people: " << r << std::endl;
		cv::Point smallPeoplePoint(peoplePoint.x - max(0, peoplePoint.x - firstFrame.cols / 4), peoplePoint.y - max(0, peoplePoint.y - firstFrame.rows / 4));
		if (smallPeoplePoint.inside(r))
		{
			//cv::rectangle(smallFrame, r, CV_RGB(0, 255, 0), 3);
			//imshow("ff", smallFrame);
			cv::rectangle(frameCopy, cv::Rect(r.x + max(0, peoplePoint.x - firstFrame.cols / 4), r.y + max(0, peoplePoint.y - firstFrame.rows / 4), r.width, r.height), CV_RGB(0, 255, 0), 3);
			firstBox = cv::Rect(r.x + max(0, peoplePoint.x - firstFrame.cols / 4), r.y + max(0, peoplePoint.y - firstFrame.rows / 4), r.width, r.height);
			t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
			fps =/* 1.0 /*/ t;
			cout << "find people and fps2 = " << fps << endl;
			cv::imshow(windName, frameCopy);
			//cv::waitKey(0);
			return true;
		}
		else
			cv::rectangle(frameCopy, cv::Rect(r.x + max(0, peoplePoint.x - firstFrame.cols / 4), r.y + max(0, peoplePoint.y - firstFrame.rows / 4), r.width, r.height), CV_RGB(255, 255, 255), 3);
	}
	cv::imshow(windName, frameCopy);
	

	//std::cout << "no people at this point" << std::endl;
	t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
	fps = /*1.0 /*/ t;
	cout << "no people at this point and fps3 = " << fps << endl;
	cv::waitKey(0);
	return false;
}

bool getFirstBoxFromDetection(const cv::Mat &firstFrame, cv::Rect &firstBox, std::string windName){
	double t = 0, fps;
	t = (double)cv::getTickCount();
	cv::Mat frameCopy;
	cv::HOGDescriptor peopleHog;
	peopleHog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

	cv::namedWindow(windName);
	cv::setMouseCallback(windName, mouseHandlerForDetection, &peoplePoint);

	if (!getPoint){
		firstFrame.copyTo(frameCopy);
		// cv::circle(frameCopy,peoplePoint,5,CV_RGB(255,0,0),3);
		cv::imshow(windName, frameCopy);
		cv::waitKey(1);
		cv::setMouseCallback(windName, NULL);
		return false;
	}
	getPoint = false;
	std::cout << "peoplePoint: " << peoplePoint << std::endl;
	cv::setMouseCallback(windName, NULL);
	firstFrame.copyTo(frameCopy);
	std::vector<cv::Rect> people, peopleFiltered;

	peopleHog.detectMultiScale(firstFrame, people, 0, cv::Size(16, 16), cv::Size(0, 0), 1.05, 2);

	for (int i = 0; i < people.size(); i++)
	{
		cv::Rect r = people[i]; 
		int j = 0;
		for (j = 0; j < people.size(); j++)
			if (j != i && (r & people[j]) == r)
				break;
		if (j == people.size())
			peopleFiltered.push_back(r);
	}
	if (peopleFiltered.empty()) //没有检测到人，直接返回
	{
		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
		fps = /*1.0 /*/ t;
		cout << "no peopleo in this frame and fps1 = " << fps << endl;
		cv::waitKey(0);
		return false;
	}

	for (cv::Rect r : peopleFiltered){
		r.x += cvRound(r.width*0.1);
		r.width = cvRound(r.width*0.8);
		r.y += cvRound(r.height*0.07);
		r.height = cvRound(r.height*0.8);
		std::cout << "people: " << r << std::endl;
		if (peoplePoint.inside(r))
		{
			cv::rectangle(frameCopy, r, CV_RGB(0, 255, 0), 3);
			firstBox = r;
			t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
			fps =/* 1.0 /*/ t;
			cout << "find people and fps2 = " << fps << endl;
			cv::imshow(windName, frameCopy);
			//cv::waitKey(0);
			return true;
		}
		else
			cv::rectangle(frameCopy, r, CV_RGB(255, 255, 255), 3);
	}
	cv::imshow(windName, frameCopy);


	//std::cout << "no people at this point" << std::endl;
	t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
	fps = /*1.0 /*/ t;
	cout << "no people at this point and fps3 = " << fps << endl;
	cv::waitKey(0);
	return false;
}

void mouseHandlerForDetection(int event, int x, int y, int flag, void *userdata){

	static bool drawPoint = false;
	cv::Point* peoplePoint = (cv::Point*)userdata;

	switch (event){
	case CV_EVENT_MOUSEMOVE:
		if (drawPoint){
			peoplePoint->x = x;
			peoplePoint->y = y;
		}
		break;
	case CV_EVENT_LBUTTONDOWN:
		drawPoint = true;
		peoplePoint->x = x;
		peoplePoint->y = y;
		break;
	case CV_EVENT_LBUTTONUP:
		peoplePoint->x = x;
		peoplePoint->y = y;
		getPoint = true;
		break;
	default:
		break;
	}
}

void mouseHandlerForMouse(int event, int x, int y, int flag, void *userdata){

	static bool drawingBox = false;
	cv::Rect* firstBox = (cv::Rect*)userdata;

	switch (event){
	case CV_EVENT_MOUSEMOVE:
		if (drawingBox){
			firstBox->width = x - firstBox->x;
			firstBox->height = y - firstBox->y;
		}
		break;
	case CV_EVENT_LBUTTONDOWN:
		drawingBox = true;
		firstBox->x = x;
		firstBox->y = y;
		break;
	case CV_EVENT_LBUTTONUP:
		drawingBox = false;
		if (firstBox->width < 0)
		{
			firstBox->x += firstBox->width;
			firstBox->width *= -1;
		}
		if (firstBox->height < 0)
		{
			firstBox->y += firstBox->height;
			firstBox->height *= -1;
		}
		getBox = true;
		break;
	default:
		break;
	}

}

