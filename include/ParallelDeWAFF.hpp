/**
 * @file ParallelDeWAFF.hpp
 * @brief
 * @author David Prado {davidp}
 * @date 11/27/2015
 * @author Isaac Fonseca Segura {isaac-fs}
 * @date 10/28/2022
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
	enum modes : unsigned int {start = 0, image = 1, video = 2, benchmark = 3}; //Processing modes
	unsigned int mode;
	int numIter; //Number of iterations for benchmark mode
	std::string programName;
	std::string inputFileName;
	std::string outputFileName;

	void help();
	void errExit(std::string msg);
	void errHelpExit(std::string msg);

	int processVideo();
	int processImage();
	Mat processFrame(const Mat & frame);
	void displayImage(const Mat &input, const Mat &output);
};
#endif /* PARALLELDEWAFF_HPP_ */
