/**
 * @file Filters.hpp
 * @author Isaac Fonseca (isaac.fonsecasegura@ucr.ac.cr)
 * @date 2022-11-06
 * @author Manuel Zumbado
 * @author David Prado (davidp)
 * @author Juan Jose Guerrero
 * @date 2015-08-29
 *
 */

#ifndef FILTERS_HPP_
#define FILTERS_HPP_

#include <omp.h>
#include <opencv2/opencv.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "Utils.hpp"
#include "GuidedFilter.hpp"

using namespace cv;

/**
 * @brief Class containing Weighted Average Filters (WAFs). This implementation relies on padding the original image to fit
 * square odd dimensioned kernels throughout the processing
 *
 */
class Filters {
    private:
        enum CIELab : int {L, a, b}; // CIELab channels
        Utils utilsLib;

    public:
        Mat BilateralFilter(const Mat &inputImage, const Mat &weightingImage, int windowSize, double spatialSigma, double rangeSigma);
        Mat ScaledBilateralFilter(const Mat &inputImage, const Mat &weightingImage, int windowSize, double spatialSigma, double rangeSigma);
        Mat NonLocalMeansFilter(const Mat &inputImage, const Mat &weightingImage, int windowSize, int neighborhoodSize, double rangeSigma);
        Mat GuidedFilter(const Mat &inputImage, const Mat &guidingImage, int windowSize, double rangeSigma);
};

#endif /* FILTERS_HPP_ */