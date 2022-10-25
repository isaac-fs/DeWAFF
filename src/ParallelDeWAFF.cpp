/*
 * main.cpp
 *
 *  Created on: Oct 27, 2015
 *      Author: davidp
 */
#include "ParallelDeWAFF.hpp"

int main(int argc, char* argv[]){
	ParallelDeWAFF deWAFF(argc,argv);
    return deWAFF.start();
}

ParallelDeWAFF::ParallelDeWAFF(int argc, char* argv[]){
	this->mode = 0x0;
	this->numIter = 0;
	this->progName = argv[0];
	this->mask = -1 * Laplacian::logKernel(17, 0.005);

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
	cout<< "------------------------------------------------------------------------------" << endl
		<< "Usage:"                                                                         << endl
		<< this->progName << " [-b N] <-v|-i> inputFile"                                    << endl
		<< "-v:\tProcess a video"                                                           << endl
		<< "-i:\tProcess an image"                                                          << endl
		<< "-b:\tRun benchmark. Process image N times. Only for images"                     << endl
		<< "------------------------------------------------------------------------------" << endl
		<< endl;
}

void ParallelDeWAFF::errExit(string msg){
	cerr << "ERROR: " << msg << endl;
	exit(-1);
}

void ParallelDeWAFF::errHelpExit(string msg){
	cerr << "ERROR: " << msg << endl;
	this->help();
	exit(-1);
}

int ParallelDeWAFF::processVideo(){

	//Open input video file
	VideoCapture iVideo = VideoCapture(inputFile);
	if (!iVideo.isOpened())
		errExit("ERROR: Could not open the input video for read: " + inputFile);

    // Acquire input information: frame rate, number of frames, codec and size
	int iRate = static_cast<int>(iVideo.get(CV_CAP_PROP_FPS));
    int iFrameCount = static_cast<int>(iVideo.get(CV_CAP_PROP_FRAME_COUNT));
    int iCodec = static_cast<int>(iVideo.get(CV_CAP_PROP_FOURCC));
    Size iSize = Size((int) iVideo.get(CV_CAP_PROP_FRAME_WIDTH),(int) iVideo.get(CV_CAP_PROP_FRAME_HEIGHT));

    // Transform from int to char via Bitwise operators
    char EXT[] = {(char)(iCodec & 0XFF) , (char)((iCodec & 0XFF00) >> 8),(char)((iCodec & 0XFF0000) >> 16),(char)((iCodec & 0XFF000000) >> 24), 0};
    string iCodecName = EXT;

    cout << "### Input Video Information ###" 						<< endl
    	 << "Resolution: " << iSize.width << "x" << iSize.height 	<< endl
         << "Number of frames: " << iFrameCount 					<< endl
         << "Frame rate: " << iRate << "fps"						<< endl
         << "Codec type: " << iCodecName 							<< endl
         << endl;

	//Define output file name
	string::size_type pAt = this->inputFile.find_last_of('.');
	this->outputFile = this->inputFile.substr(0, pAt) + "_DeWAFF.avi";

	//Open output video
	VideoWriter oVideo(outputFile, iCodec, iRate , iSize, true);
    if (!oVideo.isOpened())
        errExit("ERROR: Could not open the output video for write: " + outputFile);

    cout << "Processing video..." << endl;
	Mat iFrame,oFrame;
	double elapsed_secs = 0.0;
	for(int frame = 1;frame<=iFrameCount;frame++){

		//Read one frame from input video
		if(!iVideo.read(iFrame)){
			cerr << "ERROR: Could not read current frame from video, skipping." << endl;
			break;
		}

		timerStart();

		//Process current frame
		oFrame = this->processFrame(iFrame);

		elapsed_secs += timerStop();

		//Write frame to output video
		oVideo.write(oFrame);
	}
	if(0x4 & this->mode)//Benchmark mode?
		cout << "Processing time = " << elapsed_secs << " seconds." << endl;

	//Release video resources
	iVideo.release();
	oVideo.release();

	return 0;
}

int ParallelDeWAFF::processImage(){
	Mat iFrame = imread(inputFile);
	if(iFrame.data==NULL)
		errExit("ERROR: Could not open the input file for read: " + inputFile);

	//Process image
	cout << "Processing image..." << endl;
	Mat oFrame;
	if(0x4 & this->mode){//Benchmark mode?
		double elapsed_secs;
		for(int i = 1; i<=this->numIter; i++){
			cout << "Iteration " << i << endl;
			timerStart();

			oFrame = this->processFrame(iFrame);

			elapsed_secs = timerStop();
			cout << "Processing time = " << elapsed_secs << " seconds." << endl;
		}
	}
	else
		oFrame = this->processFrame(iFrame);

	//Define output file name
	string::size_type pAt = this->inputFile.find_last_of('.');
	this->outputFile = this->inputFile.substr(0, pAt) + "_DeWAFF.jpg";

	if(!imwrite(outputFile, oFrame))
		errExit("ERROR: Could not open the output file for write: " + outputFile);

	return 0;
}

//Input image must be BGR from 0 to 255
Mat ParallelDeWAFF::processFrame(const Mat& F){
    //Set parameters for processing
    int wRSize = 21;
    double sigma_s = wRSize/1.5;
    int sigma_r = 10;
    int lambda = 2;

    //Input checking
    int type = F.type();
	double minF, maxF;
	Tools::minMax(F,&minF,&maxF);
	if (!(type == CV_8UC1 || type == CV_8UC3) || minF < 0 || maxF > 255)
	   errExit("Input frame must be a Grayscale or RGB unsigned integer matrix of size NxMx1 or NxMx3 on the closed interval [0,255].");

    //Converto to CIELab color space
    Mat N;
    F.convertTo(N,CV_32F,1.0/255.0); 	//The image has to to have values from 0 to 1 before convertion to CIELab
	cvtColor(N,N,CV_BGR2Lab);			//Convert normalized BGR image to CIELab color space.

	Mat L = Laplacian::noAdaptive(N, this->mask, lambda);
	N = DeWAFF::filter(N, L, wRSize, sigma_s,sigma_r);

	//Convert filtered image back to BGR color space.
	cvtColor(N,N,CV_Lab2BGR);
    N.convertTo(N,CV_8U,255);			//Scale back to [0,255] range

    return N;
}
