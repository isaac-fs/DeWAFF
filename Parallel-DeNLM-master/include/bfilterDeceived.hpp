/*
 * bfilterDeceived.h
 *
 *  Created on: Oct 30, 2015
 *      Author: davidp
 */

#ifndef BFILTERDECEIVED_H_
#define BFILTERDECEIVED_H_

#include <iostream>
#include <omp.h>
#include "tools.hpp"

using namespace cv;
using namespace std;

class BFilterDeceived{

public:
	Mat bfilterDeceived(const Mat& A, const Mat& L, int w, double sigma_s, int sigma_r);

private:
	Mat bfltColorDeceived(const Mat& A, const Mat& L, int w, double sigma_d, int sigma_r);
	Mat bfil2LAB_deceived(const Mat& A, const Mat& L, int w, double sigma_d, int sigma_r);
};
#endif /* BFILTERDECEIVED_H_ */
