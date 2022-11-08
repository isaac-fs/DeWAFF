/**
 * @file NonAdaptiveUSM.hpp
 * @author David Prado (davidp)
 * @date 2015-11-01
 * @author Isaac Fonseca (isaac.fonsecasegura@ucr.ac.cr)
 * @date 2022-11-06
 * @brief 
 * @copyright Copyright (c) 2022
 * 
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
	static Mat Filter(const Mat& A, const int lambda);
	static Mat LaplacianKernel(int size, double sigma);
};
#endif /* NON_ADAPTIVE_USM_HPP */
