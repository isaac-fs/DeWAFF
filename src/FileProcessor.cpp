#include "FileProcessor.hpp"

/**
 * @brief Process a frame from an image or a video. Input frame must be BGR from 0 to 255
 * @param inputFrame Input frame
 * @return Processed frame
 */
Mat FileProcessor::processFrame(const Mat &inputFrame){
    // Input checking
    int type = inputFrame.type();
	double minVal, maxVal;
	Tools::getMinMax(inputFrame, &minVal, &maxVal);
	if (!(type == CV_8UC1 || type == CV_8UC3) || minVal < 0 || maxVal > 255)
	   errorExit("Input frame must be a Grayscale or RGB unsigned integer matrix of size NxMx1 or NxMx3 on the closed interval [0,255]");

    // Converto to CIELab color space
	Mat outputFrame;
    inputFrame.convertTo(outputFrame, CV_32F, 1.0/255.0); // The image has to to have values from 0 to 1 before convertion to CIELab
	cvtColor(outputFrame, outputFrame, cv::COLOR_BGR2Lab); // Convert normalized BGR image to CIELab color space.
	
	// Process image
	outputFrame = DeWAFF::DeceivedBilateralFilter(outputFrame);

	// Convert filtered image back to BGR color space.
	cvtColor(outputFrame, outputFrame, cv::COLOR_Lab2BGR);
    outputFrame.convertTo(outputFrame, CV_8U, 255); // Scale back to [0,255] range
	
    return outputFrame;
}

/**
 * @brief Process an image file
 * @return return status
 */
int FileProcessor::processImage(){
	Mat inputFrame = imread(inputFileName);
	if(inputFrame.empty())
		errorExit("Could not open the input file for read: " + inputFileName);

	// Process image
	std::cout << "Processing image..." << std::endl;
	Mat outputFrame;
	if(this->mode & benchmark){ // Benchmark mode?
		double elapsedSeconds;
		for(int i = 1; i <= this->benchmarkIterations; i++){
			std::cout << "Iteration " << i << std::endl;
			this->timer.start();

			outputFrame = this->processFrame(inputFrame);

			elapsedSeconds = this->timer.stop();
			std::cout << "Processing time = " << elapsedSeconds << " seconds." << std::endl;
		}
	}
	else
		outputFrame = this->processFrame(inputFrame);

	// Define the output file name
	std::string::size_type pAt = this->inputFileName.find_last_of('.');
	this->outputFileName = this->inputFileName.substr(0, pAt) + "_DeWAFF.jpg";

	if(!imwrite(outputFileName, outputFrame))
		errorExit("Could not open the output file for write: " + outputFileName);

	return 0;
}

/**
 * @brief Process a video file
 * @return return status
 */
int FileProcessor::processVideo(){
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

    std::cout 	<< "### Input Video Information ###" 							<< std::endl
    	 		<< "Resolution: " << videoSize.width << "x" << videoSize.height << std::endl
         		<< "Number of frames: " << frameCount 							<< std::endl
         		<< "Frame rate: " << frameRate << "fps"							<< std::endl
         		<< "Codec type: " << codecType 									<< std::endl
         		<< std::endl;

	// Define output file name
	std::string::size_type pAt = this->inputFileName.find_last_of('.');
	this->outputFileName = this->inputFileName.substr(0, pAt) + "_DeWAFF.avi";

	// Open output video
	VideoWriter outputVideo(outputFileName, codec, frameRate , videoSize, true);
    if (!outputVideo.isOpened())
        errorExit("Could not open the output video for write: " + outputFileName);

    std::cout << "Processing video..." << std::endl;
	Mat inputFrame, outputFrame;
	double elapsedSeconds = 0.0;
	if(this->mode & benchmark){ // Benchmark mode?
		// Start timer
		this->timer.start();

		for(int frame = 1; frame <= frameCount; frame++){
			// Read one frame from input video
			if(!inputVideo.read(inputFrame))
				errorExit("Could not read current frame from video");

			// Display current frame number
			std::cout << "Processing frame " << frame << " out of " << frameCount << std::endl;

			// Process current frame
			outputFrame = this->processFrame(inputFrame);

			// Write frame to output video
			outputVideo.write(outputFrame);

			// Stop timer
			elapsedSeconds += this->timer.stop();
		}
		
		// Display the benchmark time
		std::cout << "Processing time = " << elapsedSeconds << " seconds." << std::endl;\
		
	} else {
		for(int frame = 1; frame <= frameCount; frame++){
			// Read one frame from input video
			if(!inputVideo.read(inputFrame))
				errorExit("Could not read current frame from video");

			// Display current frame number
			std::cout << "Processing frame " << frame << " out of " << frameCount << std::endl;

			// Process current frame
			outputFrame = this->processFrame(inputFrame);
			
			// Write frame to output video
			outputVideo.write(outputFrame);
		}
	}

	// Release video resources
	inputVideo.release();
	outputVideo.release();

	return 0;
}

/**
 * @brief Display an error message
 * @param msg Error message
 */
void FileProcessor::errorExit(std::string msg){
	std::cerr << "ERROR: " << msg << std::endl;
	exit(-1);
}