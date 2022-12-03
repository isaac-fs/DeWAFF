/**
 * @file DeWAFF.hpp
 * @author Isaac Fonseca (isaac.fonsecasegura@ucr.ac.cr)
 * @date 2022-11-06
 *
 */

#ifndef DEWAFF_H_
#define DEWAFF_H_

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "Utils.hpp"
#include "Filters.hpp"

using namespace cv;

/**
 * @brief Deceived Weighted Average Filters Framework class.
 * It applies a filter where the intput is decoupled in to a weighting input and a main input
 * so it is possible to deceive the filter with a manipulated input and to still use the original
 * input weighting values for the processing
 */
class DeWAFF : protected Filters {
	public:
		DeWAFF();
		double usmLambda; /// Parameter for the Laplacian deceive
		Mat DeceivedBilateralFilter(const Mat &inputImage, int windowSize, double spatialSigma, double rangeSigma);
		Mat DeceivedScaledBilateralFilter(const Mat &inputImage, int windowSize, double spatialSigma, double rangeSigma);
		Mat DeceivedNonLocalMeansFilter(const Mat &inputImage, int windowSize, int neighborhoodSize, double spatialSigma, double rangeSigma);
		Mat DeceivedGuidedFilter(const Mat &inputImage, int windowSize, double spatialSigma, double rangeSigma);
};

#endif /* DEWAFF_H_ */
