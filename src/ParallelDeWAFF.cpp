#include "ParallelDeWAFF.hpp"

using namespace cv;

/**
 * @brief Constructor for the ParallelDeWAFF class
 * @param argc argument count from the terminal
 * @param argv arguments from the terminal
 */
ParallelDeWAFF::ParallelDeWAFF(int argc, char** argv){
	this->mode = start;
	this->numIter = 0;
	this->programName = argv[0];

    int c;
    while ((c = getopt(argc,argv,"hb:i:v:")) != -1){
		switch(c){
			case 'i': //Process an image
				if(this->mode & video) // Check if flag for video enabled
					this->errExit("Options -v and -i are mutually exclusive");
				if(this->mode & benchmark && this->numIter <= 0)
						this->errExit("Number of benchmark iterations for "
								"image mode has to be greater than 1");
				this->mode |= image;
				this->inputFileName = optarg;
				break;
			case 'v': //Process a video
				if(this->mode & image) //Check if flag for image enabled
					this->errExit("Options -v and -i are mutually exclusive");
				this->mode |= video;
				this->inputFileName = optarg;
				break;
			case 'b': // Enable benchmark mode
				this->mode |= benchmark;
				if(optarg)
					this->numIter = atoi(optarg);
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
	if (argc == 1) 
		this->help();
	else if((this->mode & benchmark) && argc == 3)
		this->errExit("Incomplete command. Use the -h flag to see the options");
}

/**
 * @brief Start program execution
 */
int ParallelDeWAFF::execute(){
	int success = -1;
	if(this->mode & image)
		success = this->processImage();
	else if (this->mode & video)
		success = this->processVideo();
	return success;
}

/**
 * @brief Display program help
 */
void ParallelDeWAFF::help(){
	std::cout<< "------------------------------------------------------------------------------" 	<< std::endl
		<< "Use:"                                                                         			<< std::endl
		<< this->programName << " -i|-v <input file> [-b [N]]"										<< std::endl
		<< std::endl
		<< "Options:"                                                                         		<< std::endl
		<< "\t -i:\tProcess an image"                                                          		<< std::endl
		<< "\t -v:\tProcess a video"                                                           		<< std::endl
		<< "\t -b:\tRun a benchmark. In image mode the number of times to run"						<< std::endl
		<< "\t\tthe benchmark <N> is required"														<< std::endl
		<< "------------------------------------------------------------------------------" 		<< std::endl
		<< std::endl;
}

/**
 * @brief Display an error message
 * @param msg Error message
 */
void ParallelDeWAFF::errExit(std::string msg){
	std::cerr << "Error: " << msg << std::endl;
	exit(-1);
}

/**
 * @brief Process an image file
 * @return return status
 */
int ParallelDeWAFF::processImage(){
	Mat inputFrame = imread(inputFileName);
	if(inputFrame.empty())
		errExit("ERROR: Could not open the input file for read: " + inputFileName);

	//Process image
	std::cout << "Processing image..." << std::endl;
	Mat outputFrame;
	if(this->mode & benchmark){ //Benchmark mode?
		double elapsedSeconds;
		for(int i = 1; i <= this->numIter; i++){
			std::cout << "Iteration " << i << std::endl;
			this->timer.start();

			outputFrame = this->processFrame(inputFrame);

			elapsedSeconds = this->timer.stop();
			std::cout << "Processing time = " << elapsedSeconds << " seconds." << std::endl;
		}
	}
	else
		outputFrame = this->processFrame(inputFrame);

	//Define the output file name
	std::string::size_type pAt = this->inputFileName.find_last_of('.');
	this->outputFileName = this->inputFileName.substr(0, pAt) + "_deWAFF.jpg";

	if(!imwrite(outputFileName, outputFrame))
		errExit("ERROR: Could not open the output file for write: " + outputFileName);

	//Display the results
	displayImage(inputFrame, outputFrame);

	return 0;
}

/**
 * @brief Process a video file
 * @return return status
 */
int ParallelDeWAFF::processVideo(){
	//Open input video file
	VideoCapture inputVideo = VideoCapture(inputFileName);
	if (!inputVideo.isOpened())
		errExit("ERROR: Could not open the input video for read: " + inputFileName);

    // Acquire input video information: 
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

	//Define output file name
	std::string::size_type pAt = this->inputFileName.find_last_of('.');
	this->outputFileName = this->inputFileName.substr(0, pAt) + "_deWAFF.avi";

	//Open output video
	VideoWriter outputVideo(outputFileName, codec, frameRate , videoSize, true);
    if (!outputVideo.isOpened())
        errExit("ERROR: Could not open the output video for write: " + outputFileName);

    std::cout << "Processing video..." << std::endl;
	Mat inputFrame, outputFrame;
	double elapsedSeconds = 0.0;
	if(this->mode & benchmark){ //Benchmark mode?
		for(int frame = 1; frame <= frameCount; frame++){
			//Read one frame from input video
			if(!inputVideo.read(inputFrame)){
				std::cerr << "ERROR: Could not read current frame from video, skipping." << std::endl;
				break;
			}
			this->timer.start();

			//Process current frame
			outputFrame = this->processFrame(inputFrame);
			//Write frame to output video
			outputVideo.write(outputFrame);

			elapsedSeconds += this->timer.stop();
		}
		std::cout << "Processing time = " << elapsedSeconds << " seconds." << std::endl;
	} else {
		for(int frame = 1; frame <= frameCount; frame++){
			//Read one frame from input video
			if(!inputVideo.read(inputFrame)){
				std::cerr << "ERROR: Could not read current frame from video, skipping." << std::endl;
				break;
			}
			//Process current frame
			outputFrame = this->processFrame(inputFrame);
			//Write frame to output video
			outputVideo.write(outputFrame);
		}
	}

	//Release video resources
	inputVideo.release();
	outputVideo.release();

	return 0;
}

/**
 * @brief Process a frame from an image or a video. Input frame must be BGR from 0 to 255
 * @param inputFrame Input frame
 * @return Processed frame
 */
Mat ParallelDeWAFF::processFrame(const Mat& inputFrame){
    //Set parameters for processing
    int wSize = 21;
    int sigmaR = 10;
    double sigmaS = wSize/1.5;
    int lambda = 2;

    //Input checking
    int type = inputFrame.type();
	double minVal, maxVal;
	Tools::getMinMax(inputFrame, &minVal, &maxVal);
	if (!(type == CV_8UC1 || type == CV_8UC3) || minVal < 0 || maxVal > 255)
	   errExit("Input frame must be a Grayscale or RGB unsigned integer matrix of size NxMx1 or NxMx3 on the closed interval [0,255].");

    //Converto to CIELab color space
    Mat frame;
	//The image has to to have values from 0 to 1 before convertion to CIELab
    inputFrame.convertTo(frame, CV_32F, 1.0/255.0); 
	//Convert normalized BGR image to CIELab color space.
	cvtColor(frame, frame, cv::COLOR_BGR2Lab); 

	Mat maskedFrame = NonAdaptiveUSM::nonAdaptiveUSM(frame, lambda);
	Mat outputFrame = deWAFF::filter(frame, maskedFrame, wSize, sigmaS,sigmaR);

	//Convert filtered image back to BGR color space.
	cvtColor(outputFrame, outputFrame, cv::COLOR_Lab2BGR);
    outputFrame.convertTo(outputFrame, CV_8U, 255); //Scale back to [0,255] range
    return outputFrame;
}

void ParallelDeWAFF::displayImage(const Mat &input, const Mat &output){
	// Load the image into a matrix
	Mat image;
    hconcat(input, output, image);

    // Display the image
    namedWindow("deWAFF result", WINDOW_AUTOSIZE);
    imshow("deWAFF result", image);

    // Wait for an user interruption to quit
    waitKey(0);
}