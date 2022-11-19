/**
 * @file Utils.hpp
 * @author David Prado (davidp)
 * @date 2015-11-05
 * @author Isaac Fonseca (isaac.fonsecasegura@ucr.ac.cr)
 * @date 2022-11-06
 * @brief
 * @copyright Copyright (c) 2022
 *
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <algorithm>
#include <sys/time.h>
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
        static void MeshGrid(const Range &range, Mat &X, Mat &Y);
        static void MinMax(const Mat& A, double* minA, double* maxA);
        static Mat GaussianFunction(Mat input, double sigma);
        static Mat GaussianKernel(int windowSize, double sigma);
		static Mat LoGKernel(int windowSize, double sigma);
        static Mat NonAdaptiveUSM(const Mat &image, int windowSize, int lambda, double sigma);
		static Mat EuclideanDistanceMatrix(const Mat& image, int windowSize, int patchSize);
};

/**
 * @brief Class containing the timer methods for the benchmarking of file processing
 *
 */
class Timer {
    private:
        double startTime;
    public:
        void start(); // Starts timer and resets the elapsed time
        double stop(); // Stops the timer and returns the elapsed time
};

#endif /* UTILS_HPP_ */