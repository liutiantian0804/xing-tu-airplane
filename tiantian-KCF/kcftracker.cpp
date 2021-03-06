﻿
#ifndef _KCFTRACKER_HEADERS
#include "kcftracker.hpp"
#include "ffttools.hpp"
#include "recttools.hpp"
#include "fhog.hpp"
#include "labdata.hpp"
#endif

	   // Constructor
//KCFTracker::KCFTracker(bool hog, bool fixed_window, bool multiscale, bool lab)
//{
//
//	// Parameters equal in all cases
//	lambda = 0.0001;
//	//start para
//	//padding = 2.5;
//	padding = 2.0;
//	//output_sigma_factor = 0.1;
//	output_sigma_factor = 0.125;
//
//
//	if (hog) {    // HOG
//		// VOT
//		interp_factor = 0.012;
//		sigma = 0.6;
//		// TPAMI
//		//interp_factor = 0.02;
//		//sigma = 0.5; 
//		cell_size = 4;
//		_hogfeatures = true;
//
//		if (lab) {
//			interp_factor = 0.005;
//			sigma = 0.4;
//			//output_sigma_factor = 0.025;
//			output_sigma_factor = 0.1;
//
//			_labfeatures = true;
//			_labCentroids = cv::Mat(nClusters, 3, CV_32FC1, &data);
//			cell_sizeQ = cell_size*cell_size;
//		}
//		else{
//			_labfeatures = false;
//		}
//	}
//	else {   // RAW
//		interp_factor = 0.075;
//		sigma = 0.2;
//		cell_size = 1;
//		_hogfeatures = false;
//
//		if (lab) {
//			printf("Lab features are only used with HOG features.\n");
//			_labfeatures = false;
//		}
//	}
//
//
//	if (multiscale) { // multiscale
//		template_size = 96;
//		//template_size = 100;
//		scale_step = 1.05;
//		scale_weight = 0.95;
//		if (!fixed_window) {
//			//printf("Multiscale does not support non-fixed window.\n");
//			fixed_window = true;
//		}
//	}
//	else if (fixed_window) {  // fit correction without multiscale
//		template_size = 96;
//		//template_size = 100;
//		scale_step = 1;
//	}
//	else {
//		template_size = 1;
//		scale_step = 1;
//	}
//}

// Initialize tracker 
void KCFTracker::init(const cv::Rect &roi, cv::Mat image/*,*/
	/*cv::Mat &G_tmpl, cv::Mat &Ghann, cv::Mat &G_alphaf, cv::Mat &G_prob, int Gsize_patch[3], cv::Rect_<float> &G_roi, cv::Size &G_tmpl_sz, float &G_scale*/)
{
	_labCentroids = cv::Mat(nClusters, 3, CV_32FC1, &data);
	_roi = roi;
	assert(roi.width >= 0 && roi.height >= 0);
	_tmpl = getFeatures(image, 1);

	_prob = createGaussianPeak(size_patch[0], size_patch[1]);
	_alphaf = cv::Mat(size_patch[0], size_patch[1], CV_32FC2, float(0));
	//_num = cv::Mat(size_patch[0], size_patch[1], CV_32FC2, float(0));
	//_den = cv::Mat(size_patch[0], size_patch[1], CV_32FC2, float(0));
	train(_tmpl, 1.0); // train with initial frame

	//G_tmpl = _tmpl;
	//Ghann = hann;
	//G_alphaf = _alphaf;
	//G_prob = _prob;
	//Gsize_patch[0] = size_patch[0];
	//Gsize_patch[1] = size_patch[1];
	//Gsize_patch[2] = size_patch[2];
	//G_roi = _roi;
	//G_tmpl_sz = _tmpl_sz;
	//G_scale = _scale;
}
// Update position based on the new frame
// Update position based on the new frame

//cv::Mat _tmpl;
//cv::Mat hann;
//cv::Mat _alphaf;
//cv::Mat _prob;
//int size_patch[3];
//cv::Rect_<float> _roi;
//cv::Size _tmpl_sz;
//float _scale;

