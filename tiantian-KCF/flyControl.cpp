#include "flyControl.h"
#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;

flyControl::flyControl()
{

}

flyControl::~flyControl()
{
}

void flyControl::init(Rect box, int frameWidth, int frameHeight)
{
	//initObjectArea = box.area();
	getInitObject = false;
	imageCenter = cv::Point(frameWidth / 2, frameHeight / 2);
	imageWidth = frameWidth;
	imageHeight = frameHeight;

	areaThreshold[0] = 0.25;
	areaThreshold[1] = 0.5;
	areaThreshold[2] = 0.75;
	areaThreshold[3] = 1;
	areaThreshold[4] = 1.25;
	areaThreshold[5] = 1.5;
	areaThreshold[6] = 1.75;

	xShiftThreshold[0] = -0.15;
	xShiftThreshold[1] = -0.1;
	xShiftThreshold[2] = -0.04;
	xShiftThreshold[3] = 0;
	xShiftThreshold[4] = 0.04;
	xShiftThreshold[5] = 0.1;
	xShiftThreshold[6] = 0.15;

	yShiftThreshold[0] = -0.15;
	yShiftThreshold[1] = -0.1;
	yShiftThreshold[2] = -0.04;
	yShiftThreshold[3] = 0;
	yShiftThreshold[4] = 0.04;
	yShiftThreshold[5] = 0.1;
	yShiftThreshold[6] = 0.15;

	rollThreshold[0] = 1300;
	rollThreshold[1] = 1400;
	rollThreshold[2] = 1500;
	rollThreshold[3] = 1500;
	rollThreshold[4] = 1500;
	rollThreshold[5] = 1600;
	rollThreshold[6] = 1700;

	pitchThreshold[0] = 1300;
	pitchThreshold[1] = 1400;
	pitchThreshold[2] = 1500;
	pitchThreshold[3] = 1500;
	pitchThreshold[4] = 1500;
	pitchThreshold[5] = 1600;
	pitchThreshold[6] = 1700;
}

void flyControl::pitchUpdateWithArea(Rect result)
{
	float boxArea = result.area();
	//cout << "boxArea / initObjectArea = " << boxArea / initObjectArea << endl;

	for (int i = 0; i < 7;)
	{
		if (boxArea > initObjectArea*areaThreshold[i])
		{
			i++;
		}
		else
		{
			pitch = pitchThreshold[i];
			break;
		}
	}
}

void flyControl::pitchUpdateWithYShift(Rect result)
{
	float yshift = (result.y + result.height / 2) - imageCenter.y;
	//cout << "yshift/ imageHeight = " << yshift / imageHeight << endl;

	for (int i = 0; i < 7;)
	{
		if (yshift > imageHeight*yShiftThreshold[i])
		{
			i++;
		}
		else
		{
			pitch = pitchThreshold[i];
			break;
		}
	}
}

void flyControl::rollUpdate(Rect result)
{
	float xshift = (result.x + result.width / 2) - imageCenter.x;
	//cout << "xshift/ imageWidth = " << xshift / imageWidth << endl;

	for (int i = 0; i < 7;)
	{
		if (xshift > imageWidth*xShiftThreshold[i])
		{
			i++;
		}
		else
		{
			roll = rollThreshold[i];
			break;
		}
	}
}

void flyControl::update(cv::Rect result)
{
	if (!getInitObject) //得到初始框，在图像的正中间
	{
		cv::Point resultCenter(result.x + result.width / 2, result.y + result.height / 2);
		if (abs(resultCenter.x - imageCenter.x) < 20 && abs(resultCenter.y - imageCenter.y) < 20) //20 can be changed
		{
			initObjectArea = result.area();
			getInitObject = true;
		}
	}
	if (getInitObject)//图像在正中间时，根据面积调节前后
	{
		pitchUpdateWithArea(result);
		//cout << "Area" << endl;
	}
	else//否则，跟踪y调节前后
	{
		pitchUpdateWithYShift(result);
		//cout << "YShift" << endl;
	}
	
	rollUpdate(result);//调节左右
}

unsigned int flyControl::getPitch()
{
	return pitch;
}

unsigned int flyControl::getRoll()
{
	return roll;
}

