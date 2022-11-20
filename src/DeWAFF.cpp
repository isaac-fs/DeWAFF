#include "DeWAFF.hpp"

/**
 * @brief DeWAFF class constructor. Sets the lambda parameter for the Laplacian deceive
 *
 */
DeWAFF::DeWAFF(){
    this->usmLambda = 2;
}

/**
 * @brief Apply the DeceivedBilateralFilter to an image. Uses an UnSharp mask as deceiver
 *
 * @param inputImage input image
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param spatialSigma spatial standard deviation
 * @param rangeSigma range or radiometric standard deviation
 * @return Mat output image
 */
Mat DeWAFF::DeceivedBilateralFilter(const Mat &inputImage, int windowSize, double spatialSigma, int rangeSigma) {
    // Pre-process the laplacian masked image
    Mat usmImage = lib.NonAdaptiveUSM(inputImage, windowSize, usmLambda, spatialSigma);
    // Calculate the deceived filter
    return BilateralFilter(inputImage, usmImage, windowSize, spatialSigma, rangeSigma);
}

/**
 * @brief Apply the DeceivedScaledBilateral Filter to an image. Uses an UnSharp mask as deceiver.
 * Similar to the Deceived Bilateral Filter, but the weighting image is low pass filtered
 *
 * @param inputImage input image
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param spatialSima spatial standard deviation
 * @param rangeSigma range or radiometric standard deviation
 * @return Mat output image
 */
Mat DeWAFF::DeceivedScaledBilateralFilter(const Mat &inputImage, int windowSize, double spatialSigma, int rangeSigma) {
    // Pre-process the laplacian masked image
    Mat usmImage = lib.NonAdaptiveUSM(inputImage, windowSize, usmLambda, spatialSigma);
    // Calculate the deceived filter
    return ScaledBilateralFilter(inputImage, usmImage, windowSize, spatialSigma, rangeSigma);
}

/**
 * @brief Apply the DeceivedNonLocalMeansFilter to an image. Uses an UnSharp mask as deceiver.
 * Computationally demanding algorithm, can take as much as ten times more than the other filters
 * in the framework
 *
 * @param inputImage input image
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param patchSize spatial standard deviation
 * @param spatialSigma range or radiometric standard deviation
 * @param rangeSigma output image
 * @return Mat
 */
Mat DeWAFF::DeceivedNonLocalMeansFilter(const Mat &inputImage, int windowSize, int patchSize, double spatialSigma, int rangeSigma) {
    // Pre-process the laplacian masked image
    Mat usmImage = lib.NonAdaptiveUSM(inputImage, windowSize, usmLambda, spatialSigma);
    // Calculate the deceived filter
    return NonLocalMeansFilter(inputImage, usmImage, windowSize, patchSize, rangeSigma);
}

/**
 * @brief Apply the DeceivedGuidedFilter to an image. Uses an UnSharp mask as deceiver.
 * The fastest WAF for the DeWAFF yet
 *
 * @param inputImage input image
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param spatialSigma spatial standard deviation
 * @param rangeSigma range or radiometric standard deviation
 * @return Mat output image
 */
Mat DeWAFF::DeceivedGuidedFilter(const Mat &inputImage, int windowSize, double spatialSigma, int rangeSigma) {
    Mat usmImage = lib.NonAdaptiveUSM(inputImage, windowSize, usmLambda, spatialSigma);
    // Calculate the deceived filter
    return GuidedFilter(usmImage, inputImage, windowSize, rangeSigma);
}