/**
 * @file Utils.hpp
 * @author Isaac Fonseca (isaac.fonsecasegura@ucr.ac.cr)
 * @date 2022-11-06
 * @author David Prado (davidp)
 * @date 2015-11-05
 *
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <algorithm>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;

/**
 * @brief Useful tools for image processing
 * These tools are statics objects to use them in the lifetime of the program
 * without the need of constinuous instantiation
 *
 */
class Utils {
    public:
        void MeshGrid(const Range &range, Mat &X, Mat &Y);
        void MinMax(const Mat& A, double* minA, double* maxA);
        Mat GaussianFunction(Mat input, double sigma);
        Mat GaussianKernel(int windowSize, double sigma);
        Mat LoGFilter(const Mat &image, int windowSize, double sigma);
        Mat NonAdaptiveUSMFilter(const Mat &image, int windowSize, double lambda, double sigma);
		Mat RegionDistancesMatrix(const Mat& image, int neighborhoodSize);
};

#endif /* UTILS_HPP_ */