/**
 * @file Filters.hpp
 * @author Manuel Zumbado
 * @author David Prado (davidp)
 * @author Juan Jose Guerrero
 * @date 2015-08-29
 * @author Isaac Fonseca (isaac.fonsecasegura@ucr.ac.cr)
 * @date 2022-11-06
 *
 */

#ifndef FILTERS_HPP_
#define FILTERS_HPP_

#include <omp.h>
#include <opencv2/opencv.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "GuidedFilter.hpp"
#include "Utils.hpp"

using namespace cv;

/**
 * @brief Class containing Weighted Average Filters (WAFs)
 *
 */
class Filters {
    private:
		enum CIELab : int {L, a, b}; // CIELab channels

    public:
        Mat BilateralFilter(const Mat &weightingImage, const Mat &inputImage, const int windowSize, const double spatialSigma, const int rangeSigma);
        Mat ScaledBilateralFilter(const Mat &weightingImage, const Mat &inputImage, const int windowSize, const double spatialSigma, const int rangeSigma);
        Mat NonLocalMeansFilter(const Mat &weightingImage, const Mat &inputImage, const int windowSize, const int patchSize, const double rangeSigma);
        Mat GuidedFilter(const Mat &inputImage, const Mat &guidingImage, const int windowSize, const int rangeSigma);
};

#endif /* FILTERS_HPP_ */