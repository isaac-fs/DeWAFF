#include "ProgramInterface.hpp"

/**
 * @brief Constructor for the ProgramInterface class. Sets all the necessary parameters for the DeWAFF processing,
 * including the ones captured from the user input in the terminal
 * @param argc argument count from the terminal
 * @param argv arguments from the terminal
 */
ProgramInterface::ProgramInterface(int argc, char** argv): framework(DeWAFF()), lib(Utils()), timer(Timer()) {
	// Incomplete command catch
	if (argc == 1) help();
	else if((mode & benchmark) && argc == 3) errorMessage("Incomplete command. Use the -h flag to see the options");

	programName = argv[0];
	mode = start;
	benchmarkIterations = 0;

	filterType = DeWAFF::DGF;
	windowSize = 11;
	spatialSigma = windowSize / 1.5;
	rangeSigma = 10;
	patchSize = windowSize/2;

	// Check the windows size
    if (windowSize < 3 || windowSize % 2 == 0) {
        std::cout << "Window size must be equal or greater than 3 and an odd number" << std::endl;
        exit(-1);
    }

	// Check the patch size (used in NLM)
    if (patchSize > windowSize) {
        std::cout << "Patch size must be smaller than the window size" << std::endl;
        exit(-1);
    }

	if (patchSize < 3 || patchSize % 2 == 0) {
		std::cout << "Patch size must be an odd number equal or greater than 3" << std::endl;
		exit(-1);
	}

	int c;
    while ((c = getopt(argc,argv,"hb:i:v:f:")) != -1) {
		switch(c) {
			case 'i': // Process an image
				if(mode & video) errorMessage("Options -v and -i are mutually exclusive");
				mode |= image;
				inputFileName = optarg;
				break;
			case 'v': // Process a video
				if(mode & image) errorMessage("Options -v and -i are mutually exclusive");
				mode |= video;
				inputFileName = optarg;
				break;
			case 'b': // Enable benchmark mode
				mode |= benchmark;
				if(optarg) {
					benchmarkIterations = atoi(optarg);
					if(benchmarkIterations < 1) errorMessage("The number of benchmark iterations [N] needs to be 1 or greater");
				}
				break;
			case 'f': // Filters type
				if(optarg) filterType = (unsigned int) atoi(optarg);
				break;
			case 'h':
				help();
				exit(-1);
				break;
			case '?':
				help();
				exit(-1);
		        break;
			default:
				abort();
		}
    }

	// Get the las dot position in the file name
	this->dotPos = inputFileName.find_last_of('.');
	// Set the output file name
	setOutputFileName();
}

/**
 * @brief Starts the program execution
 */
int ProgramInterface::run() {
	switch (mode) {
	case image:
		processImage();
		return 1;
		break;
	case image | benchmark:
		benchmarkImage();
		return 1;
		break;
	case video:
		processVideo();
		return 1;
		break;
	case video | benchmark:
		benchmarkVideo();
		return 1;
		break;
	default:
		errorMessage("Unknown issue. Could not run program");
		return -1;
		break;
	}
}

/**
 * @brief Pre processes the input. This includes size checking and type checking.
 * It converts the input to a CIELab format for further processing
 *
 * @param inputImage
 * @return Mat
 */
Mat ProgramInterface::inputPreProcessor(const Mat &inputImage) {
    // Input checking
    int type = inputImage.type();
	double minVal, maxVal;
	lib.MinMax(inputImage, &minVal, &maxVal);
	if (!(type == CV_8UC1 || type == CV_8UC3) || minVal < 0 || maxVal > 255)
	   errorMessage("Input frame must be a Grayscale or RGB unsigned integer matrix of size NxMx1 or NxMx3 on the closed interval [0,255]");

    // Converto to CIELab color space
	Mat input;
    inputImage.convertTo(input, CV_32F, 1.0/255.0); // The image has to to have values from 0 to 1 before convertion to CIELab
	cvtColor(input, input, COLOR_BGR2Lab); // Convert normalized BGR image to CIELab color space.

	return input;
}

