#include "CLI.hpp"

/**
 * @brief Constructor for the CLI class
 * @param argc argument count from the terminal
 * @param argv arguments from the terminal
 */
CLI::CLI(int argc, char** argv) {
	this->mode = start;
	this->benchmarkIterations = 0;
	this->programName = argv[0];

    int c;
    while ((c = getopt(argc,argv,"hb:i:v:")) != -1) {
		switch(c) {
			case 'i': // Process an image
				if(this->mode & video) // Check if flag for video enabled
					this->errorExit("Options -v and -i are mutually exclusive");
				if(this->mode & benchmark && this->benchmarkIterations <= 0)
						this->errorExit("Number of benchmark iterations for "
								"image mode has to be greater than 1");
				this->mode |= image;
				this->inputFileName = optarg;
				break;
			case 'v': // Process a video
				if(this->mode & image) // Check if flag for image enabled
					this->errorExit("Options -v and -i are mutually exclusive");
				this->mode |= video;
				this->inputFileName = optarg;
				break;
			case 'b': // Enable benchmark mode
				this->mode |= benchmark;
				if(optarg)
					this->benchmarkIterations = atoi(optarg);
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
		this->errorExit("Incomplete command. Use the -h flag to see the options");
}

/**
 * @brief Start program execution
 */
int CLI::run() {
	int success = -1;
	if(this->mode & image)
		success = processImage();
	else if (this->mode & video)
		success = processVideo();
	return success;
}

/**
 * @brief Display program help
 */
void CLI::help() {
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