/**
 * @file Tools.hpp
 * @author Esteban Meneses, PhD (emeneses@ic-itcr.ac.cr)
 * @author Isaac Fonseca (isaac.fonsecasegura@ucr.ac.cr)
 * @date 2022-11-06
 * @brief 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef TIMER_HPP_
#define TIMER_HPP_
#include <sys/time.h>

class Timer {
    private:
        double startTime;
    public:
        void start(); // Starts timer and resets the elapsed time
        double stop(); // Stops the timer and returns the elapsed time
};

#endif /* TIMER_HPP_ */