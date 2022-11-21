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
#include <getopt.h>
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
	enum programModes : unsigned int {
		start = 0, 		// 000
		image = 1, 		// 001
		video = 2, 		// 010
		benchmark = 4 	// 100
	};
	int benchmarkIterations;
	std::string programName, inputFileName, outputFileName;
	std::string::size_type dotPos;
	Size frameSize;
	int codec, frameCount, frameRate;
	std::string codecType;

	// Framework configuration
	DeWAFF framework;
	Utils lib;
	Timer timer;
	int windowSize, rangeSigma, neighborhoodSize;
	double spatialSigma;
	int filterType;
	enum filterTypes {
		DBF = 1,
		DSBF,
		DNLMF,
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
	void displayFilterParams();

	// Helper methods
	void getVideoInfo(VideoCapture inputVideo);
	void displayVideoInfo();
	void displayImageInfo();
	void displayBenchmarkHeader();
	void displayBenchmarkFooter();
	void setOutputFileName();
	void errorMessage(std::string msg);
	void help();

	// Output spacing
	enum spacing {
        MAIN_LINE = 29,
        DATA_SPACE = 11,
        VALUE_SPACE = 10,
        BENCHMARK_LINE = 20,
        NUMBER_SPACE = 3,
        TIME_SPACE = 9,
		PARAMS_LINE = 57,
		PARAM_DESC_SPACE = 17,
		PARAM_VAL_SPACE = 32
	};
};

#endif /* PROGRAM_INTERFACE_HPP_ */
