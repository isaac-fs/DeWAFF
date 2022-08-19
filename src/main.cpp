#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;

int main(int argc, char** argv) {
    // Check if an image was passed to the program
    if (argc != 2) {
        std::cout << "usage: DisplayImage.out <Image_Path>" << std::endl;
        return -1;
    }

    // Load the image into a matrix
    Mat image;
    image = imread(argv[1], 1);

    // Check that the image has data
    if (!image.data) {
        std::cout << "No image data\n" << std::endl;
        return -1;
    }

    // Present the image
    namedWindow("Display Image", WINDOW_AUTOSIZE);
    imshow("Display Image", image);

    // Wait for an user interruption to quit
    waitKey(0);

    return 0;

}