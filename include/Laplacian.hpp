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
#include "opencv2/highgui/highgui.hpp"
#include "Tools.hpp"

using namespace cv;

class Laplacian{

public:
	static Mat noAdaptive(const Mat& A, const Mat& mask, const int lambda);
	static Mat logKernel(int size, double sigma);
};
#endif /* NOADAPTIVELAPLACIAN_HPP_ */