/**
 * @brief Pos processes the output. It converts the filtered image to its original format
 *
 * @param input
 * @return Mat
 */
Mat ProgramInterface::outputPosProcessor(const Mat &input) {
	Mat output;
    // Convert filtered image back to BGR color space
	cvtColor(input, output, COLOR_Lab2BGR);
	//Scale back to [0,255]
    output.convertTo(output, CV_8U, 255);

	return output;
}

/**
 * @brief Process a frame from an image or a video in the chosen DeWAFF filter
 * @param inputFrame Input frame
 * @return Processed frame
 */
Mat ProgramInterface::processFrame(const Mat &inputFrame) {
	// Process frame
	Mat input = inputPreProcessor(inputFrame);
	Mat output;
	switch (filterType) {
	case DeWAFF::DBF:
		output = framework.DeceivedBilateralFilter(input, windowSize, spatialSigma, rangeSigma);
		break;
	case DeWAFF::DSBF:
		output = framework.DeceivedScaledBilateralFilter(input, windowSize, spatialSigma, rangeSigma);
		break;
	case DeWAFF::DNLM:
		output = framework.DeceivedNonLocalMeansFilter(input, windowSize, patchSize, spatialSigma, rangeSigma);
		break;
	case DeWAFF::DGF:
		output = framework.DeceivedGuidedFilter(input, windowSize, spatialSigma, rangeSigma);
		break;
	default:
		help();
		break;
	}

	return outputPosProcessor(output);
}


/**
 * @brief Processes an image file
 *
 */
void ProgramInterface::processImage() {
	Mat inputFrame = imread(inputFileName);
	if(inputFrame.empty()) errorMessage("Could not open the input file for read: " + inputFileName);

	frameSize = inputFrame.size();
	printImageInfo();

	// Process image
	Mat outputFrame;
	outputFrame = this->processFrame(inputFrame);

	if(!imwrite(outputFileName, outputFrame)) errorMessage("Could not open the output file for write: " + outputFileName);

	// Display exit
	std::cout << "Processing done" << std::endl;
}

/**
 * @brief Processes a video file
 *
 */
void ProgramInterface::processVideo() {
	// Open input video file
	VideoCapture inputVideo = VideoCapture(inputFileName);
	if (!inputVideo.isOpened()) errorMessage("Could not open the input video for read: " + inputFileName);

	// Acquire input video information
	getVideoInfo(inputVideo);

	// Print collected video information
	printVideoInfo();

	// Open output video
	VideoWriter outputVideo(outputFileName, codec, frameRate , frameSize, true);
    if(!outputVideo.isOpened()) errorMessage("Could not open the output video for write: " + outputFileName);

	// Read one frame at a time
	Mat inputFrame, outputFrame;
	while(inputVideo.read(inputFrame)) {
		// Process current frame
		outputFrame = this->processFrame(inputFrame);

		// Write frame to output video
		outputVideo.write(outputFrame);
	}

	// Release video resources
	inputVideo.release();
	outputVideo.release();

	// Display exit
	std::cout << "Processing done" << std::endl;
}

/**
 * @brief Benchmarks an image
 *
 */
void ProgramInterface::benchmarkImage() {
		Mat inputFrame = imread(inputFileName);
		if(inputFrame.empty()) errorMessage("Could not open the input file for read: " + inputFileName);

		frameSize = inputFrame.size();
		printImageInfo();

		double elapsedSeconds;

		printBenchmarkHeader();
		for(int i = 1; i <= benchmarkIterations; i++) {
			timer.start();

			processFrame(inputFrame);

			elapsedSeconds = timer.stop();

			// Print results
			std::cout << "| "
			<< std::left << std::setw(NUMBER_SPACE) << i
			<< " | "
			<< std::left << std::setw(TIME_SPACE) << elapsedSeconds
			<< " |";
			if(i != benchmarkIterations) std::cout << std::endl;
		}
		printBenchmarkFooter();
}