cv::Rect KCFTracker::update(cv::Mat image,
	cv::Mat G_tmpl, cv::Mat Ghann, cv::Mat G_alphaf, cv::Mat G_prob, int Gsize_patch[3], cv::Rect_<float> G_roi, cv::Size G_tmpl_sz,float G_scale)
{
	_tmpl = G_tmpl;
	hann = Ghann;
	_alphaf = G_alphaf;
	_prob = G_prob;
	size_patch[0] = Gsize_patch[0];
	size_patch[1] = Gsize_patch[1];
	size_patch[2] = Gsize_patch[2];
	_roi = G_roi;
	_tmpl_sz = G_tmpl_sz;
	_scale = G_scale;
	_labCentroids = cv::Mat(nClusters, 3, CV_32FC1, &data);

	if (_roi.x + _roi.width <= 0) _roi.x = -_roi.width + 1;
	if (_roi.y + _roi.height <= 0) _roi.y = -_roi.height + 1;
	if (_roi.x >= image.cols - 1) _roi.x = image.cols - 2;
	if (_roi.y >= image.rows - 1) _roi.y = image.rows - 2;

	float cx = _roi.x + _roi.width / 2.0f;
	float cy = _roi.y + _roi.height / 2.0f;

	float noScalePeak;
	cv::Point2f res = detect(_tmpl, getFeatures(image, 0, 1.0f), noScalePeak);
	//_prob = createGaussianPeak(size_patch[0], size_patch[1]);//add
	peak_value = noScalePeak;
	if (scale_step != 1) {
		// Test at a smaller _scale
		float new_peak_value;
		int flag = 0;
		cv::Point2f new_res = detect(_tmpl, getFeatures(image, 0, 1.0f / scale_step), new_peak_value);

		if (scale_weight * new_peak_value > noScalePeak) {
			res = new_res;
			peak_value = new_peak_value;
			flag = 1;

		}
		// Test at a bigger _scale
		new_res = detect(_tmpl, getFeatures(image, 0, scale_step), new_peak_value);
		if (scale_weight * new_peak_value > noScalePeak  && new_peak_value > peak_value) {
			res = new_res;
			peak_value = new_peak_value;
			flag = 2;

		}
		switch (flag){
		case 1:
			_scale /= scale_step;
			_roi.width /= scale_step;
			_roi.height /= scale_step;
			break;
		case 2:
			_scale *= scale_step;
			_roi.width *= scale_step;
			_roi.height *= scale_step;
			break;
		default:
			break;
		}
	}

	//printf("peak_value: %f\n", peak_value);

	// Adjust by cell size and _scale
	_roi.x = cx - _roi.width / 2.0f + ((float)res.x * cell_size * _scale);
	_roi.y = cy - _roi.height / 2.0f + ((float)res.y * cell_size * _scale);

	if (_roi.x >= image.cols - 1) _roi.x = image.cols - 1;
	if (_roi.y >= image.rows - 1) _roi.y = image.rows - 1;
	if (_roi.x + _roi.width <= 0) _roi.x = -_roi.width + 2;
	if (_roi.y + _roi.height <= 0) _roi.y = -_roi.height + 2;

	assert(_roi.width >= 0 && _roi.height >= 0);
	cv::Mat x = getFeatures(image, 0);
	train(x, interp_factor);

	return _roi;
}

