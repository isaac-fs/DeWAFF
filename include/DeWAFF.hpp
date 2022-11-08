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

class DeWAFF{
	private:
		enum LAB : int {L, a, b};
		int lambda; ///< lambda value for the USM 
		const Mat &originalImage;
		Mat USMFilteredImage;
	public:
		DeWAFF(const Mat &image);
		Mat deceivedBilateralFilter(int windowSize, double spatialSigma, int rangeSigma);
};

#endif /* DEWAFF_H_ */
