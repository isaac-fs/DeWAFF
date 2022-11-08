#include <iostream>
#include <opencv2/opencv.hpp>
#include "ProgramInterface.hpp"

int main(int argc, char** argv) {
    ProgramInterface Interface(argc,argv);
    return Interface.run();
}