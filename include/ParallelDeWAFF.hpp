/**
 * @file ParallelDeWAFF.hpp
 * @author David Prado (davidp)
 * @date 2015-11-05
 * @author Isaac Fonseca (isaac.fonsecasegura@ucr.ac.cr)
 * @date 2022-11-06
 * @brief 
 * @copyright Copyright (c) 2022
 * 
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
	int execute();

private:
	enum modes : unsigned int {start = 0, image = 1, video = 2, benchmark = 4}; //Processing modes
	unsigned int mode;
	int numIter; //Number of iterations for benchmark mode
	std::string programName;
	std::string inputFileName;
	std::string outputFileName;
	Timer timer;

	void help();
	void errExit(std::string msg);
	void errHelpExit(std::string msg);

	int processVideo();
	int processImage();
	Mat processFrame(const Mat & frame);
	void displayImage(const Mat &input, const Mat &output);
};
#endif /* PARALLELDEWAFF_HPP_ */
