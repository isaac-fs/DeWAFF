/*
 * main.cpp
 *
 *  Created on: Oct 27, 2015
 *      Author: davidp
 */

#include "ParallelDeWAFF.hpp"

using namespace cv;

ParallelDeWAFF::ParallelDeWAFF(int argc, char** argv){
	this->mode = 0x0;
	this->numIter = 0;
	this->programName = argv[0];

    int c;
    while ((c = getopt(argc,argv,"hb:v:i:")) != -1){
		switch(c){
			case 'v': //Process a video
				if(this->mode & 0x2)//Check if flag for image enabled
					errHelpExit("Options -v and -i are mutually exclusive");
				this->mode |= 0x1;
				this->inputFile = optarg;
				break;
			case 'i': //Process an image
				if(this->mode & 0x1)//Check if flag for video enabled
					errHelpExit("Options -v and -i are mutually exclusive");
				this->mode |= 0x2;
				this->inputFile = optarg;
				break;
			case 'b'://Enable benchmarking
				this->mode |= 0x4;
				this->numIter = atoi(optarg);
				if(this->numIter<=0)
					errHelpExit("Number of benchmark iterations must be >= 1");
				break;
			case 'h':
				this->help();
				exit(0);
				break;
			case '?':
				this->help();
				exit(-1);
		        break;
			default:
				abort();
		}
    }

    if(!(mode & 0x3) || this->inputFile.empty())
    	errHelpExit("Required option or argument missing. Check syntax");
}

int ParallelDeWAFF::start(){
	int success = -1;

	if(this->mode & 0x1)
		success = this->processVideo();
	else if (this->mode & 0x2)
		success = this->processImage();

	return success;
}

void ParallelDeWAFF::help(){
	std::cout<< "------------------------------------------------------------------------------" 	<< std::endl
		<< "Usage:"                                                                         		<< std::endl
		<< this->programName << " [-b N] <-v|-i> inputFile"                                    		<< std::endl
		<< "-v:\tProcess a video"                                                           		<< std::endl
		<< "-i:\tProcess an image"                                                          		<< std::endl
		<< "-b:\tRun benchmark. Process image N times. Only for images"                     		<< std::endl
		<< "------------------------------------------------------------------------------" 		<< std::endl
		<< std::endl;
}

void ParallelDeWAFF::errExit(std::string msg){
	std::cerr << "ERROR: " << msg << std::endl;
	exit(-1);
}

void ParallelDeWAFF::errHelpExit(std::string msg){
	std::cerr << "ERROR: " << msg << std::endl;
	this->help();
	exit(-1);
}

int ParallelDeWAFF::processVideo(){

	//Open input video file
	VideoCapture iVideo = VideoCapture(inputFile);
	if (!iVideo.isOpened())
		errExit("ERROR: Could not open the input video for read: " + inputFile);

    // Acquire input information: frame rate, number of frames, codec and size
	int iRate = static_cast<int>(iVideo.get(cv::CAP_PROP_FPS));
    int iFrameCount = static_cast<int>(iVideo.get(cv::CAP_PROP_FRAME_COUNT));
    int iCodec = static_cast<int>(iVideo.get(cv::CAP_PROP_FOURCC));
    Size iSize = Size((int) iVideo.get(cv::CAP_PROP_FRAME_WIDTH),(int) iVideo.get(cv::CAP_PROP_FRAME_HEIGHT));

    // Transform from int to char via Bitwise operators
    char EXT[] = {(char)(iCodec & 0XFF) , (char)((iCodec & 0XFF00) >> 8),(char)((iCodec & 0XFF0000) >> 16),(char)((iCodec & 0XFF000000) >> 24), 0};
    std::string iCodecName = EXT;

    std::cout << "### Input Video Information ###" 					<< std::endl
    	 << "Resolution: " << iSize.width << "x" << iSize.height 	<< std::endl
         << "Number of frames: " << iFrameCount 					<< std::endl
         << "Frame rate: " << iRate << "fps"						<< std::endl
         << "Codec type: " << iCodecName 							<< std::endl
         << std::endl;

	//Define output file name
	std::string::size_type pAt = this->inputFile.find_last_of('.');
	this->outputFile = this->inputFile.substr(0, pAt) + "_deWAFF.avi";

	//Open output video
	VideoWriter oVideo(outputFile, iCodec, iRate , iSize, true);
    if (!oVideo.isOpened())
        errExit("ERROR: Could not open the output video for write: " + outputFile);

    std::cout << "Processing video..." << std::endl;
	Mat inputFrame,outputFrame;
	double elapsedSeconds = 0.0;
	for(int frame = 1;frame<=iFrameCount;frame++){

		//Read one frame from input video
		if(!iVideo.read(inputFrame)){
			std::cerr << "ERROR: Could not read current frame from video, skipping." << std::endl;
			break;
		}

		timerStart();

		//Process current frame
		outputFrame = this->processFrame(inputFrame);

		elapsedSeconds += timerStop();

		//Write frame to output video
		oVideo.write(outputFrame);
	}
	if(0x4 & this->mode)//Benchmark mode?
		std::cout << "Processing time = " << elapsedSeconds << " seconds." << std::endl;

	//Release video resources
	iVideo.release();
	oVideo.release();

	return 0;
}

