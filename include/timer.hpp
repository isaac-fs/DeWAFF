/**
 * Costa Rica Institute of Technology
 * School of Computing
 * MC8836: Parallel Computing
 * Instructor Esteban Meneses, PhD (emeneses@ic-itcr.ac.cr)
 * Timing operations.
 */

#ifndef TIMER_HPP_
#define TIMER_HPP_

#include <sys/time.h>

// Starts timer and resets the elapsed time
void timerStart();

// Stops the timer and returns the elapsed time
double timerStop();

#endif /* TIMER_HPP_ */