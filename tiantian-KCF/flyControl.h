#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;

class flyControl
{
public:
	flyControl();
	~flyControl();

	void update(cv::Rect result, int frameWidth, int frameHeight, bool GgetInitObject, int GcenterObjectArea);
	
	void init(Rect box, int frameWidth,int frameHeight);
	void pitchUpdateWithArea(Rect result, int initObjectArea, int &P);
	void pitchUpdateWithYShift(Rect result);

	void pitchUpdateWithYShift(Rect result, int frameWidth, int frameHeight, int &P);
	void rollUpdate(Rect result, int frameWidth, int frameHeight, int &R);

	int P;
	int R;
	bool getInitObject; 
	int centerObjectArea;
};

