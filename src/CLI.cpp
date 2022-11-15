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
				if(optarg){
					this->benchmarkIterations = atoi(optarg);
					if(benchmarkIterations < 1) errorExit("The number of benchmark iterations [N] needs to be 1 or greater");
				}
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