/**
 * @brief Benchmarks a video
 *
 */
void ProgramInterface::benchmarkVideo() {
	VideoCapture inputVideo = VideoCapture(inputFileName);
	if (!inputVideo.isOpened()) errorMessage("Could not open the input video for read: " + inputFileName);

	// Acquire input video information
	getVideoInfo(inputVideo);

	// Print collected video information
	printVideoInfo();

	Mat inputFrame, outputFrame;
	double elapsedSeconds;

	printBenchmarkHeader();
	for(int i = 1; i <= benchmarkIterations; i++) {
		// Start timer
		timer.start();

		// Read one frame at a time
		while(inputVideo.read(inputFrame)) {
			// Process current frame
			processFrame(inputFrame);
		}

		// Update timer
		elapsedSeconds = timer.stop();

		// Print results
		std::cout << "| "
		<< std::left << std::setw(NUMBER_SPACE) << i
		<< " | "
		<< std::left << std::setw(TIME_SPACE) << elapsedSeconds << std::setw(8)
		<< " |";
		if(i != benchmarkIterations) std::cout << std::endl;

		// Update files for a new iteration
		inputVideo = VideoCapture(inputFileName);
	}
	printBenchmarkFooter();
}

/**
 * @brief Sets the video information
 *
 */
void ProgramInterface::getVideoInfo(VideoCapture inputVideo) {
	frameRate = static_cast<int>(inputVideo.get(cv::CAP_PROP_FPS));
    frameCount = static_cast<int>(inputVideo.get(cv::CAP_PROP_FRAME_COUNT));
    frameSize = Size((int) inputVideo.get(cv::CAP_PROP_FRAME_WIDTH),(int) inputVideo.get(cv::CAP_PROP_FRAME_HEIGHT));

    // Transform from int to char via Bitwise operators
    int codec = static_cast<int>(inputVideo.get(cv::CAP_PROP_FOURCC));
    char EXT[] = {	static_cast<char> ((codec & 0XFF)),
					static_cast<char> ((codec & 0XFF00) >> 8),
					static_cast<char> ((codec & 0XFF0000) >> 16),
					static_cast<char> ((codec & 0XFF00000) >> 24),0};
    codecType = EXT;
}

/**
 * @brief Prints the video information
 *
 */