cv::Rect KCFTracker::update(cv::Mat image, float &peak_value, float &PRS)
{
	if (_roi.x + _roi.width <= 0) _roi.x = -_roi.width + 1;
	if (_roi.y + _roi.height <= 0) _roi.y = -_roi.height + 1;
	if (_roi.x >= image.cols - 1) _roi.x = image.cols - 2;
	if (_roi.y >= image.rows - 1) _roi.y = image.rows - 2;

	float cx = _roi.x + _roi.width / 2.0f;
	float cy = _roi.y + _roi.height / 2.0f;

	float PSR_temp = 0;

	cv::Point2f res = detect(_tmpl, getFeatures(image, 0, 1.0f), peak_value, PSR_temp);
	//res = detect(_tmpl, getFeatures(image, 0, 1.0f, cv::Point(cx, cy)), peak_value, PSR_temp);
	PRS = PSR_temp;
	if (scale_step != 1) {
		// Test at a smaller _scale
		float new_peak_value;
		cv::Point2f new_res = detect(_tmpl, getFeatures(image, 0, 1.0f / scale_step), new_peak_value, PSR_temp);
		if (scale_weight * new_peak_value > peak_value) {
			res = new_res;
			peak_value = new_peak_value;
			_scale /= scale_step;
			_roi.width /= scale_step;
			_roi.height /= scale_step;
			PRS = PSR_temp;
		}

		// Test at a bigger _scale
		new_res = detect(_tmpl, getFeatures(image, 0, scale_step), new_peak_value, PSR_temp);
		if (scale_weight * new_peak_value > peak_value) {
			res = new_res;
			peak_value = new_peak_value;
			_scale *= scale_step;
			_roi.width *= scale_step;
			_roi.height *= scale_step;
			PRS = PSR_temp;
		}
	}

	// Adjust by cell size and _scale  特征坐标变换到位置坐标((float)res.x * cell_size * _scale) 新坐标相对于原来坐标的偏移
	_roi.x = cx - _roi.width / 2.0f + ((float)res.x * cell_size * _scale);
	_roi.y = cy - _roi.height / 2.0f + ((float)res.y * cell_size * _scale);
	//std::cout << ((float)res.x * cell_size * _scale) << " " << cell_size << " " << _scale << std::endl;
	if (_roi.x >= image.cols - 1) _roi.x = image.cols - 1;
	if (_roi.y >= image.rows - 1) _roi.y = image.rows - 1;
	if (_roi.x + _roi.width <= 0) _roi.x = -_roi.width + 2;
	if (_roi.y + _roi.height <= 0) _roi.y = -_roi.height + 2;

	int _roiCenX = _roi.x + _roi.width / 2;
	int _roiCenY = _roi.y + _roi.height / 2;

	assert(_roi.width >= 0 && _roi.height >= 0);
	cv::Mat x = getFeatures(image, 0);
	train(x, interp_factor);

	return _roi;
}

cv::Rect KCFTracker::detectWithBox(cv::Mat image, float &peak_value, float &PRS, cv::Rect box)
{
	cv::Rect result = box;
	float scale_temp = _scale;

	if (box.x + box.width <= 0) box.x = -box.width + 1;
	if (box.y + box.height <= 0) box.y = -box.height + 1;
	if (box.x >= image.cols - 1) box.x = image.cols - 2;
	if (box.y >= image.rows - 1) box.y = image.rows - 2;

	float cx = box.x + box.width / 2.0f;
	float cy = box.y + box.height / 2.0f;

	float PSR_temp = 0;

	cv::Point2f res = detect(_tmpl, getFeatures2(image, 0, 1.0f, box), peak_value, PSR_temp);
	//res = detect(_tmpl, getFeatures(image, 0, 1.0f, cv::Point(cx, cy)), peak_value, PSR_temp);
	PRS = PSR_temp;
	if (scale_step != 1) {
		// Test at a smaller _scale
		float new_peak_value;
		cv::Point2f new_res = detect(_tmpl, getFeatures2(image, 0, 1.0f, box), new_peak_value, PSR_temp);
		if (scale_weight * new_peak_value > peak_value) {
			res = new_res;
			peak_value = new_peak_value;
			scale_temp /= scale_step;//要改变吗
			result.width /= scale_step;
			result.height /= scale_step;
			PRS = PSR_temp;
		}

		// Test at a bigger _scale
		new_res = detect(_tmpl, getFeatures2(image, 0, 1.0f, box), new_peak_value, PSR_temp);
		if (scale_weight * new_peak_value > peak_value) {
			res = new_res;
			peak_value = new_peak_value;
			scale_temp *= scale_step;
			result.width *= scale_step;
			result.height *= scale_step;
			PRS = PSR_temp;
		}
	}

	// Adjust by cell size and _scale  特征坐标变换到位置坐标((float)res.x * cell_size * _scale) 新坐标相对于原来坐标的偏移
	result.x = cx - result.width / 2.0f + ((float)res.x * cell_size * _scale);
	result.y = cy - result.height / 2.0f + ((float)res.y * cell_size * _scale);
	//std::cout << ((float)res.x * cell_size * _scale) << " " << cell_size << " " << _scale << std::endl;
	if (result.x >= image.cols - 1) result.x = image.cols - 1;
	if (result.y >= image.rows - 1) result.y = image.rows - 1;
	if (result.x + result.width <= 0) result.x = -result.width + 2;
	if (result.y + result.height <= 0) result.y = -result.height + 2;

	int _roiCenX = result.x + result.width / 2;
	int _roiCenY = result.y + result.height / 2;

	assert(result.width >= 0 && result.height >= 0);

	return result;
}

