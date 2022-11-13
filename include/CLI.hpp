/**
 * @file ParallelDeWAFF.hpp
 * @author David Prado (davidp)
 * @date 2015-11-05
 * @author Isaac Fonseca (isaac.fonsecasegura@ucr.ac.cr)
 * @date 2022-11-06
 * @brief
 * @copyright Copyright (c) 2022
 *
 */

#ifndef PROGRAM_INTERFACE_HPP_
#define PROGRAM_INTERFACE_HPP_

#include <string>
#include <unistd.h>
#include <iostream>
#include "FileProcessor.hpp"

/**
 * @brief In charge of displaying the CLI of the program, including the help commands.
 * It takes the CLI instructions to process the file. This is why this class inherits from
 * the FileProcessor class.
 */
class CLI : protected FileProcessor {
public:
	CLI(int argc, char** argv);
	int run();

private:
	std::string programName;
	void help();
};

#endif /* PROGRAM_INTERFACE_HPP_ */
