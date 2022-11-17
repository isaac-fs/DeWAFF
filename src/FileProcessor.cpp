#include "FileProcessor.hpp"

int windowSize_ = 15;
int lambda_ = 2;
double spatialSigma_ = windowSize_/1.5;
int rangeSigma_ = 2;

/**
 * @brief Construct a new File Processor:: File Processor object
 * It initializes the DeWAFF constructor
 *
 */
FileProcessor::FileProcessor() : DeWAFF(windowSize = windowSize_, lambda = lambda_, spatialSigma = spatialSigma_, rangeSigma = rangeSigma_) {
	// Check the windows size
    if (windowSize < 3 || windowSize % 2 == 0) {
        std::cout << "Window size must be equal or greater than 3 and an odd number" << std::endl;
        exit(-1);
    }
}

/**
 * @brief Process a frame from an image or a video. Input frame must be BGR from 0 to 255
 * @param inputFrame Input frame
 * @return Processed frame
 */
Mat FileProcessor::processFrame(const Mat &inputFrame) {
    // Input checking
    int type = inputFrame.type();
	double minVal, maxVal;
	Tools::getMinMax(inputFrame, &minVal, &maxVal);
	if (!(type == CV_8UC1 || type == CV_8UC3) || minVal < 0 || maxVal > 255)
	   errorExit("Input frame must be a Grayscale or RGB unsigned integer matrix of size NxMx1 or NxMx3 on the closed interval [0,255]");

    // Converto to CIELab color space
	Mat outputFrame;
    inputFrame.convertTo(outputFrame, CV_32F, 1.0/255.0); // The image has to to have values from 0 to 1 before convertion to CIELab
	cvtColor(outputFrame, outputFrame, COLOR_BGR2Lab); // Convert normalized BGR image to CIELab color space.

	// Process frame
	outputFrame = DeceivedBilateralFilter(outputFrame);
	//outputFrame = ScaledDeceivedBilateralFilter(outputFrame);
	//outputFrame = DeceivedNonLocalMeansFilter(outputFrame);

	// Convert filtered image back to BGR color space.
	cvtColor(outputFrame, outputFrame, COLOR_Lab2BGR);
    outputFrame.convertTo(outputFrame, CV_8U, 255); // Scale back to [0,255] range

    return outputFrame;
}

/**
 * @brief Process an image file
 * @return return status
 */
