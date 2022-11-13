/**
 * @file Tools.hpp
 * @author David Prado (davidp)
 * @date 2015-11-05
 * @author Isaac Fonseca (isaac.fonsecasegura@ucr.ac.cr)
 * @date 2022-11-06
 * @brief 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef TOOLS_HPP_
#define TOOLS_HPP_

#include <iostream>
#include <algorithm>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;

/**
 * @brief Useful tools for the file processing
 * 
 */
class Tools {
public:
	static void meshGrid(const Range &range, Mat &X, Mat &Y);
	static void getMinMax(const Mat& A, double* minA, double* maxA);
	static void addPadding(Mat &RoI, int xRangeSize, int yRangeSize);
};

#endif /* TOOLS_HPP_ */
