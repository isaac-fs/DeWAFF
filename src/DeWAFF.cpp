#include "DeWAFF.hpp"

/**
 * @brief DeWAFF class constructor. Sets the lambda parameter for the Laplacian deceive
 *
 */
DeWAFF::DeWAFF(): usmLambda(2){}

/**
 * @brief Apply a Deceived Bilateral Filter to an image.
 *
 * @param inputImage input image
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param spatialSigma spatial standard deviation
 * @param rangeSigma range or radiometric standard deviation
 * @return
 * \f[ Y_{\phi_{\text BF}}(p) = \left( \sum_{m \subset \Omega} \phi_{\text BF}(U, m, p) \right)^{-1}
 * \left( \sum_{m \subset \Omega} \phi_{\text BF}(U, p, m) \, \hat{f}_{\text USM}(m) \right) \f]
 * where
 * \f[ \hat{f}_{\text USM} = U + \lambda \mathcal{L} \f]
 */
Mat DeWAFF::DeceivedBilateralFilter(const Mat &inputImage, int windowSize, double spatialSigma, double rangeSigma) {
    // Pre process the USM image
    Mat usmImage = lib.NonAdaptiveUSMFilter(inputImage, windowSize, usmLambda, spatialSigma);
    // Calculate the deceived filter
    return BilateralFilter(usmImage, inputImage, windowSize, spatialSigma, rangeSigma);
}

/**
 * @brief Apply a Deceived Scaled Bilateral Filter to an image.
 * Similar to the Deceived Bilateral Filter, but the weighting image is low pass filtered
 *
 * @param inputImage input image
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param spatialSigma spatial standard deviation
 * @param rangeSigma range or radiometric standard deviation
 * @return
 * \f[ Y_{\phi_{\text SBF}}(p) = \left( \sum_{m \subset \Omega} \phi_{\text SBF}(U^s, U, m, p) \right)^{-1}
 * \left( \sum_{m \subset \Omega} \phi_{\text SBF}(U^s, U, m, p) \, \hat{f}_{\text USM}(m) \right) \f]
 * where
 * \f[ \hat{f}_{\text USM} = U + \lambda \mathcal{L} \f]
 */
Mat DeWAFF::DeceivedScaledBilateralFilter(const Mat &inputImage, int windowSize, double spatialSigma, double rangeSigma) {
    // Pre process the USM image
    Mat usmImage = lib.NonAdaptiveUSMFilter(inputImage, windowSize, usmLambda, spatialSigma);
    // Calculate the deceived filter
    return ScaledBilateralFilter(usmImage, inputImage, windowSize, spatialSigma, rangeSigma);
}

/**
 * @brief Apply a Deceived Non Local Means Filter to an image.
 * Computationally demanding algorithm, can take as much as ten times more than the other filters
 * in the framework
 *
 * @param inputImage input image
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param neighborhoodSize spatial standard deviation
 * @param spatialSigma range or radiometric standard deviation
 * @param rangeSigma output image
 * @return
 * \f[ Y_{\phi_{\text NLM}}(p) = \left( \sum_{m \subset \Omega} \phi_{\text NLM}(U, m, p) \right)^{-1}
 * \left( \sum_{m \subset \Omega} \phi_{\text NLM}(U, p, m) \, \hat{f}_{\text USM}(m) \right) \f]
 * where
 * \f[ \hat{f}_{\text USM} = U + \lambda \mathcal{L} \f]
 */
Mat DeWAFF::DeceivedNonLocalMeansFilter(const Mat &inputImage, int windowSize, int neighborhoodSize, double spatialSigma, double rangeSigma) {
    // Pre process the USM image
    Mat usmImage = lib.NonAdaptiveUSMFilter(inputImage, windowSize, usmLambda, spatialSigma);
    // Calculate the deceived filter
    return NonLocalMeansFilter(usmImage, inputImage, windowSize, neighborhoodSize, rangeSigma);
}

/**
 * @brief Apply a Deceived Guided Filter to an image.
 * The fastest WAF for the DeWAFF yet
 *
 * @param inputImage input image
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param spatialSigma spatial standard deviation
 * @param rangeSigma range or radiometric standard deviation
 * @return
 * \f[ Y_{\phi_{\text GF}}(p) = \frac{1}{|\Omega|} \sum_{k:i \in \Omega_k} (a_k(U, \hat{f}_{\text USM}) \, U(m) + b_k(U, \hat{f}_{\text USM})\f]
 * where
 * \f[ \hat{f}_{\text USM} = U + \lambda \mathcal{L} \f]
 */
Mat DeWAFF::DeceivedGuidedFilter(const Mat &inputImage, int windowSize, double spatialSigma, double rangeSigma) {
    // Pre process the USM image
    Mat usmImage = lib.NonAdaptiveUSMFilter(inputImage, windowSize, usmLambda, spatialSigma);
    // Calculate the deceived filter
    return GuidedFilter(usmImage, inputImage, windowSize, rangeSigma);
}