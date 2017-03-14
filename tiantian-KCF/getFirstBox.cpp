//
// Created by ise on 16-10-28.
//

#include "getFirstBox.h"
#include <iostream>
bool getBox = false;
bool getPoint = false;
cv::Point peoplePoint;

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

bool getFirstBoxFromDetection(const cv::Mat &firstFrame, cv::Rect &firstBox, std::string windName){

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
	peopleHog.detectMultiScale(firstFrame, people, 0, cv::Size(8, 8), cv::Size(32, 32), 1.05, 2);

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
		return false;
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
			return true;
		}
		else
			cv::rectangle(frameCopy, r, CV_RGB(255, 0, 0), 3);
	}
	cv::imshow(windName, frameCopy);
	//cv::waitKey(0);

	std::cout << "no people at this point" << std::endl;
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

