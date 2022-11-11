/**
 * @file DeWAFF.cpp
 * @author David Prado (davidp)
 * @date 2015-08-29
 * @author Isaac Fonseca (isaac.fonsecasegura@ucr.ac.cr)
 * @date 2022-11-06
 * @brief 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef DEWAFF_H_
#define DEWAFF_H_

#include <omp.h>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "Tools.hpp"

using namespace cv;

/**
 * @brief Deceived Weighted Averaged Filter Framework class.
 * It applies filter using the deceived weighted averaged values of an image
 */
class DeWAFF {
	private:
		enum CIELab : int {L, a, b}; // CIELab channels
		Mat inputImage; // Original input
		int windowSize;
		Range range;
		int lambda; // lambda value for the USM
		double spatialSigma;
    	double spatialVariance;
    	int rangeSigma;
    	int rangeVariance;
		Mat1f exponentialFactor;
		Mat1f gaussianKernel;
		Mat1f X, Y, XY;
		Mat laplacianFilteredImage; // Laplacian of Gaussian filtered image

		Mat NonAdaptiveUSMFilter(const Mat &image);
		Mat1f LaplacianKernel();
		Mat1f GaussianKernel();
		Mat1f GaussianExponentialFactor();

	public:
		DeWAFF();
		Mat DeceivedBilateralFilter(const Mat &image);
};

#endif /* DEWAFF_H_ */
