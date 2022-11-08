#ifndef FILE_PROCESSOR_HPP_
#define FILE_PROCESSOR_HPP_

#include "DeWAFF.hpp"
#include "Timer.hpp"

class FileProcessor {
    protected:
        std::string inputFileName;
        std::string outputFileName;
        enum modes : unsigned int {start = 0, image = 1, video = 2, benchmark = 4}; // Processing modes
        unsigned int mode;
        int numIter; // Number of iterations for benchmark mode
        int processImage();
        int processVideo();
        void errorExit(std::string msg);

    private:
        Timer timer;
        Mat processFrame(const Mat & frame);
        void displayImage(const Mat &input, const Mat &output);
};

#endif /* FILE_PROCESSOR_HPP_ */