int ParallelDeWAFF::processImage(){
	Mat inputFrame = imread(inputFile);
	if(inputFrame.empty())
		errExit("ERROR: Could not open the input file for read: " + inputFile);

	//Process image
	std::cout << "Processing image..." << std::endl;
	Mat outputFrame;
	if(0x4 & this->mode){ //Benchmark mode?
		double elapsedSeconds;
		for(int i = 1; i<=this->numIter; i++){
			std::cout << "Iteration " << i << std::endl;
			timerStart();

			outputFrame = this->processFrame(inputFrame);

			elapsedSeconds = timerStop();
			std::cout << "Processing time = " << elapsedSeconds << " seconds." << std::endl;
		}
	}
	else
		outputFrame = this->processFrame(inputFrame);

	//Define output file name
	std::string::size_type pAt = this->inputFile.find_last_of('.');
	this->outputFile = this->inputFile.substr(0, pAt) + "_deWAFF.jpg";

	if(!imwrite(outputFile, outputFrame))
		errExit("ERROR: Could not open the output file for write: " + outputFile);

	display_img(inputFrame, outputFrame);

	return 0;
}

//Input image must be BGR from 0 to 255
Mat ParallelDeWAFF::processFrame(const Mat& inputFrame){
    //Set parameters for processing
    int wRSize = 21;
    int sigma_r = 10;
    double sigma_s = wRSize/1.5;
    int lambda = 2;

    //Input checking
    int type = inputFrame.type();
	double minVal, maxVal;
	Tools::getMinMax(inputFrame, &minVal, &maxVal);
	if (!(type == CV_8UC1 || type == CV_8UC3) || minVal < 0 || maxVal > 255)
	   errExit("Input frame must be a Grayscale or RGB unsigned integer matrix of size NxMx1 or NxMx3 on the closed interval [0,255].");

    //Converto to CIELab color space
    Mat convertedFrame;
    inputFrame.convertTo(convertedFrame,CV_32F,1.0/255.0); //The image has to to have values from 0 to 1 before convertion to CIELab
	cvtColor(convertedFrame, convertedFrame, cv::COLOR_BGR2Lab); //Convert normalized BGR image to CIELab color space.

	Mat USMFrame = NonAdaptiveUSM::nonAdaptiveUSM(convertedFrame, lambda);
	Mat outputFrame = deWAFF::filter(convertedFrame, USMFrame, wRSize, sigma_s,sigma_r);

	//Convert filtered image back to BGR color space.
	cvtColor(outputFrame,outputFrame,cv::COLOR_Lab2BGR);
    outputFrame.convertTo(outputFrame,CV_8U,255); //Scale back to [0,255] range
    return outputFrame;
}

void ParallelDeWAFF::display_img(const Mat &input, const Mat &output){
	// Load the image into a matrix
	Mat image;
    hconcat(input, output, image);

    // Present the image
    namedWindow("deWAFF result", WINDOW_AUTOSIZE);
    imshow("deWAFF result", image);

    // Wait for an user interruption to quit
    waitKey(0);
}