// Detect object in the current frame.
cv::Point2f KCFTracker::detect(cv::Mat z, cv::Mat x, float &peak_value)
{
	using namespace FFTTools;

	cv::Mat k = gaussianCorrelation(x, z);
	cv::Mat res = (real(fftd(complexMultiplication(_alphaf, fftd(k)), true)));

	//minMaxLoc only accepts doubles for the peak, and integer points for the coordinates
	cv::Point2i pi;
	double pv;
	cv::minMaxLoc(res, NULL, &pv, NULL, &pi);
	peak_value = (float)pv;

	//subpixel peak estimation, coordinates will be non-integer
	cv::Point2f p((float)pi.x, (float)pi.y);

	if (pi.x > 0 && pi.x < res.cols - 1) {
		p.x += subPixelPeak(res.at<float>(pi.y, pi.x - 1), peak_value, res.at<float>(pi.y, pi.x + 1));
	}

	if (pi.y > 0 && pi.y < res.rows - 1) {
		p.y += subPixelPeak(res.at<float>(pi.y - 1, pi.x), peak_value, res.at<float>(pi.y + 1, pi.x));
	}

	p.x -= (res.cols) / 2;
	p.y -= (res.rows) / 2;

	return p;
}

cv::Point2f KCFTracker::detect(cv::Mat z, cv::Mat x, float &peak_value, float &PSR)
{
	using namespace FFTTools;

	cv::Mat k = gaussianCorrelation(x, z);
	cv::Mat res = (real(fftd(complexMultiplication(_alphaf, fftd(k)), true)));
	//cv::imshow("res", res);//响应值

	//minMaxLoc only accepts doubles for the peak, and integer points for the coordinates
	cv::Point2i pi;
	double pv;
	cv::minMaxLoc(res, NULL, &pv, NULL, &pi);//pv-peak_value  pi-integer points for the coordinates
	peak_value = (float)pv;
	//std::cout << "pi = " << pi << std::endl;
	PSR = calPSR(res, peak_value, pi);

	//subpixel peak estimation, coordinates will be non-integer
	cv::Point2f p((float)pi.x, (float)pi.y);

	if (pi.x > 0 && pi.x < res.cols - 1) {//差值求的最大值
		p.x += subPixelPeak(res.at<float>(pi.y, pi.x - 1), peak_value, res.at<float>(pi.y, pi.x + 1));
	}

	if (pi.y > 0 && pi.y < res.rows - 1) {
		p.y += subPixelPeak(res.at<float>(pi.y - 1, pi.x), peak_value, res.at<float>(pi.y + 1, pi.x));
	}

	p.x -= (res.cols) / 2;//在特征空间上，相对于中心位置的偏移 需要转换到物理坐标上
	p.y -= (res.rows) / 2;

	return p;
}

float KCFTracker::calPSR(cv::Mat res, float peak_value, cv::Point2f p)
{
	std::vector<float> vec;
	//std::cout << "res = " << res.size() << std::endl;

	int PSRwidrh = 5;

	//if (res.cols < 11)
	//{
	//	PSRwidrh = res.cols;
	//}
	for (int i = 0; i < res.rows; i++)
	{
		for (int j = 0; j < res.cols; j++)
		{
			if (i >= res.rows / 2 - PSRwidrh && i <= res.rows / 2 + PSRwidrh && j >= res.cols / 2 - PSRwidrh && j <= res.cols / 2 + PSRwidrh)
			{

			}
			else
			{
				vec.push_back(res.at<float>(i, j));
			}

		}

	}

	cv::Scalar mean_subRes, dev_subRes;
	cv::meanStdDev(vec, mean_subRes, dev_subRes);
	float       mean_pxl = mean_subRes.val[0];
	float       stddev_pxl = dev_subRes.val[0];
	//std::cout << "peak_value = " << peak_value << " mean = " << mean_pxl << " std = " << stddev_pxl << std::endl;
	return  (peak_value - mean_pxl) / stddev_pxl;
	//return 10000* (peak_value * mean_pxl) / (peak_value + mean_pxl);
	//return (peak_value - mean_pxl) /  mean_pxl;
}

