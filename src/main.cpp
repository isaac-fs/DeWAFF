/**
 * @file main.cpp
 * @brief Starting point for the DeWAFF program
 * @author Isaac Fonseca Segura {isaac-fs}
 * @date 10/29/2022
 */

#include <iostream>
#include <opencv2/opencv.hpp>
#include "ParallelDeWAFF.hpp"

int main(int argc, char** argv) {
    ParallelDeWAFF deWAFF(argc,argv);
    return deWAFF.execute();
}