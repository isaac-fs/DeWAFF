/*
 * NonAdaptiveUSM.h
 *
 *  Created on: Nov 1, 2015
 *      Author: davidp
 */

#ifndef NON_ADAPTIVE_USM_HPP_
#define NON_ADAPTIVE_USM_HPP_

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "Tools.hpp"

using namespace cv;

class NonAdaptiveUSM{

public:
	static Mat LoGkernel(int size, double sigma);
	static Mat nonAdaptiveUSM(const Mat& A, const int lambda);
};
#endif /* NON_ADAPTIVE_USM_HPP */
