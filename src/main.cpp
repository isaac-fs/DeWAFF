#include <iostream>
#include <opencv2/opencv.hpp>
#include "ParallelDeWAFF.hpp"

int main(int argc, char** argv) {
    ParallelDeWAFF deWAFF(argc,argv);
    return deWAFF.start();

    return 0;
}