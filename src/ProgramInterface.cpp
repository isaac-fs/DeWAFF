#include "ProgramInterface.hpp"

/**
 * @brief Constructor for the ProgramInterface class. Sets all the necessary parameters for the DeWAFF processing,
 * including the ones captured from the user input in the terminal
 * @param argc argument count from the terminal
 * @param argv arguments from the terminal
 */
ProgramInterface::ProgramInterface(int argc, char** argv) {
	// Initial values
	mode = start;
	benchmarkIterations = 0;
	quietMode = false; // Print info

	// Framework
	framework = DeWAFF();
	filterType = DBF;
	windowSize = 3;
	neighborhoodSize = 3;
	spatialSigma = 1.0;
	rangeSigma = 1.0;

	// Libraries
	utilsLib = Utils();
	timer = Timer();

	// Set the program name
	programName = argv[0];

	// Filter option identifiers
	enum filterOptsValues {
		WINDOW_SIZE,
		RANGE_SIGMA,
		SPATIAL_SIGMA,
		LAMBDA,
		NEIGHBORHOOD_SIZE,
	};

	// Filter options
	const char *filterOpts[] = {
		"ws",		// window_size,
		"rs",		// range_sigma,
		"ss",		// spatial_sigma,
		"lambda",	// lambda,
		"ns",		// neighborhood_size,
		NULL
	};

	// Map to convert CLI capture option into option value
	std::map<std::string, int> filterIdentifierMap = {
		{"dbf", DBF},
		{"dsbf", DSBF},
		{"dnlmf", DNLMF},
		{"dgf", DGF}
	};

	struct option long_options[] = {
          {"image",     	required_argument, 0, 'i'},
          {"video",  		required_argument, 0, 'v'},
          {"filter",  		required_argument, 0, 'f'},
          {"parameters",    required_argument, 0, 'p'},
          {"benchmark",  	required_argument, 0, 'b'},
		  {"quiet",  		no_argument		, 0, 'q'},
		  {"help",  		no_argument		, 0, 'H'},
          {0, 0, 0, 0}
    };

	// getopt and getsubopt vars
	char *subopts, *value;
    int opt, opt_index;

	// Capture user input
    while ((opt = getopt_long(argc, argv, "b:i:f:v:p:hq", long_options, &opt_index)) != -1) {
		switch(opt) {
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
			case 'f': {
				std::string fName = optarg;
				int f = filterIdentifierMap[fName];
				if(f < DBF || f > DGF) errorMessage("Not a valid filter option. Use option --help to check valid filters");
				else filterType = f;
				break;
			}
			case 'p': // Filter parameters
				subopts = optarg;
                while (*subopts != '\0') {
                    char *saved = subopts;
                    switch(getsubopt(&subopts, (char **)filterOpts, &value)) {
						case WINDOW_SIZE: {
							if(value == NULL) abort();
							int wS = atoi(value);
							if(wS < 3 || wS % 2 == 0) errorMessage("Window size must be equal or greater than 3 and an odd number");
							else windowSize = wS;
							break;
						}
						case RANGE_SIGMA: {
							if(value == NULL) abort();
							double rs = atof(value);
							if(rs < 0.0001) errorMessage("Range Sigma value must be greater than 0.0001");
							else rangeSigma = rs;
							break;
						}
						case SPATIAL_SIGMA: {
							if(value == NULL) abort();
							double ss = atof(value);
							if(ss < 0.0001) errorMessage("Spatial Sigma value must be greater than 0.0001");
							else spatialSigma = ss;
							break;
						}
						case LAMBDA: {
							if(value == NULL) abort();
							double l = atof(value);
							if(l < 0) errorMessage("Lambda value must be equal or greater than zero");
							else framework.usmLambda = l;
                        	break;
						}
						case NEIGHBORHOOD_SIZE: {
							if(value == NULL) abort();
							if(filterType != DNLMF)
								errorMessage("Neighborhoodsize option only applies when the filter type is set to Deceived Non Local Means Filter");
							int ns = atoi(value);
							if(ns > windowSize) errorMessage("Neighborhood size must be smaller than the window size");
							if(ns < 3 || ns % 2 == 0) errorMessage("Neighborhood size must be an odd number equal or greater than 3");
							else neighborhoodSize = ns;
							break;
						}
						default:
							/* Unknown suboption. */
							std::string s = "Unknown filter option: ";
							s+=*saved;
							errorMessage(s);
							abort();
                   	}
                }
				break;
			case 'q':
				quietMode = true;
				break;
			case 'H':
				longHelp();
				exit(-1);
				break;
			case 'h':
				help();
				exit(-1);
				break;
			case '?':
				exit(-1);
		        break;
			default:
				abort();
		}
    }

	// Catch extra arguments in the terminal
	if(argc-1 == optind) {
		std::string error = "Unexpected argument \"" + (std::string) argv[argc-1] + "\"";
		errorMessage(error);
	}

	// Get the las dot position in the file name
	dotPos = inputFileName.find_last_of('.');

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
		std::cout << "Use " << programName << " --help to see the program's full usage" << std::endl;
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
	utilsLib.MinMax(inputImage, &minVal, &maxVal);
	if(!(type == CV_8UC1 || type == CV_8UC3) || minVal < 0 || maxVal > 255)
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
	case DBF:
		output = framework.DeceivedBilateralFilter(input, windowSize, spatialSigma, rangeSigma);
		break;
	case DSBF:
		output = framework.DeceivedScaledBilateralFilter(input, windowSize, spatialSigma, rangeSigma);
		break;
	case DNLMF:
		output = framework.DeceivedNonLocalMeansFilter(input, windowSize, neighborhoodSize, spatialSigma, rangeSigma);
		break;
	case DGF:
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
	if(!quietMode) {
		displayImageInfo();
		displayFilterParams();
	}

	// Process image
	Mat outputFrame;
	outputFrame = processFrame(inputFrame);

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
	if(!inputVideo.isOpened()) errorMessage("Could not open the input video for read: " + inputFileName);

	// Acquire input video information
	getVideoInfo(inputVideo);

	// Print collected video information
	if(!quietMode){
		displayVideoInfo();
		displayFilterParams();
	}

	// Open output video
	VideoWriter outputVideo(outputFileName, codec, frameRate , frameSize, true);
    if(!outputVideo.isOpened()) errorMessage("Could not open the output video for write: " + outputFileName);

	// Read one frame at a time
	Mat inputFrame, outputFrame;
	while(inputVideo.read(inputFrame)) {
		// Process current frame
		outputFrame = processFrame(inputFrame);

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
		if(!quietMode) {
			displayImageInfo();
			displayFilterParams();
		}

		double elapsedSeconds;

		displayBenchmarkHeader();
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
		displayBenchmarkFooter();
}

/**
 * @brief Benchmarks a video
 *
 */
void ProgramInterface::benchmarkVideo() {
	VideoCapture inputVideo = VideoCapture(inputFileName);
	if(!inputVideo.isOpened()) errorMessage("Could not open the input video for read: " + inputFileName);

	// Acquire input video information
	getVideoInfo(inputVideo);

	// Print collected video information
	if(!quietMode) {
		displayVideoInfo();
		displayFilterParams();
	}

	Mat inputFrame, outputFrame;
	double elapsedSeconds;

	displayBenchmarkHeader();
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
	displayBenchmarkFooter();
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
void ProgramInterface::displayVideoInfo() {
	std::cout << "Input video information" << std::endl;
	std::cout << std::setw(MAIN_LINE) << std::setfill('-') << '\n' << std::setfill(' ');
	std::cout << "| "
	<< std::left << std::setw(DATA_SPACE) << "Data"
	<< " | "
	<< std::left << std::setw(VALUE_SPACE+1) << "Value"
	<< "|";
	std::cout << std::setw(MAIN_LINE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
	std::ostringstream stringStream;
	stringStream << frameSize.width << "x" << frameSize.height;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Resolution"  << " | "  << std::setw(VALUE_SPACE) << std::left << stringStream.str() << " |" << std::endl;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Frame count" << " | "  << std::setw(VALUE_SPACE) << std::left << frameCount << " |" << std::endl;
	stringStream.str(""); stringStream.clear();
	stringStream << frameRate << " fps";
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Frame rate"  << " | "  << std::setw(VALUE_SPACE) << std::left << stringStream.str() << " |" << std::endl;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Codec type"  << " | "  << std::setw(VALUE_SPACE) << std::left << codecType	 << "  |";
	std::cout << std::setw(MAIN_LINE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
}

/**
 * @brief Prints the image information
 *
 */
void ProgramInterface::displayImageInfo() {
	// Get the image extension
	std::string imageExtension = inputFileName.substr(dotPos+1);
	std::transform(imageExtension.begin(), imageExtension.end(),imageExtension.begin(), ::toupper);

	// Print image information
	std::cout << "Input image information" << std::endl;
	std::cout << std::setw(MAIN_LINE) << std::setfill('-') << '\n' << std::setfill(' ');
	std::cout << "| "
	<< std::left << std::setw(DATA_SPACE) << "Data"
	<< " | "
	<< std::left << std::setw(VALUE_SPACE+1) << "Value"
	<< "|";
	std::cout << std::setw(MAIN_LINE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
	std::ostringstream stringStream;
	stringStream << frameSize.width << "x" << frameSize.height;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Resolution"  << " | "  << std::setw(VALUE_SPACE) << std::left << stringStream.str()	<< " |" << std::endl;
	std::cout << "| " << std::setw(DATA_SPACE) << std::left  << "Image Type" << " | " 	<< std::setw(VALUE_SPACE) << std::left << imageExtension		<< " |";
	std::cout << std::setw(MAIN_LINE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;

}

/**
 * @brief Prints the benchmark header
 *
 */
void ProgramInterface::displayBenchmarkHeader() {
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
void ProgramInterface::displayBenchmarkFooter() {
	std::cout << std::setw(BENCHMARK_LINE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
}

/**
 * @brief Displays the program's short help
 */
void ProgramInterface::help() {
	std::cout
	<< "usage: " << programName << " "
	<< "[-i | --image <file name>] | [-v | --video <file name>]" << std::endl
	<< "\t\t" << "[-f | --filter <filter type>]" << std::endl
	<< "\t\t" << "[-p | --parameters <filter parameters>]" << std::endl
	<< "\t\t" << "[-b | --benchmark <number of iterations>] [-h | --help]"
	<< std::endl;
}

/**
 * @brief Displays the program's full help
 */
void ProgramInterface::longHelp() {
	help();
	std::cout
	<< "\n\t" << std::left << "DEFAULT PARAMETERS"
	<< "\n\t" << std::setw(21) << "- Filter:" << "dbf (Deceived Bilateral Filter)"
	<< "\n\t" << std::setw(21) << "- Window size:" << 3
	<< "\n\t" << std::setw(21) << "- Neighborhood size:" << 3
	<< "\n\t" << std::setw(21) << "- Range Sigma:" << 1.0
	<< "\n\t" << std::setw(21) << "- Spatial Sigma:" << 1.0
	<< "\n\t" << std::setw(21) << "- USM Lambda:" << 1.0
	<< "\n" << std::endl

	<< "\t" << std::left << "PROGRAM OPTIONS\n"

	<< "\t" << std::left << "-i, --image"
	<< ": " << "Process an image given a file name. The file name goes"
	<< "\n\t" << "after the option."
	<< "\n\t" << "Example: \'-i picture.png\'"
	<< "\n" << std::endl

	<< "\t" << std::left << "-v, --video"
	<< ": " << "Process a video given a file name. The file name goes"
	<< "\n\t" << "after the option."
	<< "\n\t" << "Example: \'-v video.mp4\'"
	<< "\n" << std::endl

	<< "\t" << std::left << "-f, --filter"
	<< ": " << "Choose which filter to use. The availabe options are:"
	<< "\n\t\t" << std::setw(8) << "- dbf:" << "deceived bilateral filter"
	<< "\n\t\t" << std::setw(8) << "- dsbf:" << "deceived scaled bilateral filter"
	<< "\n\t\t" << std::setw(8) << "- dnlmf:" << "deceived non local means filter"
	<< "\n\t\t" << std::setw(8) << "- dgf:" << "deceived guided filter"
	<< "\n\t" << "For example, to process an image using the deceived bilateral filter"
	<< "\n\t" << "use: \'./DeWAFF -i image.png -f dbf\'."
	<< "\n" << std::endl

	<< "\t" << std::left << "-p, --parameters"
	<< ": " << "Change the filter parameters. Available parameters:"
	<< "\n\t\t" << std::setw(9) << "- ws:" << "Window size"
	<< "\n\t\t" << std::setw(9) << "- rs:" << "Range Sigma"
	<< "\n\t\t" << std::setw(9) << "- ss:" << "Spatial Sigma"
	<< "\n\t\t" << std::setw(9) << "- lambda:" << "Lambda value for the Laplacian deceive"
	<< "\n\t\t" << std::setw(9) << "- ns:" << "Neighborhood size for the DNLM filter"
	<< "\n\t" << "It is possible to change one or more parameters in the same line,"
	<< "\n\t" << "for example \'-p ws=15,rs=10,ss=10\' would change the window size and"
	<< "\n\t" << "the range and spatial sigma values for the filter. Using just"
	<< "\n\t" << "\'-p ws=15\' would only change its window size."
	<< "\n\t" << "The \'ns\' option only works with the filter set to \'dnlm\'."
	<< "\n\t" << "If \'lambda=0\' the Laplacian the deceive will be disabled."
	<< "\n" << std::endl

	<< "\t" << std::left << "-b, --benchmark"
	<< ": " << "Run a series of N benchmarks for a video or an image."
	<< "\n\t" << "This option will run aseries of N benchmarks and"
	<< "\n\t" << "display the results in the terminal."
	<< "\n\t" << "Note: The results are NOT saved during this process."
	<< "\n\t" << "Indicate the number of iterations after the flag,"
	<< "\n\t" << "for example \'-b 10\' would indicate to run the filter"
	<< "\n\t" << "ten separate times."
	<< "\n" << std::endl

	<< "\t" << std::left << "-q, --quiet"
	<< ": " << "Run in quiet mode. Does not displays the file and"
	<< "\n\t" << "filter information."
	<< "\n" << std::endl

	<< "\t" << std::left << "-h, --help"
	<< ": " << "Display the program's help message. The long version"
	<< "\n\t" << "--help shows the full program's help."

	<< std::endl;
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
	case DBF:
		filterAcronym = "DBF";
		break;
	case DSBF:
		filterAcronym = "DSBF";
		break;
	case DNLMF:
		filterAcronym = "DNLMF";
		break;
	case DGF:
		filterAcronym = "DGF";
		break;
	default:
		filterAcronym = "";
		break;
	}

	std::string extension;
	if(mode & image) extension = ".png";
	else if(mode & video) extension = ".avi";

	// Set the output file names
	outputFileName = inputFileName.substr(0, dotPos) + "_" + filterAcronym + extension;
}

/**
 * @brief Display the filter parametric information
 */
void ProgramInterface::displayFilterParams() {
	std::map<int, std::string> filteNameMap = {
		{DBF, "Deceived Bilateral Filter"},
		{DSBF, "Deceived Scaled Bilateral Filter"},
		{DNLMF, "Deceived Non Local Means Filter"},
		{DGF, "Deceived Guided Filter"}
	};

	std::cout << "\nFilter parameters";
	std::cout << std::setw(PARAMS_LINE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
	std::cout << "| "
	<< std::left << std::setw(PARAM_DESC_SPACE) << "Parameter"
	<< " | "
	<< std::left << std::setw(PARAM_VAL_SPACE+1) << "Value"
	<< "|";
	std::cout << std::setw(PARAMS_LINE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
	std::cout << "| " << std::setw(PARAM_DESC_SPACE) << std::left  << "Filter"  << " | "  << std::setw(PARAM_VAL_SPACE) << std::left << filteNameMap[filterType] << " |" << std::endl;
	std::cout << "| " << std::setw(PARAM_DESC_SPACE) << std::left  << "Window size"  << " | "  << std::setw(PARAM_VAL_SPACE) << std::left << windowSize	<< " |" << std::endl;
	if(filterType == DNLMF) std::cout << "| " << std::setw(PARAM_DESC_SPACE) << std::left  << "Neighborhood size"  << " | "  << std::setw(PARAM_VAL_SPACE) << std::left << neighborhoodSize	<< " |" << std::endl;
	std::cout << "| " << std::setw(PARAM_DESC_SPACE) << std::left  << "Range Sigma"  << " | "  << std::setw(PARAM_VAL_SPACE) << std::left << rangeSigma	<< " |" << std::endl;
	std::cout << "| " << std::setw(PARAM_DESC_SPACE) << std::left  << "Spatial Sigma"  << " | "  << std::setw(PARAM_VAL_SPACE) << std::left << spatialSigma	<< " |" << std::endl;
	std::cout << "| " << std::setw(PARAM_DESC_SPACE) << std::left  << "USM Lambda"  << " | "  << std::setw(PARAM_VAL_SPACE) << std::left << framework.usmLambda	<< " |";

	std::cout << std::setw(PARAMS_LINE) << std::setfill('-') << '\n' << std::setfill(' ') << std::endl;
}