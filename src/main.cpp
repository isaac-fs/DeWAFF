#include <iostream>
#include <opencv2/opencv.hpp>
#include "parallel_deWAFF.hpp"

using namespace cv;

int main(int argc, char** argv) {

    // // Load the image into a matrix
    // Mat image;
    // image = imread(argv[1], 1);

    // // Check that the image has data
    // if (!image.data) {
    //     std::cout << "No image data\n" << std::endl;
    //     return -1;
    // }

    // // Present the image
    // namedWindow("Display Image", WINDOW_AUTOSIZE);
    // imshow("Display Image", image);

    // // Wait for an user interruption to quit
    // waitKey(0);
    
    Parallel_deWAFF deWAFF(argc,argv);
    return deWAFF.start();

    return 0;

}