// train tracker with a single image
void KCFTracker::train(cv::Mat x, float train_interp_factor)
{
	using namespace FFTTools;

	cv::Mat k = gaussianCorrelation(x, x);
	cv::Mat alphaf = complexDivision(_prob, (fftd(k) + lambda));

	_tmpl = (1 - train_interp_factor) * _tmpl + (train_interp_factor)* x;
	_alphaf = (1 - train_interp_factor) * _alphaf + (train_interp_factor)* alphaf;


	/*cv::Mat kf = fftd(gaussianCorrelation(x, x));
	cv::Mat num = complexMultiplication(kf, _prob);
	cv::Mat den = complexMultiplication(kf, kf + lambda);

	_tmpl = (1 - train_interp_factor) * _tmpl + (train_interp_factor) * x;
	_num = (1 - train_interp_factor) * _num + (train_interp_factor) * num;
	_den = (1 - train_interp_factor) * _den + (train_interp_factor) * den;
	_alphaf = complexDivision(_num, _den);*/

}

// Evaluates a Gaussian kernel with bandwidth SIGMA for all relative shifts between input images X and Y, which must both be MxN. They must    also be periodic (ie., pre-processed with a cosine window).
cv::Mat KCFTracker::gaussianCorrelation(cv::Mat x1, cv::Mat x2)
{
	using namespace FFTTools;
	cv::Mat c = cv::Mat(cv::Size(size_patch[1], size_patch[0]), CV_32F, cv::Scalar(0));
	// HOG features
	if (_hogfeatures) {
		cv::Mat caux;
		cv::Mat x1aux;
		cv::Mat x2aux;
		for (int i = 0; i < size_patch[2]; i++) {
			x1aux = x1.row(i);   // Procedure do deal with cv::Mat multichannel bug
			x1aux = x1aux.reshape(1, size_patch[0]);
			x2aux = x2.row(i).reshape(1, size_patch[0]);
			cv::mulSpectrums(fftd(x1aux), fftd(x2aux), caux, 0, true);
			caux = fftd(caux, true);
			rearrange(caux);
			caux.convertTo(caux, CV_32F);
			c = c + real(caux);
		}
	}
	// Gray features
	else {
		cv::mulSpectrums(fftd(x1), fftd(x2), c, 0, true);
		c = fftd(c, true);
		rearrange(c);
		c = real(c);
	}
	cv::Mat d;
	cv::max(((cv::sum(x1.mul(x1))[0] + cv::sum(x2.mul(x2))[0]) - 2. * c) / (size_patch[0] * size_patch[1] * size_patch[2]), 0, d);

	cv::Mat k;
	cv::exp((-d / (sigma * sigma)), k);
	return k;
}

// Create Gaussian Peak. Function called only in the first frame.
cv::Mat KCFTracker::createGaussianPeak(int sizey, int sizex)
{
	cv::Mat_<float> res(sizey, sizex);

	int syh = (sizey) / 2;
	int sxh = (sizex) / 2;

	printf("sizex:%d, sizey:%d\n", sizex, sizey);

	output_sigma_factor = 0.1;
	float sigmax2 = -0.5*padding*padding / output_sigma_factor / output_sigma_factor / (sizex);
	float sigmay2 = -0.5*padding*padding / output_sigma_factor / output_sigma_factor / (sizey);


	for (int i = 0; i < sizey; i++)
		for (int j = 0; j < sizex; j++)
		{
			int ih = i - syh;
			int jh = j - sxh;
			res(i, j) = std::exp(sigmay2*ih * ih + sigmax2*jh * jh);
		}
	return FFTTools::fftd(res);
}

