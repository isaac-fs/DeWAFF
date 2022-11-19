/**
 * @file ProgramInterface.hpp
 * @author Isaac Fonseca (isaac.fonsecasegura@ucr.ac.cr)
 * @date 2022-11-06
 * @author David Prado (davidp)
 * @date 2015-11-05
 *
 */

#ifndef PROGRAM_INTERFACE_HPP_
#define PROGRAM_INTERFACE_HPP_

#include <string>
#include <cstdio>
#include <iomanip>
#include <unistd.h>
#include <iostream>
#include "Utils.hpp"
#include "DeWAFF.hpp"

/**
 * @brief In charge of displaying the program and capturing the needed parameters
 *
 */
class ProgramInterface {
public:
	ProgramInterface(int argc, char** argv);
	int run();

private:
	DeWAFF framework;
	Utils lib;
	Timer timer;

	enum modes : unsigned int { // Processing modes
		start = 0,
		image = 1,
		video = 2,
		benchmark = 4};

	std::string programName;
	std::string inputFileName;
	std::string outputFileName;
	std::string::size_type dotPos;
	int benchmarkIterations; // Number of iterations for benchmark mode
	Size frameSize;
	int codec, frameCount, frameRate;
	std::string codecType;
	unsigned int mode;
	unsigned int filterType;
	int windowSize, rangeSigma, patchSize;
	double spatialSigma;


	Mat inputPreProcessor(const Mat &inputImage);
	Mat outputPosProcessor(const Mat &inputImage);
	Mat processFrame(const Mat &frame);

	void processImage();
	void processVideo();
	void benchmarkImage();
	void benchmarkVideo();

	void getVideoInfo(VideoCapture inputVideo);
	void printVideoInfo();
	void printImageInfo();

	void printBenchmarkHeader();
	void printBenchmarkFooter();

	void help();
	void errorMessage(std::string msg);
	void setOutputFileName();

	enum SPACE {
        VIDEO_DIVIDER_SPACE = 27,
        DATA_SPACE = 11,
        VALUE_SPACE = 8,
        DIVIDER_SPACE = 20,
        NUMBER_SPACE = 3,
        TIME_SPACE = 9};
};

#endif /* PROGRAM_INTERFACE_HPP_ */
