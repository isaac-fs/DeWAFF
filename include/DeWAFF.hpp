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
		int padding;
		Mat image; // input image with padding for kernel consistency
		Range range;
		Mat1f X, Y, S, spatialGaussianKernel, LoGkernel;
		Mat laplacianImage; // Laplacian of Gaussian filtered image
		Mat LaplacianFilter(const Mat &inputImage);
		Mat1f LaplacianKernel();
		Mat EuclideanDistanceKernel(const Mat& imageRegion, int subWindowSize);

	protected:
		int windowSize;
		int lambda; // lambda value for the USM
		double spatialSigma, spatialVariance;
		int rangeSigma, rangeVariance;
		int neighborhoodWindowSize; // For the NLM filter

	public:
		DeWAFF(int windowSize, int lambda, double spatialSigma, int rangeSigma);
		Mat DeceivedBilateralFilter(const Mat &inputImage);
		// Mat DeceivedScaledBilateralFilter(const Mat &inputImage);
		Mat DeceivedNonLocalMeansFilter(const Mat &inputImage);
};

#endif /* DEWAFF_H_ */