int FileProcessor::processImage() {
	Mat inputFrame = imread(inputFileName);
	if(inputFrame.empty())
		errorExit("Could not open the input file for read: " + inputFileName);

	// Get the las dot position in the file name
	std::string::size_type dotPos = this->inputFileName.find_last_of('.');
	std::string imageExtension = inputFileName.substr(dotPos+1);
	std::transform(imageExtension.begin(), imageExtension.end(),imageExtension.begin(), ::toupper);

	// Print image information
	std::cout << "\nInput image information" << std::endl;
	std::cout << std::setw(DIVIDER_SPACE_2) << std::setfill('-') << '\n' << std::setfill(' ');
	std::cout << "| "
	<< std::left << std::setw(DATA_SPACE) << "Data"
	<< " | "
	<< std::left << std::setw(VALUE_SPACE+1) << "Value"
	<< "|";
	std::cout << std::setw(DIVIDER_SPACE_2) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
	std::ostringstream stringStream;
	stringStream << inputFrame.rows << "x" << inputFrame.cols;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Resolution"  << " | "  << std::setw(VALUE_SPACE) << std::left << stringStream.str()	<< " |" << std::endl;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Image Type" << " | " 	<< std::setw(VALUE_SPACE) << std::left << imageExtension		<< " |";
	std::cout << std::setw(DIVIDER_SPACE_2) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;

	// Process image
	Mat outputFrame;
	if(this->mode & benchmark) { // Benchmark mode?
		double elapsedSeconds;

		// Print header
		std::cout << std::internal <<"\nBenchmark mode" << std::endl;
		std::cout << std::setw(DIVIDER_SPACE) << std::setfill('-') << '\n' << std::setfill(' ');
		std::cout << "| "
		<< std::left << std::setw(NUMBER_SPACE) << "N"
		<< " | "
		<< std::left << std::setw(TIME_SPACE+1) << "Time [s]"
		<< "|";
		std::cout << std::setw(DIVIDER_SPACE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;

		for(int i = 1; i <= this->benchmarkIterations; i++) {
			this->timer.start();

			outputFrame = this->processFrame(inputFrame);

			elapsedSeconds = this->timer.stop();

			// Print results
			std::cout << "| "
			<< std::left << std::setw(NUMBER_SPACE) << i
			<< " | "
			<< std::left << std::setw(TIME_SPACE) << elapsedSeconds
			<< " |";
			if(i != benchmarkIterations) std::cout << std::endl;
		}

		// Print footer
		std::cout << std::setw(DIVIDER_SPACE) << std::setfill('-') << '\n' << std::setfill(' ') << '\n' << std::endl;
	} else outputFrame = this->processFrame(inputFrame);

	// Define the output file name
	this->outputFileName = this->inputFileName.substr(0, dotPos) + "_DeWAFF.png";

	if(!imwrite(outputFileName, outputFrame)) errorExit("Could not open the output file for write: " + outputFileName);

	// Display exit
	std::cout << "Processing done" << std::endl;

	return 0;
}

/**
 * @brief Process a video file
 * @return return status
 */
int FileProcessor::processVideo() {
	// Open input video file
	VideoCapture inputVideo = VideoCapture(inputFileName);
	if (!inputVideo.isOpened())
		errorExit("Could not open the input video for read: " + inputFileName);

    // Acquire input video information
	int frameRate = static_cast<int>(inputVideo.get(cv::CAP_PROP_FPS));
    int frameCount = static_cast<int>(inputVideo.get(cv::CAP_PROP_FRAME_COUNT));
    int codec = static_cast<int>(inputVideo.get(cv::CAP_PROP_FOURCC));
    Size videoSize = Size((int) inputVideo.get(cv::CAP_PROP_FRAME_WIDTH),(int) inputVideo.get(cv::CAP_PROP_FRAME_HEIGHT));

    // Transform from int to char via Bitwise operators
    char EXT[] = {	(char)((codec & 0XFF)),
					(char)((codec & 0XFF00) >> 8),
					(char)((codec & 0XFF0000) >> 16),
					(char)((codec & 0XFF000000) >> 24),
					0
				};
    std::string codecType = EXT;

	// Print collected video information
	std::cout << "\nInput video information" << std::endl;
	std::cout << std::setw(DIVIDER_SPACE_2) << std::setfill('-') << '\n' << std::setfill(' ');
	std::cout << "| "
	<< std::left << std::setw(DATA_SPACE) << "Data"
	<< " | "
	<< std::left << std::setw(VALUE_SPACE+1) << "Value"
	<< "|";
	std::cout << std::setw(DIVIDER_SPACE_2) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
	std::ostringstream stringStream;
	stringStream << videoSize.width << "x" << videoSize.height;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Resolution"  << " | "  << std::setw(VALUE_SPACE) << std::left << stringStream.str()	<< " |" << std::endl;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Frame count" << " | "  << std::setw(VALUE_SPACE) << std::left << frameCount			<< " |" << std::endl;
	stringStream.str(""); stringStream.clear();
	stringStream << frameRate << " fps";
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Frame rate"  << " | "  << std::setw(VALUE_SPACE) << std::left << stringStream.str()	<< " |" << std::endl;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Codec type"  << " | "  << std::setw(VALUE_SPACE) << std::left << codecType			<< " |";
	std::cout << std::setw(DIVIDER_SPACE_2) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;

	// Define output file name
	std::string::size_type dotPos = this->inputFileName.find_last_of('.');
	this->outputFileName = this->inputFileName.substr(0, dotPos) + "_DeWAFF.avi";

	// Open output video
	VideoWriter outputVideo(outputFileName, codec, frameRate , videoSize, true);
    if(!outputVideo.isOpened()) errorExit("Could not open the output video for write: " + outputFileName);

	Mat inputFrame, outputFrame;
	double elapsedSeconds = 0.0;
	if(this->mode & benchmark) { // Benchmark mode?
		// Print header
		std::cout << std::internal <<"\nBenchmark mode" << std::endl;
		std::cout << std::setw(DIVIDER_SPACE) << std::setfill('-') << '\n' << std::setfill(' ');
		std::cout << "| "
		<< std::left << std::setw(NUMBER_SPACE) << "N"
		<< " | "
		<< std::left << std::setw(TIME_SPACE) << "Time [s]"
		<< "|";
		std::cout << std::setw(DIVIDER_SPACE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;

		for(int i = 1; i <= this->benchmarkIterations; i++) {
			// Start timer
			this->timer.start();

			// Read one frame at a time
			while(inputVideo.read(inputFrame)) {
				// Process current frame
				outputFrame = this->processFrame(inputFrame);

				if(i == benchmarkIterations) {
					// Write frame to output video
					outputVideo.write(outputFrame);
				}
			}

			// Update timer
			elapsedSeconds = this->timer.stop();

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

		// Print footer
		std::cout << std::setw(DIVIDER_SPACE) << std::setfill('-') << '\n' << std::setfill(' ') << '\n' << std::endl;
	} else {
		// Read one frame at a time
		while(inputVideo.read(inputFrame)) {
			// Process current frame
			outputFrame = this->processFrame(inputFrame);

			// Write frame to output video
			outputVideo.write(outputFrame);
		}
	}

	// Release video resources
	inputVideo.release();
	outputVideo.release();

	// Display exit
	std::cout << "Processing done" << std::endl;

	return 0;
}

/**
 * @brief Display an error message
 * @param msg Error message
 */
void FileProcessor::errorExit(std::string msg) {
	std::cerr << "ERROR: " << msg << std::endl;
	exit(-1);
}