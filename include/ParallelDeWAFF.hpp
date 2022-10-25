/*
 * ParallelDeWAFF.hpp
 *
 *  Created on: Nov 5, 2015
 *      Author: davidp
 */

#ifndef PARALLELDEWAFF_HPP_
#define PARALLELDEWAFF_HPP_

#include <string>
#include <unistd.h>
#include "DeWAFF.hpp"
#include "Laplacian.hpp"
#include "Timer.h"
using namespace std;

class ParallelDeWAFF{
public:
	ParallelDeWAFF(int argc, char* argv[]);
	int start();

private:
	unsigned int mode;		//Processing mode 00000001 = Video, 00000010 = Image,  00000101 = Video Benchmark, 00000110 = Image Benchmark
	int numIter;			//Number of iterations for benchmark
	string progName;		//This program name
	Mat mask; 				//A Laplacian of Gaussian mask
	string inputFile;
	string outputFile;

	void help();
	void errExit(string msg);
	void errHelpExit(string msg);

	int processVideo();
	int processImage();
	Mat processFrame(const Mat & frame);
};
#endif /* PARALLELDEWAFF_HPP_ */
