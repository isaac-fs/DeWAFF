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
#include "NonAdaptiveUSM.hpp"
#include "Timer.hpp"

class ParallelDeWAFF{
public:
	ParallelDeWAFF(int argc, char** argv);
	int start();

private:
	unsigned int mode;		//Processing mode 00000001 = Video, 00000010 = Image,  00000101 = Video Benchmark, 00000110 = Image Benchmark
	int numIter;			//Number of iterations for benchmark
	std::string programName;		//This program name
	std::string inputFile;
	std::string outputFile;

	void help();
	void errExit(std::string msg);
	void errHelpExit(std::string msg);

	int processVideo();
	int processImage();
	Mat processFrame(const Mat & frame);
	void display_img(const Mat &input, const Mat &output);
};
#endif /* PARALLELDEWAFF_HPP_ */
