#include "DeWAFF.hpp"


DeWAFF::DeWAFF(){
    this->usmLambda = 2;
}

Mat DeWAFF::DeceivedBilateralFilter(const Mat &inputImage, int windowSize, double spatialSigma, int rangeSigma) {
    // Pre-process the laplacian masked image
    Mat usmImage = Utils::NonAdaptiveUSM(inputImage, windowSize, usmLambda, spatialSigma);

    // Calculate the deceived bilateral filter
    return BilateralFilter(inputImage, usmImage, windowSize, spatialSigma, rangeSigma);
}

Mat DeWAFF::DeceivedScaledBilateralFilter(const Mat &inputImage, int windowSize, double spatialSigma, int rangeSigma) {
    // Pre-process the laplacian masked image
    Mat usmImage = Utils::NonAdaptiveUSM(inputImage, windowSize, usmLambda, spatialSigma);
    return ScaledBilateralFilter(inputImage, usmImage, windowSize, spatialSigma, rangeSigma);
}

Mat DeWAFF::DeceivedNonLocalMeansFilter(const Mat &inputImage, int windowSize, int patchSize, double spatialSigma, int rangeSigma) {
    // Pre-process the laplacian masked image
    Mat usmImage = Utils::NonAdaptiveUSM(inputImage, windowSize, usmLambda, spatialSigma);
    return NonLocalMeansFilter(inputImage, usmImage, windowSize, patchSize, rangeSigma);
}

Mat DeWAFF::DeceivedGuidedFilter(const Mat &inputImage, int windowSize, double spatialSigma, int rangeSigma) {
    Mat usmImage = Utils::NonAdaptiveUSM(inputImage, windowSize, usmLambda, spatialSigma);
    return GuidedFilter(usmImage, inputImage, windowSize, rangeSigma);
}