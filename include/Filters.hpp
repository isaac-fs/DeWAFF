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

using namespace cv;

/**
 * @brief Class containing Weighted Average Filters (WAFs). This implementation relies on padding the original image to fit
 * square odd dimensioned kernels throughout the processing
 *
 */
class Filters {
    private:
        enum CIELab : int {L, a, b}; // CIELab channels

    protected:
        Utils lib;

    public:
        Mat BilateralFilter(const Mat &weightingImage, const Mat &inputImage, const int windowSize, const double spatialSigma, const int rangeSigma);
        Mat ScaledBilateralFilter(const Mat &weightingImage, const Mat &inputImage, const int windowSize, const double spatialSigma, const int rangeSigma);
        Mat NonLocalMeansFilter(const Mat &weightingImage, const Mat &inputImage, const int windowSize, const int patchSize, const double rangeSigma);
        Mat GuidedFilter(const Mat &inputImage, const Mat &guidingImage, const int windowSize, const int rangeSigma);
};

/**
 * @file Filters.hpp
 * @author Atılım Çetin
 * @author Nikolai Poliarnyi
 * @brief Guided filter implementation from https://github.com/atilimcetin/guided-filter
 * @brief An open source OpenCV guided filter implementation under the MIT license
 * @date 2020-06-1
 */
class GuidedFilterImpl;

class GuidedFilter {
public:
    GuidedFilter(const cv::Mat &I, int r, double eps);
    ~GuidedFilter();

    cv::Mat filter(const cv::Mat &p, int depth = -1) const;

private:
    GuidedFilterImpl *impl_;
};

cv::Mat guidedFilter(const cv::Mat &I, const cv::Mat &p, int r, double eps, int depth = -1);


#endif /* FILTERS_HPP_ */