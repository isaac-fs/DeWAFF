#include "DeWAFF.hpp"

/**
 * @brief DeWAFF class constructor. Sets the lambda parameter for the Laplacian deceive
 *
 */
DeWAFF::DeWAFF(): usmLambda(2){}

/**
 * @brief Apply the Deceived Bilateral Filter to an image. Uses an UnSharp mask as deceiver
 *
 * @param inputImage input image
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param spatialSigma spatial standard deviation
 * @param rangeSigma range or radiometric standard deviation
 * @return Mat output image
 */
Mat DeWAFF::DeceivedBilateralFilter(const Mat &inputImage, int windowSize, double spatialSigma, double rangeSigma) {
    // Pre-process the laplacian masked image
    Mat usmImage = lib.NonAdaptiveUSMFilter(inputImage, windowSize, usmLambda, spatialSigma);
    // Calculate the deceived filter
    return BilateralFilter(usmImage, inputImage, windowSize, spatialSigma, rangeSigma);
}

/**
 * @brief Apply the Deceived Scaled Bilateral Filter to an image. Uses an UnSharp mask as deceiver.
 * Similar to the Deceived Bilateral Filter, but the weighting image is low pass filtered
 *
 * @param inputImage input image
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param spatialSigma spatial standard deviation
 * @param rangeSigma range or radiometric standard deviation
 * @return Mat output image
 */
Mat DeWAFF::DeceivedScaledBilateralFilter(const Mat &inputImage, int windowSize, double spatialSigma, double rangeSigma) {
    // Pre-process the laplacian masked image
    Mat usmImage = lib.NonAdaptiveUSMFilter(inputImage, windowSize, usmLambda, spatialSigma);
    // Calculate the deceived filter
    return ScaledBilateralFilter(usmImage, inputImage, windowSize, spatialSigma, rangeSigma);
}

/**
 * @brief Apply the Deceived Non Local Means Filter to an image. Uses an UnSharp mask as deceiver.
 * Computationally demanding algorithm, can take as much as ten times more than the other filters
 * in the framework
 *
 * @param inputImage input image
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param neighborhoodSize spatial standard deviation
 * @param spatialSigma range or radiometric standard deviation
 * @param rangeSigma output image
 * @return Mat
 */
Mat DeWAFF::DeceivedNonLocalMeansFilter(const Mat &inputImage, int windowSize, int neighborhoodSize, double spatialSigma, double rangeSigma) {
    // Pre-process the laplacian masked image
    Mat usmImage = lib.NonAdaptiveUSMFilter(inputImage, windowSize, usmLambda, spatialSigma);
    // Calculate the deceived filter
    return NonLocalMeansFilter(usmImage, inputImage, windowSize, neighborhoodSize, rangeSigma);
}

/**
 * @brief Apply the Deceived Guided Filter to an image. Uses an UnSharp mask as deceiver.
 * The fastest WAF for the DeWAFF yet
 *
 * @param inputImage input image
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param spatialSigma spatial standard deviation
 * @param rangeSigma range or radiometric standard deviation
 * @return Mat output image
 */
Mat DeWAFF::DeceivedGuidedFilter(const Mat &inputImage, int windowSize, double spatialSigma, double rangeSigma) {
    Mat usmImage = lib.NonAdaptiveUSMFilter(inputImage, windowSize, usmLambda, spatialSigma);
    // Calculate the deceived filter
    return GuidedFilter(usmImage, inputImage, windowSize, rangeSigma);
}