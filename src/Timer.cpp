#include "Timer.hpp"

// Starts timer and resets the elapsed time
void Timer::start(){
	struct timeval tod;
	gettimeofday(&tod, nullptr);
	this->startTime = (double)tod.tv_sec + ((double)tod.tv_usec * 1.0e-6); 
}

// Stops the timer and returns the elapsed time
double Timer::stop(){
	struct timeval tod;
	gettimeofday(&tod, nullptr);
	return ((double)tod.tv_sec + ((double)tod.tv_usec * 1.0e-6)) - this->startTime;
}

