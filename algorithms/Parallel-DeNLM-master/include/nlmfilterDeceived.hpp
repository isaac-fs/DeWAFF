/*
 * nlmfilterDeceived.h
 *
 *  Created on: Jun 20, 2016
 *      Author: davidp, manzumbado
 */

#ifndef NLMFILTERDECEIVED_H_
#define NLMFILTERDECEIVED_H_

#include <iostream>
#include <omp.h>
#include "tools.hpp"

using namespace cv;
using namespace std;

class NLMFilterDeceived{

public:
	Mat nlmfilterDeceived(const Mat& A, const Mat& L, int w, int w_n, double sigma_s, int sigma_r);

private:
	Mat nlmfilBW_deceived(const Mat& A, const Mat& L, int w, int w_n, double sigma_d, int sigma_r);
	Mat nlmfltBWDeceived(const Mat& A, const Mat& L, int w, int w_n, double sigma_d, int sigma_r);
    Mat CalcEuclideanDistMat(const Mat& I, int w_n, int i, int j, int iMin, int jMin);
};
#endif /* NLMFILTERDECEIVED_H_ */
