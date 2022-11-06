/**
 * @file DeWAFF.cpp
 * @author David Prado (davidp)
 * @date 2015-08-29
 * @author Isaac Fonseca (isaac.fonsecasegura@ucr.ac.cr)
 * @date 2022-11-06
 * @brief 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef DEWAFF_H_
#define DEWAFF_H_

#include <iostream>
#include <omp.h>
#include "Tools.hpp"

class deWAFF{
public:
	static Mat filter(const Mat& originalImage, const Mat& USMFilteredImage, int ws, double sigma_s, int sigma_r);
};

#endif /* DEWAFF_H_ */
