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

class ProgramInterface : protected FileProcessor {
public:
	ProgramInterface(int argc, char** argv);
	int run();

private:
	std::string programName;
	void help();
};

#endif /* PROGRAM_INTERFACE_HPP_ */
