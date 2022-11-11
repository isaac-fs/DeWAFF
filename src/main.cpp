#include <iostream>
#include <opencv2/opencv.hpp>
#include "CLI.hpp"

int main(int argc, char** argv) {
    CLI interface(argc, argv);
    return interface.run();
}