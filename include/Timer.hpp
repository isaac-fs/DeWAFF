/**
 * @file Timer.hpp
 * @author Isaac Fonseca (isaac.fonsecasegura@ucr.ac.cr)
 * @date 2022-11-06
 * @author Esteban Meneses, PhD (emeneses@ic-itcr.ac.cr)
 * @date 2015-11-05
 *
 */

#ifndef TIMER_HPP_
#define TIMER_HPP_

#include <sys/time.h>

/**
 * @brief Class containing the timer methods for the benchmarking of file processing
 *
 */
class Timer {
	private:
		double startTime;
	public:
		void start(); // Starts timer and resets the elapsed time
		double stop(); // Stops the timer and returns the elapsed time
};

#endif /* TIMER_HPP_ */