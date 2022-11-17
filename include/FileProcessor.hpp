/**
 * @file FileProcessor.hpp
 * @author David Prado (davidp)
 * @date 2015-11-05
 * @author Isaac Fonseca (isaac.fonsecasegura@ucr.ac.cr)
 * @date 2022-11-06
 * @copyright Copyright (c) 2022
 *
 */

#ifndef FILE_PROCESSOR_HPP_
#define FILE_PROCESSOR_HPP_

#include <cstdio>
#include <iomanip>
#include "Timer.hpp"
#include "DeWAFF.hpp"

/**
 * @brief Class in charge of al the file processing, may it be a an image or a video.
 * This class also allows for benchmarking of the processing
 */
class FileProcessor : protected DeWAFF {
    protected:
        std::string inputFileName;
        std::string outputFileName;
        int benchmarkIterations; // Number of iterations for benchmark mode
        enum modes : unsigned int {start = 0, image = 1, video = 2, benchmark = 4}; // Processing modes
        unsigned int mode;
        int processImage();
        int processVideo();
        void errorExit(std::string msg);

    private:
        enum SPACE {
            DIVIDER_SPACE = 19,
            DIVIDER_SPACE_2 = 27,
            NUMBER_SPACE = 3,
            TIME_SPACE = 8,
            DATA_SPACE = 11,
            VALUE_SPACE = 8
            };
        Timer timer;
        Mat processFrame(const Mat &frame);

    public:
        FileProcessor();
};

#endif /* FILE_PROCESSOR_HPP_ */