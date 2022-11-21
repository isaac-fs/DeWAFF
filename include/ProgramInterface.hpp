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
	// Class configuration
	unsigned int mode;
	int benchmarkIterations;
	enum programModes : unsigned int {
		start = 0, 		// 000
		image = 1, 		// 001
		video = 2, 		// 010
		benchmark = 4 	// 100
	};
	std::string programName, inputFileName, outputFileName;
	std::string::size_type dotPos;
	Size frameSize;
	int codec, frameCount, frameRate;
	std::string codecType;

	// Framework configuration
	DeWAFF framework;
	Utils lib;
	Timer timer;
	int windowSize, rangeSigma, patchSize;
	double spatialSigma;
	unsigned int filterType;
	enum filterTypes : unsigned int {
		DBF,
		DSBF,
		DNLM,
		DGF
	};

	// Input processing
	Mat inputPreProcessor(const Mat &inputImage);
	Mat outputPosProcessor(const Mat &inputImage);
	Mat processFrame(const Mat &frame);
	void processImage();
	void processVideo();
	void benchmarkImage();
	void benchmarkVideo();

	// Helper methods
	void getVideoInfo(VideoCapture inputVideo);
	void printVideoInfo();
	void printImageInfo();
	void printBenchmarkHeader();
	void printBenchmarkFooter();
	void setOutputFileName();
	void errorMessage(std::string msg);
	void help();

	// Output spacing
	enum spacing {
        VIDEO_LINE = 27,
        DATA_SPACE = 11,
        VALUE_SPACE = 8,
        BENCHMARK_LINE = 20,
        NUMBER_SPACE = 3,
        TIME_SPACE = 9,
		HELP_SPACE = 69,
		HELP_OPTION = 8,
		HELP_DESCR = 53
	};
};

#endif /* PROGRAM_INTERFACE_HPP_ */
