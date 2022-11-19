#include "ProgramInterface.hpp"

/**
 * @brief Constructor for the ProgramInterface class
 * @param argc argument count from the terminal
 * @param argv arguments from the terminal
 */
ProgramInterface::ProgramInterface(int argc, char** argv) {
	// Params
	this->filterType = DGF;
	this->windowSize = 15;
    this->spatialSigma = windowSize/1.5;
    this->rangeSigma = 10;
	this->patchSize = 3;

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


	// Inner params
	this->mode = start;
	this->benchmarkIterations = 0;
	this->programName = argv[0];

	int c;
    while ((c = getopt(argc,argv,"hb:i:v:f:")) != -1) {
		switch(c) {
			case 'i': // Process an image
				if(this->mode & video) // Check if flag for video enabled
					this->errorMessage("Options -v and -i are mutually exclusive");
				this->mode |= image;
				this->inputFileName = optarg;
				break;
			case 'v': // Process a video
				if(this->mode & image) // Check if flag for image enabled
					this->errorMessage("Options -v and -i are mutually exclusive");
				this->mode |= video;
				this->inputFileName = optarg;
				break;
			case 'b': // Enable benchmark mode
				this->mode |= benchmark;
				if(optarg){
					this->benchmarkIterations = atoi(optarg);
					if(benchmarkIterations < 1) errorMessage("The number of benchmark iterations [N] needs to be 1 or greater");
				}
				break;
			case 'f': // Filter type
				if(optarg) this->filterType = atoi(optarg);
			break;
			case 'h':
				this->help();
				exit(-1);
				break;
			case '?':
				this->help();
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

	// Incomplete command catch
	if (argc == 1) this->help();
	else if((this->mode & benchmark) && argc == 3)
		this->errorMessage("Incomplete command. Use the -h flag to see the options");
}

/**
 * @brief Start program execution
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


Mat ProgramInterface::inputPreProcessor(const Mat &inputImage) {
    // Input checking
    int type = inputImage.type();
	double minVal, maxVal;
	Utils::MinMax(inputImage, &minVal, &maxVal);
	if (!(type == CV_8UC1 || type == CV_8UC3) || minVal < 0 || maxVal > 255)
	   errorMessage("Input frame must be a Grayscale or RGB unsigned integer matrix of size NxMx1 or NxMx3 on the closed interval [0,255]");

    // Converto to CIELab color space
	Mat input;
    inputImage.convertTo(input, CV_32F, 1.0/255.0); // The image has to to have values from 0 to 1 before convertion to CIELab
	cvtColor(input, input, COLOR_BGR2Lab); // Convert normalized BGR image to CIELab color space.

	return input;
}

Mat ProgramInterface::outputPosProcessor(const Mat &input) {
	Mat output;
    // Convert filtered image back to BGR color space
	cvtColor(input, output, COLOR_Lab2BGR);
	//Scale back to [0,255]
    output.convertTo(output, CV_8U, 255);

	return output;
}

/**
 * @brief Process a frame from an image or a video. Input frame must be BGR from 0 to 255
 * @param inputFrame Input frame
 * @return Processed frame
 */
Mat ProgramInterface::processFrame(const Mat &inputFrame) {
	// Process frame
	Mat input = inputPreProcessor(inputFrame);
	Mat output;
	switch (filterType) {
	case DBF:
		output = DeceivedBilateralFilter(input, windowSize, spatialSigma, rangeSigma);
		break;
	case DSBF:
		output = DeceivedScaledBilateralFilter(input, windowSize, spatialSigma, rangeSigma);
		break;
	case DNLM:
		output = DeceivedNonLocalMeansFilter(input, windowSize, patchSize, spatialSigma, rangeSigma);
		break;
	case DGF:
		output = output = DeceivedGuidedFilter(input, windowSize, spatialSigma, rangeSigma);
		break;
	default:
		help();
		break;
	}

	return outputPosProcessor(output);
}


/**
 * @brief Process an image file
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
 * @brief Process a video file
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

void ProgramInterface::getVideoInfo(VideoCapture inputVideo) {
	frameRate = static_cast<int>(inputVideo.get(cv::CAP_PROP_FPS));
    frameCount = static_cast<int>(inputVideo.get(cv::CAP_PROP_FRAME_COUNT));
    frameSize = Size((int) inputVideo.get(cv::CAP_PROP_FRAME_WIDTH),(int) inputVideo.get(cv::CAP_PROP_FRAME_HEIGHT));

    // Transform from int to char via Bitwise operators
    int codec = static_cast<int>(inputVideo.get(cv::CAP_PROP_FOURCC));
    char EXT[] = {	(char)((codec & 0XFF)),
					(char)((codec & 0XFF00) >> 8),
					(char)((codec & 0XFF0000) >> 16),
					(char)((codec & 0XFF000000) >> 24),
					0
				};
    codecType = EXT;
}

void ProgramInterface::printVideoInfo() {
	std::cout << "\nInput video information" << std::endl;
	std::cout << std::setw(VIDEO_DIVIDER_SPACE) << std::setfill('-') << '\n' << std::setfill(' ');
	std::cout << "| "
	<< std::left << std::setw(DATA_SPACE) << "Data"
	<< " | "
	<< std::left << std::setw(VALUE_SPACE+1) << "Value"
	<< "|";
	std::cout << std::setw(VIDEO_DIVIDER_SPACE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
	std::ostringstream stringStream;
	stringStream << frameSize.width << "x" << frameSize.height;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Resolution"  << " | "  << std::setw(VALUE_SPACE) << std::left << stringStream.str()	<< " |" << std::endl;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Frame count" << " | "  << std::setw(VALUE_SPACE) << std::left << frameCount			<< " |" << std::endl;
	stringStream.str(""); stringStream.clear();
	stringStream << frameRate << " fps";
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Frame rate"  << " | "  << std::setw(VALUE_SPACE) << std::left << stringStream.str()	<< " |" << std::endl;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Codec type"  << " | "  << std::setw(VALUE_SPACE) << std::left << codecType			<< " |";
	std::cout << std::setw(VIDEO_DIVIDER_SPACE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
}

void ProgramInterface::printImageInfo() {
	// Get the image extension
	std::string imageExtension = inputFileName.substr(dotPos+1);
	std::transform(imageExtension.begin(), imageExtension.end(),imageExtension.begin(), ::toupper);

	// Print image information
	std::cout << "\nInput image information" << std::endl;
	std::cout << std::setw(VIDEO_DIVIDER_SPACE) << std::setfill('-') << '\n' << std::setfill(' ');
	std::cout << "| "
	<< std::left << std::setw(DATA_SPACE) << "Data"
	<< " | "
	<< std::left << std::setw(VALUE_SPACE+1) << "Value"
	<< "|";
	std::cout << std::setw(VIDEO_DIVIDER_SPACE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
	std::ostringstream stringStream;
	stringStream << frameSize.width << "x" << frameSize.height;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Resolution"  << " | "  << std::setw(VALUE_SPACE) << std::left << stringStream.str()	<< " |" << std::endl;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Image Type" << " | " 	<< std::setw(VALUE_SPACE) << std::left << imageExtension		<< " |";
	std::cout << std::setw(VIDEO_DIVIDER_SPACE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;

}

void ProgramInterface::printBenchmarkHeader() {
	// Print header
	std::cout << std::internal <<"\nBenchmark mode" << std::endl;
	std::cout << std::setw(DIVIDER_SPACE) << std::setfill('-') << '\n' << std::setfill(' ');
	std::cout << "| "
	<< std::left << std::setw(NUMBER_SPACE) << "N"
	<< " | "
	<< std::left << std::setw(TIME_SPACE+1) << "Time [s]"
	<< "|";
	std::cout << std::setw(DIVIDER_SPACE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
}

void ProgramInterface::printBenchmarkFooter() {
	std::cout << std::setw(DIVIDER_SPACE) << std::setfill('-') << '\n' << std::setfill(' ') << '\n' << std::endl;
}

/**
 * @brief Display program help
 */
void ProgramInterface::help() {
	// Print header
	std::cout << std::internal <<"\nHelp for " << this->programName << std::endl;

	std::cout << std::setw(68) << std::setfill('-') << '\n' << std::setfill(' ');
	std::cout << "| "
	<< std::setw(6) << std::left << "Options" << " | " << std::setw(54) << std::left << "Description" << " |"
	<< std::setw(68) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl

	<< "| "
	<< std::setw(6) << std::left << "-i"
	<< " | "
	<< std::setw(55) << "Process an image given a file name -i <file name>"
	<< " |"
	<< std::endl

	<< "| "
	<< std::setw(6) << std::left << "-v"
	<< " | "
	<< std::setw(55) << "Process a video given a file name -v <file name>"
	<< " |"
	<< std::endl

	<< "| "
	<< std::setw(6) << std::left << "-b <N>"
	<< " | "
	<< std::setw(55) << "Run a series of <N> benchmarks for a video or an image."
	<< " |"
	<< std::endl

	<< "| "
	<< std::setw(6) << std::left << "-h"
	<< " | "
	<< std::setw(55) << "Display this help message"
	<< " |"
	<< std::setw(68) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl

	<< std::setw(68) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl
	<< "| " << std::setw(64) << std::left << "Examples" << " |"
	<< std::setw(68) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl

	<< "| "
	<< std::setw(64) << std::left << "-> Run an image benchmark for 10 iterations: " << " |" << std::endl
	<< "| ";
	std::ostringstream stringStream;
	stringStream << this->programName << " -i someFile.png -b 10";
	std::cout << std::setw(64) << std::left << stringStream.str() << " |"
	<< std::setw(68) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl

	<< "| "
	<< std::setw(64) << std::left << "-> Process a video: " << " |" << std::endl
	<< "| ";
	stringStream.str(""), stringStream.clear();
	stringStream << this->programName << " -v someFile.mp4";
	std::cout << std::setw(64) << std::left << stringStream.str() << " |"
	<< std::setw(68) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl
	<< std::endl;
}

/**
 * @brief Display an error message
 * @param msg Error message
 */
void ProgramInterface::errorMessage(std::string msg) {
	std::cerr << "ERROR: " << msg << std::endl;
	exit(-1);
}

void ProgramInterface::setOutputFileName() {
	std::string filterAcronym;
	switch (filterType) {
	case DBF:
		filterAcronym = "DBF";
		break;
	case DSBF:
		filterAcronym = "DSBF";
		break;
	case DNLM:
		filterAcronym = "DNLM";
		break;
	case DGF:
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