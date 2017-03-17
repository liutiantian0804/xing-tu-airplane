//
// Created by ise on 16-10-28.
//

#ifndef TEST_GETFIRSTBOX_H
#define TEST_GETFIRSTBOX_H


#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>

//#include <opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <string>

void getFirstBoxFromMouse(const cv::Mat &firstFrame, cv::Rect &firstBox, std::string windowName);
void mouseHandlerForMouse(int event, int x, int y, int flag, void *userdata);
void mouseHandlerForDetection(int event, int x, int y, int flag, void *userdata);
bool getFirstBoxFromDetection(const cv::Mat &firstFrame, cv::Rect &firstBox, std::string windName);

bool getFirstBoxFromDetectionSmallSize(const cv::Mat &secondFrame, cv::Rect &firstBox, std::string windName);
#endif //TEST_GETFIRSTBOX_H
