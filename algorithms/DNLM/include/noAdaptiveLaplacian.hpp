/*
 * noAdaptiveLaplacian.h
 *
 *  Created on: Nov 1, 2015
 *      Author: davidp
 */

#ifndef NOADAPTIVELAPLACIAN_HPP_
#define NOADAPTIVELAPLACIAN_HPP_

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "highgui/highgui.hpp"
#include "tools.hpp"
using namespace cv;

class NoAdaptiveLaplacian{

public:
	void setMask(Mat mask);
	Mat noAdaptiveLaplacian(const Mat& U, int lambda);

private:
	Mat mask;
	Mat filterUM_laplacianLAB(const Mat& U, int lambda1);
	Mat filterLaplacian2(const Mat& U);
};
#endif /* NOADAPTIVELAPLACIAN_HPP_ */
