/*
 * tools.hpp
 *
 *  Created on: Nov 5, 2015
 *      Author: davidp
 */

#ifndef TOOLS_HPP_
#define TOOLS_HPP_

#include <algorithm>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "highgui/highgui.hpp"

using namespace cv;
using namespace std;

class Tools {
public:
	static Mat fspecialLoG(int size, double sigma);
	static void meshgrid(const Range &xgv, const Range &ygv, Mat &X, Mat &Y);
	static void minMax(const Mat& A, double* minA, double* maxA);
	static void showImg(const Mat& A);
};

#endif /* TOOLS_HPP_ */
