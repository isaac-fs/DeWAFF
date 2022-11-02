/**
 * @file NonAdaptiveUSM.h
 * @author David Prado{davidp}
 * @date 11/1/2015
 * @author Isaac Fonseca Segura {isaac-fs}
 * @date 10/28/2022
 */

#ifndef NON_ADAPTIVE_USM_HPP_
#define NON_ADAPTIVE_USM_HPP_

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "Tools.hpp"

using namespace cv;

class NonAdaptiveUSM{

public:
	static Mat nonAdaptiveUSM(const Mat& A, const int lambda);
	static Mat LoGkernel(int size, double sigma);
};
#endif /* NON_ADAPTIVE_USM_HPP */
