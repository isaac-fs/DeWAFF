#include "Timer.hpp"

/**
 * @brief Starts timer and resets the elapsed time
 *
 */
void Timer::start() {
	struct timeval timeOfDay;
	gettimeofday(&timeOfDay, nullptr);
	this->startTime = (double)timeOfDay.tv_sec + ((double)timeOfDay.tv_usec * 1.0e-6);
}

/**
 * @brief Stops the timer and returns the elapsed time
 * @return Elapsed time in seconds
 */
double Timer::stop() {
	struct timeval timeOfDay;
	gettimeofday(&timeOfDay, nullptr);
	return ((double)timeOfDay.tv_sec + ((double)timeOfDay.tv_usec * 1.0e-6)) - this->startTime;
}

