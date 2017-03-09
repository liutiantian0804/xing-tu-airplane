#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;

class flyControl
{
public:
	flyControl();
	~flyControl();
	void update(Rect box);
	int getPitch();
	void init(Rect box, int frameWidth,int frameHeight);
	void pitchUpdateWithArea(Rect result);
	void pitchUpdateWithYShift(Rect result);
	void rollUpdate(Rect result);
	int getRoll();

private:
	cv::Point imageCenter;
	float initObjectArea;
	bool getInitObject;
	float imageWidth;
	float imageHeight;
	int pitch;
	int roll;

	float areaThreshold[7] ;
	int pitchThreshold[7] ;
	float xShiftThreshold[7];
	int rollThreshold[7];
	float yShiftThreshold[7];
};

