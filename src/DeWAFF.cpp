#include "DeWAFF.hpp"

/**
 * @brief Construct a new DeWAFF::DeWAFF object
 * 
 * @param image A CIELab image to be processed by the framework
 * 
 * This constructor initializes a reference to the image and starts the USM filtering
 * processing on it. It sets the lambda value for the Laplacian mask.
 */
DeWAFF::DeWAFF(const Mat &image):
    originalImage(image){
    lambda = 2;
    USMFilteredImage = NonAdaptiveUSM::Filter(originalImage, this->lambda);
};

/**
 * @brief A deceived Bilateral Filter implementatio for the DeWAFF
 * 
 * @param windowSize Window size for Gaussian kernels and the image region to process
 * @param spatialSigma Same as a BF spatial sigma. Is the standard deviation for the spatial kernel
 * @param rangeSigma Same as a BF range sigma. Is the standard deviation for the range (or radiometric) kernel
 * @return Mat A deceived Bilateral Filter processed image
 */
Mat DeWAFF::DeceivedBilateralFilter(int windowSize, double spatialSigma, int rangeSigma){
    /**
     * Compute a Gaussian kernel with the spatial values (pixel positions) of the window
     * \f$ G_{\text spatial}(m, p) = \exp\left(-\frac{X^2 - Y^2}{2{\sigma_s^2}}\right) \f$ \n
     * where \f$ m = (X, Y) \f$ is a pixel matrix with all the window pixels positions such that \f$ X = \{x_i, ..., x_n\} \text{ for each } 
     * n \in X \f$ and \f$ Y = \{y_i, ..., y_n\} \text{ for each } n \in Y \f$ contain the coordinates of each \f$ p = \{ x_i, y_i\}\f$ pixel
     * in \f$ m \f$
     */
    Mat1f X, Y, XYdiffSquared, spatialGaussianKernel;
    double spatialVariance = pow(spatialSigma, 2); // The variance is the squared standard deviation
    Tools::meshGrid(Range(-windowSize, windowSize), Range(-windowSize, windowSize), X, Y); // Generate mesh grid X and Y coordinates
    pow(X-Y, 2, XYdiffSquared);
    exp(XYdiffSquared * (-1 / (2 * spatialVariance)), spatialGaussianKernel);

    // Apply the bilateral filter
    Mat BFImage = Mat(originalImage.size(), originalImage.type());
    Mat BilateralFilterKernel, rangeGaussianKernel, localRegion, LaplacianImageRegion, dL, da, db;
    double rangeVariance = pow(rangeSigma, 2);
    int iMin, iMax, jMin, jMax;
    std::vector<Mat> channels(3), LaplacianChannels(3);
	Vec3f pixel;
	double normBF;

    // Set the parallelization pragma
    #pragma omp parallel for\
    private(localRegion, iMin, iMax, jMin, jMax, pixel, channels, LaplacianChannels, dL, da, db, rangeGaussianKernel, BilateralFilterKernel, normBF, LaplacianImageRegion)\
	shared(originalImage, USMFilteredImage, BFImage, spatialGaussianKernel, windowSize, spatialSigma, rangeSigma)
    for(int i = 0; i < originalImage.rows; i++){
        for(int j = 0; j < originalImage.cols; j++){
            // Extract local region based on the window size
            iMin = max(i - windowSize, 0);
            iMax = min(i + windowSize, originalImage.rows-1);
            jMin = max(j - windowSize, 0);
            jMax = min(j + windowSize, originalImage.cols-1);
            localRegion = originalImage(Range(iMin, iMax), Range(jMin, jMax));
            
            /**
            * Compute a Gaussian kernel with the range values (pixel intensities) of the image channels
            * \f$ G_{\text range}(U, m, p) = \exp\left(-\frac{U(X)^2 - U(Y)^2}{2{\sigma_r^2}}\right) \f$
            * where \f$ U(m) \f$ is the image region of image U.
            * In this case a pair of \f$ X_{\text channel} \f$ and \f$ Y_{\text channel} \f$ is computed for each
            * CIELab channel
            */
            pixel = originalImage.at<Vec3f>(i, j);
            split(localRegion, channels);
            pow(channels[L] - pixel.val[L], 2, dL);
            pow(channels[a] - pixel.val[a], 2, da);
            pow(channels[b] - pixel.val[b], 2, db);
            exp((dL + da + db) / (-2 * rangeVariance), rangeGaussianKernel);
            
            /**
            * Apply the bilateral filter kernel
            * \f$ \varphi_{\text BF}(U, m, p) = \sum_{m \in \Omega} G_{\text spatial}(m, p) \, G_{\text range}(U, m, p) \f$
            * where \f$ \Omega \f$ is the selected window
            */
            BilateralFilterKernel = rangeGaussianKernel.mul(spatialGaussianKernel(
            Range((iMin-i)+windowSize, iMax-i+windowSize),
            Range((jMin-j)+windowSize, (jMax-j)+windowSize))
            );

            /**
            * Calculate the Bilateral filter norm
            * \f$ \frac{1}{\varphi_{\text BF}} \f$
            */
            normBF = sum(BilateralFilterKernel).val[0];

            /** The Laplacian deceive consists on weighting the Bilateral Filter kernel with the
            * original image values, but the image that is used in the filter is the Laplacian image (previously USM filtered)
            * \f$ Y_{\varphi_{\text BF}}(p) = \sum_{m \in \Omega} \varphi_{\text BF}(U, p, m) \, \hat{f}_{\text USM}(m) \f$ 
            */
            LaplacianImageRegion = USMFilteredImage(Range(iMin, iMax), Range(jMin, jMax));
            split(LaplacianImageRegion, LaplacianChannels);
            BFImage.at<Vec3f>(i,j)[L] = (1/normBF) * sum(BilateralFilterKernel.mul(LaplacianChannels[L])).val[0];
            BFImage.at<Vec3f>(i,j)[a] = (1/normBF) * sum(BilateralFilterKernel.mul(LaplacianChannels[a])).val[0];
            BFImage.at<Vec3f>(i,j)[b] = (1/normBF) * sum(BilateralFilterKernel.mul(LaplacianChannels[b])).val[0];
        }
    }

    return BFImage;
}
