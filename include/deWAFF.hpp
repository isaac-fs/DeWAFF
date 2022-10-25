/*
 * bfilterDeceived.h
 *
 *  Created on: Oct 30, 2015
 *      Author: davidp
 */

#ifndef DEWAFF_H_
#define DEWAFF_H_

#include <iostream>
#include <omp.h>
#include "tools.hpp"

using namespace std;

class deWAFF{
public:
	static Mat filter(const Mat& A, const Mat& L, int w, double sigma_s, int sigma_r);
};

#endif /* DEWAFF_H_ */
