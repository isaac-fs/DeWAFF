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
#include "NonAdaptiveUSM.hpp"
#include "Tools.hpp"

using namespace cv;

/**
 * @brief Deceived Weighted Averaged Filter Framework class.
 * It applies filter using the deceived weighted averaged values of an image
 */
class DeWAFF{
	private:
		enum CIELab : int {L, a, b}; // CIELab channels
		int lambda; // lambda value for the USM
		const Mat &originalImage; // Reference to the image to process
		Mat USMFilteredImage; // Laplacian of Gaussian filtered image
	public:
		DeWAFF(const Mat &image);
		Mat DeceivedBilateralFilter(int windowSize, double spatialSigma, int rangeSigma);
};

#endif /* DEWAFF_H_ */
