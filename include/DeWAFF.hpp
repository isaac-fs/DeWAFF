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
 * It applies a filter which intput and weighting input are decoupled, so it is possible
 * deceive the input and to still use the original input weighting values
 *
 */
class DeWAFF : protected Filters {
	public:
		DeWAFF();
		int usmLambda; /// Parameter for the Laplacian deceive
		Mat DeceivedBilateralFilter(const Mat &inputImage, int windowSize, double spatialSigma, int rangeSigma);
		Mat DeceivedScaledBilateralFilter(const Mat &inputImage, int windowSize, double spatialSigma, int rangeSigma);
		Mat DeceivedNonLocalMeansFilter(const Mat &inputImage, int windowSize, int patchSize, double spatialSigma, int rangeSigma);
		Mat DeceivedGuidedFilter(const Mat &inputImage, int windowSize, double spatialSigma, int rangeSigma);
};

#endif /* DEWAFF_H_ */