// Obtain sub-window from image, with replication-padding and extract features
cv::Mat KCFTracker::getFeatures(const cv::Mat & image, bool inithann, float scale_adjust)
{
	cv::Rect extracted_roi;

	float cx = _roi.x + _roi.width / 2;
	float cy = _roi.y + _roi.height / 2;

	if (inithann) {
		int padded_w = _roi.width * padding;
		int padded_h = _roi.height * padding;

		if (template_size > 1) {  // Fit largest dimension to the given template size
			if (padded_w >= padded_h)  //fit to width
				_scale = padded_w / (float)template_size;
			else
				_scale = padded_h / (float)template_size;

			_tmpl_sz.width = padded_w / _scale;
			_tmpl_sz.height = padded_h / _scale;
		}
		else {  //No template size given, use ROI size
			_tmpl_sz.width = padded_w;
			_tmpl_sz.height = padded_h;
			_scale = 1;
			// original code from paper:
			/*if (sqrt(padded_w * padded_h) >= 100) {   //Normal size
			_tmpl_sz.width = padded_w;
			_tmpl_sz.height = padded_h;
			_scale = 1;
			}
			else {   //ROI is too big, track at half size
			_tmpl_sz.width = padded_w / 2;
			_tmpl_sz.height = padded_h / 2;
			_scale = 2;
			}*/
		}

		if (_hogfeatures) {
			// Round to cell size and also make it even
			_tmpl_sz.width = (((int)(_tmpl_sz.width / (2 * cell_size))) * 2 * cell_size) + cell_size * 2;
			_tmpl_sz.height = (((int)(_tmpl_sz.height / (2 * cell_size))) * 2 * cell_size) + cell_size * 2;
		}
		else {  //Make number of pixels even (helps with some logic involving half-dimensions)
			_tmpl_sz.width = (_tmpl_sz.width / 2) * 2;
			_tmpl_sz.height = (_tmpl_sz.height / 2) * 2;
		}
	}

	extracted_roi.width = scale_adjust * _scale * _tmpl_sz.width;
	extracted_roi.height = scale_adjust * _scale * _tmpl_sz.height;

	// center roi with new size
	extracted_roi.x = cx - extracted_roi.width / 2;
	extracted_roi.y = cy - extracted_roi.height / 2;

	cv::Mat FeaturesMap;
	cv::Mat z = RectTools::subwindow(image, extracted_roi, cv::BORDER_REPLICATE);

	if (z.cols != _tmpl_sz.width || z.rows != _tmpl_sz.height) {
		cv::resize(z, z, _tmpl_sz);
	}
	//cv::imshow("z",z);
	// HOG features
	if (_hogfeatures) {
		IplImage z_ipl = z;
		CvLSVMFeatureMapCaskade *map;
		getFeatureMaps(&z_ipl, cell_size, &map);
		normalizeAndTruncate(map, 0.2f);
		PCAFeatureMaps(map);
		size_patch[0] = map->sizeY;
		size_patch[1] = map->sizeX;
		size_patch[2] = map->numFeatures;

		FeaturesMap = cv::Mat(cv::Size(map->numFeatures, map->sizeX*map->sizeY), CV_32F, map->map);  // Procedure do deal with cv::Mat multichannel bug
		FeaturesMap = FeaturesMap.t();
		freeFeatureMapObject(&map);

		// Lab features
		if (_labfeatures) {
			cv::Mat imgLab;
			cvtColor(z, imgLab, CV_BGR2Lab);
			unsigned char *input = (unsigned char*)(imgLab.data);

			// Sparse output vector
			cv::Mat outputLab = cv::Mat(_labCentroids.rows, size_patch[0] * size_patch[1], CV_32F, float(0));

			int cntCell = 0;
			// Iterate through each cell
			for (int cY = cell_size; cY < z.rows - cell_size; cY += cell_size){
				for (int cX = cell_size; cX < z.cols - cell_size; cX += cell_size){
					// Iterate through each pixel of cell (cX,cY)
					for (int y = cY; y < cY + cell_size; ++y){
						for (int x = cX; x < cX + cell_size; ++x){
							// Lab components for each pixel
							float l = (float)input[(z.cols * y + x) * 3];
							float a = (float)input[(z.cols * y + x) * 3 + 1];
							float b = (float)input[(z.cols * y + x) * 3 + 2];

							// Iterate trough each centroid
							float minDist = FLT_MAX;
							int minIdx = 0;
							float *inputCentroid = (float*)(_labCentroids.data);
							for (int k = 0; k < _labCentroids.rows; ++k){
								float dist = ((l - inputCentroid[3 * k]) * (l - inputCentroid[3 * k]))
									+ ((a - inputCentroid[3 * k + 1]) * (a - inputCentroid[3 * k + 1]))
									+ ((b - inputCentroid[3 * k + 2]) * (b - inputCentroid[3 * k + 2]));
								if (dist < minDist){
									minDist = dist;
									minIdx = k;
								}
							}
							// Store result at output
							outputLab.at<float>(minIdx, cntCell) += 1.0 / cell_sizeQ;
							//((float*) outputLab.data)[minIdx * (size_patch[0]*size_patch[1]) + cntCell] += 1.0 / cell_sizeQ; 
						}
					}
					cntCell++;
				}
			}
			// Update size_patch[2] and add features to FeaturesMap
			size_patch[2] += _labCentroids.rows;
			FeaturesMap.push_back(outputLab);
		}
	}
	else {
		FeaturesMap = RectTools::getGrayImage(z);
		FeaturesMap -= (float) 0.5; // In Paper;
		size_patch[0] = z.rows;
		size_patch[1] = z.cols;
		size_patch[2] = 1;
	}

	if (inithann) {
		createHanningMats();
	}
	FeaturesMap = hann.mul(FeaturesMap);
	return FeaturesMap;
}