void ProgramInterface::printVideoInfo() {
	std::cout << "\nInput video information" << std::endl;
	std::cout << std::setw(VIDEO_LINE) << std::setfill('-') << '\n' << std::setfill(' ');
	std::cout << "| "
	<< std::left << std::setw(DATA_SPACE) << "Data"
	<< " | "
	<< std::left << std::setw(VALUE_SPACE+1) << "Value"
	<< "|";
	std::cout << std::setw(VIDEO_LINE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
	std::ostringstream stringStream;
	stringStream << frameSize.width << "x" << frameSize.height;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Resolution"  << " | "  << std::setw(VALUE_SPACE) << std::left << stringStream.str()	<< " |" << std::endl;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Frame count" << " | "  << std::setw(VALUE_SPACE) << std::left << frameCount			<< " |" << std::endl;
	stringStream.str(""); stringStream.clear();
	stringStream << frameRate << " fps";
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Frame rate"  << " | "  << std::setw(VALUE_SPACE) << std::left << stringStream.str()	<< " |" << std::endl;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Codec type"  << " | "  << std::setw(VALUE_SPACE) << std::left << codecType			<< " |";
	std::cout << std::setw(VIDEO_LINE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
}

/**
 * @brief Prints the image information
 *
 */
void ProgramInterface::printImageInfo() {
	// Get the image extension
	std::string imageExtension = inputFileName.substr(dotPos+1);
	std::transform(imageExtension.begin(), imageExtension.end(),imageExtension.begin(), ::toupper);

	// Print image information
	std::cout << "\nInput image information" << std::endl;
	std::cout << std::setw(VIDEO_LINE) << std::setfill('-') << '\n' << std::setfill(' ');
	std::cout << "| "
	<< std::left << std::setw(DATA_SPACE) << "Data"
	<< " | "
	<< std::left << std::setw(VALUE_SPACE+1) << "Value"
	<< "|";
	std::cout << std::setw(VIDEO_LINE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
	std::ostringstream stringStream;
	stringStream << frameSize.width << "x" << frameSize.height;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Resolution"  << " | "  << std::setw(VALUE_SPACE) << std::left << stringStream.str()	<< " |" << std::endl;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Image Type" << " | " 	<< std::setw(VALUE_SPACE) << std::left << imageExtension		<< " |";
	std::cout << std::setw(VIDEO_LINE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;

}

/**
 * @brief Prints the benchmark header
 *
 */
void ProgramInterface::printBenchmarkHeader() {
	// Print header
	std::cout << std::internal <<"\nBenchmark mode" << std::endl;
	std::cout << std::setw(BENCHMARK_LINE) << std::setfill('-') << '\n' << std::setfill(' ');
	std::cout << "| "
	<< std::left << std::setw(NUMBER_SPACE) << "N"
	<< " | "
	<< std::left << std::setw(TIME_SPACE+1) << "Time [s]"
	<< "|";
	std::cout << std::setw(BENCHMARK_LINE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
}

/**
 * @brief Prints the benchmark footer
 *
 */
void ProgramInterface::printBenchmarkFooter() {
	std::cout << std::setw(BENCHMARK_LINE) << std::setfill('-') << '\n' << std::setfill(' ') << '\n' << std::endl;
}

/**
 * @brief Displays the program's help
 */
void ProgramInterface::help() {
	// Print header
	std::cout << std::internal <<"\nHelp for " << this->programName << std::endl;

	std::cout << std::setw(HELP_SPACE) << std::setfill('-') << '\n' << std::setfill(' ');
	std::cout << "| "
	<< std::setw(HELP_OPTION) << std::left << "Options" << " | " << std::setw(HELP_DESCR) << std::left << "Description" << " |"
	<< std::setw(HELP_SPACE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl

	<< "| "
	<< std::setw(HELP_OPTION) << std::left << "-i"
	<< " | "
	<< std::setw(HELP_DESCR) << "Process an image given a file name -i <file name>"
	<< " |"
	<< std::endl

	<< "| "
	<< std::setw(HELP_OPTION) << std::left << "-v"
	<< " | "
	<< std::setw(HELP_DESCR) << "Process a video given a file name -v <file name>"
	<< " |"
	<< std::endl

	<< "| "
	<< std::setw(HELP_OPTION) << std::left << "-b < N >"
	<< " | "
	<< std::setw(HELP_DESCR) << "Run a series of N benchmarks for a video or an image."
	<< " |"
	<< std::endl

	<< "| "
	<< std::setw(HELP_OPTION) << std::left << "-h"
	<< " | "
	<< std::setw(HELP_DESCR) << "Display this help message"
	<< " |"
	<< std::setw(HELP_SPACE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
}

/**
 * @brief Displays an error message and exits the program
 * @param msg Error message
 */
void ProgramInterface::errorMessage(std::string msg) {
	std::cerr << "ERROR: " << msg << std::endl;
	exit(-1);
}

/**
 * @brief Sets the output file name with the corresponding applied filter acronym
 *
 */
void ProgramInterface::setOutputFileName() {
	std::string filterAcronym;
	switch (filterType) {
	case DeWAFF::DBF:
		filterAcronym = "DBF";
		break;
	case DeWAFF::DSBF:
		filterAcronym = "DSBF";
		break;
	case DeWAFF::DNLM:
		filterAcronym = "DNLM";
		break;
	case DeWAFF::DGF:
		filterAcronym = "DGF";
		break;
	default:
		filterAcronym = "";
		break;
	}

	std::string extension;
	if (mode & image) extension = ".png";
	else if(mode & video) extension = ".avi";

	// Set the output file names
	this->outputFileName = this->inputFileName.substr(0, dotPos) + "_" + filterAcronym + extension;
}