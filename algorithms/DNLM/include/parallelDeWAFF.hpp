/*
 * ParallelDeWAFF.hpp
 *
 *  Created on: Nov 5, 2015
 *      Author: davidp
 */

#ifndef PARALLELDEWAFF_HPP_
#define PARALLELDEWAFF_HPP_

#include <string>
#include "bfilterDeceived.hpp"
#include "nlmfilterDeceived.hpp"
#include "noAdaptiveLaplacian.hpp"
using namespace std;

class ParallelDeWAFF{
public:
	void help();
	Mat processImage(const Mat& U, double sigma_r, double sigma_s, double lambda);
	NoAdaptiveLaplacian* getNAL();

private:
	NoAdaptiveLaplacian nal;
	BFilterDeceived bfd;
	NLMFilterDeceived nlmfd;

	Mat filterDeceivedNLM(const Mat& U, int wSize, int wSize_n, double sigma_s, int sigma_r, int lambda);
};
#endif /* PARALLELDEWAFF_HPP_ */
