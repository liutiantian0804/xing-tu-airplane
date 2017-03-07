#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;

class flyControl
{
public:
	flyControl();
	~flyControl();
	void update(Rect box);
	unsigned int getPitch();
	void init(Rect box, int frameWidth,int frameHeight);
	void pitchUpdateWithArea(Rect result);
	void pitchUpdateWithYShift(Rect result);
	void rollUpdate(Rect result);
	unsigned int getRoll();

private:
	cv::Point imageCenter;
	float initObjectArea;
	bool getInitObject;
	float imageWidth;
	float imageHeight;
	unsigned int pitch;
	unsigned int roll;

	float areaThreshold[7] ;
	unsigned int pitchThreshold[7] ;
	float xShiftThreshold[7];
	unsigned int rollThreshold[7];
	float yShiftThreshold[7];
};