cv::Mat KCFTracker::getFeatures2(const cv::Mat & image, bool inithann, float scale_adjust, cv::Rect box)
{
	if (box.x + box.width <= 0) box.x = -box.width + 1;
	if (box.y + box.height <= 0) box.y = -box.height + 1;
	if (box.x >= image.cols - 1) box.x = image.cols - 2;
	if (box.y >= image.rows - 1) box.y = image.rows - 2;

	cv::Rect extracted_roi;
	//cv::resize(box, box, cv::Size(_roi.width, _roi.height));
	float cx = box.x + box.width / 2;
	float cy = box.y + box.height / 2;

	if (inithann) {
		int padded_w = box.width * padding;
		int padded_h = box.height * padding;

		if (template_size > 1) {  // Fit largest dimension to the given template size
			if (padded_w >= padded_h)  //fit to width
				_scale = padded_w / (float)template_size;
			else
				_scale = padded_h / (float)template_size;

			_tmpl_sz.width = padded_w / _scale;
			_tmpl_sz.height = padded_h / _scale;
		}
		else {  //No template size given, use ROI size
			_tmpl_sz.width = padded_w;
			_tmpl_sz.height = padded_h;
			_scale = 1;
			// original code from paper:
			/*if (sqrt(padded_w * padded_h) >= 100) {   //Normal size
			_tmpl_sz.width = padded_w;
			_tmpl_sz.height = padded_h;
			_scale = 1;
			}
			else {   //ROI is too big, track at half size
			_tmpl_sz.width = padded_w / 2;
			_tmpl_sz.height = padded_h / 2;
			_scale = 2;
			}*/
		}

		if (_hogfeatures) {
			// Round to cell size and also make it even
			_tmpl_sz.width = (((int)(_tmpl_sz.width / (2 * cell_size))) * 2 * cell_size) + cell_size * 2;
			_tmpl_sz.height = (((int)(_tmpl_sz.height / (2 * cell_size))) * 2 * cell_size) + cell_size * 2;
		}
		else {  //Make number of pixels even (helps with some logic involving half-dimensions)
			_tmpl_sz.width = (_tmpl_sz.width / 2) * 2;
			_tmpl_sz.height = (_tmpl_sz.height / 2) * 2;
		}
	}

	extracted_roi.width = scale_adjust * _scale * _tmpl_sz.width;
	extracted_roi.height = scale_adjust * _scale * _tmpl_sz.height;

	// center roi with new size
	extracted_roi.x = cx - extracted_roi.width / 2;  /////
	extracted_roi.y = cy - extracted_roi.height / 2;  ////

	cv::Mat FeaturesMap;
	cv::Mat z = RectTools::subwindow(image, extracted_roi, cv::BORDER_REPLICATE);

	if (z.cols != _tmpl_sz.width || z.rows != _tmpl_sz.height) {
		cv::resize(z, z, _tmpl_sz);
	}
	//cv::imshow("z",z);
	// HOG features
	if (_hogfeatures) {
		IplImage z_ipl = z;
		CvLSVMFeatureMapCaskade *map;
		getFeatureMaps(&z_ipl, cell_size, &map);
		normalizeAndTruncate(map, 0.2f);
		PCAFeatureMaps(map);
		size_patch[0] = map->sizeY;
		size_patch[1] = map->sizeX;
		size_patch[2] = map->numFeatures;

		FeaturesMap = cv::Mat(cv::Size(map->numFeatures, map->sizeX*map->sizeY), CV_32F, map->map);  // Procedure do deal with cv::Mat multichannel bug
		FeaturesMap = FeaturesMap.t();
		freeFeatureMapObject(&map);

		// Lab features
		if (_labfeatures) {
			cv::Mat imgLab;
			cvtColor(z, imgLab, CV_BGR2Lab);
			unsigned char *input = (unsigned char*)(imgLab.data);

			// Sparse output vector
			cv::Mat outputLab = cv::Mat(_labCentroids.rows, size_patch[0] * size_patch[1], CV_32F, float(0));

			int cntCell = 0;
			// Iterate through each cell
			for (int cY = cell_size; cY < z.rows - cell_size; cY += cell_size){
				for (int cX = cell_size; cX < z.cols - cell_size; cX += cell_size){
					// Iterate through each pixel of cell (cX,cY)
					for (int y = cY; y < cY + cell_size; ++y){
						for (int x = cX; x < cX + cell_size; ++x){
							// Lab components for each pixel
							float l = (float)input[(z.cols * y + x) * 3];
							float a = (float)input[(z.cols * y + x) * 3 + 1];
							float b = (float)input[(z.cols * y + x) * 3 + 2];

							// Iterate trough each centroid
							float minDist = FLT_MAX;
							int minIdx = 0;
							float *inputCentroid = (float*)(_labCentroids.data);
							for (int k = 0; k < _labCentroids.rows; ++k){
								float dist = ((l - inputCentroid[3 * k]) * (l - inputCentroid[3 * k]))
									+ ((a - inputCentroid[3 * k + 1]) * (a - inputCentroid[3 * k + 1]))
									+ ((b - inputCentroid[3 * k + 2]) * (b - inputCentroid[3 * k + 2]));
								if (dist < minDist){
									minDist = dist;
									minIdx = k;
								}
							}
							// Store result at output
							outputLab.at<float>(minIdx, cntCell) += 1.0 / cell_sizeQ;
							//((float*) outputLab.data)[minIdx * (size_patch[0]*size_patch[1]) + cntCell] += 1.0 / cell_sizeQ; 
						}
					}
					cntCell++;
				}
			}
			// Update size_patch[2] and add features to FeaturesMap
			size_patch[2] += _labCentroids.rows;
			FeaturesMap.push_back(outputLab);
		}
	}
	else {
		FeaturesMap = RectTools::getGrayImage(z);
		FeaturesMap -= (float) 0.5; // In Paper;
		size_patch[0] = z.rows;
		size_patch[1] = z.cols;
		size_patch[2] = 1;
	}

	if (inithann) {
		createHanningMats();
	}
	FeaturesMap = hann.mul(FeaturesMap);
	return FeaturesMap;
}

