
#ifndef _OPENCV_KCFTRACKER_HPP_
#define _OPENCV_KCFTRACKER_HPP_
#endif

	   //#include <opencv2/core.hpp>

	   //#include <opencv/cxcore.h>
#include<opencv2/core/core.hpp>

class KCFTracker {
public:
	// Constructor
	//KCFTracker(bool hog = true, bool fixed_window = true, bool multiscale = true, bool lab = true);

	// Initialize tracker 
	void init(const cv::Rect &roi, cv::Mat image);

	// Update position based on the new frame
	cv::Rect update(cv::Mat image, float &peak_value);
	cv::Rect update(cv::Mat image, float &peak_value, float &PRS);
	cv::Rect detectWithBox(cv::Mat image, float &peak_value, float &PRS, cv::Rect box);

	float interp_factor = 0.012; // linear interpolation factor for adaptation
	float sigma = 0.6; // gaussian kernel bandwidth
	float lambda = 0.0001; // regularization
	int cell_size = 4; // HOG cell size
	int cell_sizeQ = 16; // cell size^2, to avoid repeated operations   lab有关
	float padding = 2.0; // extra area surrounding the target
	float output_sigma_factor = 0.125; // bandwidth of gaussian target
	int template_size = 96; // template size
	float scale_step = 1.05; // scale step for multi-scale estimation
	float scale_weight = 0.95;  // to downweight detection scores of other scales for added stability
	bool _hogfeatures = true;
	bool _labfeatures = true;
	cv::Mat _labCentroids;

	//需要传出去的变量
	cv::Mat _tmpl;
	cv::Mat hann;
	cv::Mat _alphaf;
	cv::Mat _prob;	
	int size_patch[3];
	cv::Rect_<float> _roi;
	cv::Size _tmpl_sz;
	float _scale;


	cv::Mat getFeatures(const cv::Mat & image, bool inithann, float scale_adjust = 1.0f);
	cv::Mat getFeatures2(const cv::Mat & image, bool inithann, float scale_adjust, cv::Rect box);
	cv::Point2f detect(cv::Mat z, cv::Mat x, float &peak_value, float &PSR);
	// train tracker with a single image
	void train(cv::Mat x, float train_interp_factor);

	// Detect object in the current frame.
	cv::Point2f detect(cv::Mat z, cv::Mat x, float &peak_value);
	//cv::Point2f detect(cv::Mat z, cv::Mat x, float &peak_value, float &PSR);
	float calPSR(cv::Mat res, float peak_value, cv::Point2f p);
	float peak_value;

	// Evaluates a Gaussian kernel with bandwidth SIGMA for all relative shifts between input images X and Y, which must both be MxN. They must    also be periodic (ie., pre-processed with a cosine window).
	cv::Mat gaussianCorrelation(cv::Mat x1, cv::Mat x2);

	// Create Gaussian Peak. Function called only in the first frame.
	cv::Mat createGaussianPeak(int sizey, int sizex);

	// Obtain sub-window from image, with replication-padding and extract features
	//cv::Mat getFeatures(const cv::Mat & image, bool inithann, float scale_adjust = 1.0f);

	// Initialize Hanning window. Function called only in the first frame.
	void createHanningMats();

	// Calculate sub-pixel peak for one dimension
	float subPixelPeak(float left, float center, float right);



};