// Initialize Hanning window. Function called only in the first frame.
void KCFTracker::createHanningMats()
{
	cv::Mat hann1t = cv::Mat(cv::Size(size_patch[1], 1), CV_32F, cv::Scalar(0));
	cv::Mat hann2t = cv::Mat(cv::Size(1, size_patch[0]), CV_32F, cv::Scalar(0));

	for (int i = 0; i < hann1t.cols; i++)
		hann1t.at<float >(0, i) = 0.5 * (1 - std::cos(2 * 3.14159265358979323846 * i / (hann1t.cols - 1)));
	for (int i = 0; i < hann2t.rows; i++)
		hann2t.at<float >(i, 0) = 0.5 * (1 - std::cos(2 * 3.14159265358979323846 * i / (hann2t.rows - 1)));

	cv::Mat hann2d = hann2t * hann1t;
	// HOG features
	if (_hogfeatures) {
		cv::Mat hann1d = hann2d.reshape(1, 1); // Procedure do deal with cv::Mat multichannel bug

		hann = cv::Mat(cv::Size(size_patch[0] * size_patch[1], size_patch[2]), CV_32F, cv::Scalar(0));
		for (int i = 0; i < size_patch[2]; i++) {
			for (int j = 0; j<size_patch[0] * size_patch[1]; j++) {
				hann.at<float>(i, j) = hann1d.at<float>(0, j);
			}
		}
	}
	// Gray features
	else {
		hann = hann2d;
	}
}

// Calculate sub-pixel peak for one dimension
float KCFTracker::subPixelPeak(float left, float center, float right)
{
	float divisor = 2 * center - right - left;

	if (divisor == 0)
		return 0;

	return 0.5 * (right - left) / divisor;
}
