#include "DeWAFF.hpp"

DeWAFF::DeWAFF(const Mat &image):
    originalImage(image){
    lambda = 2;
    USMFilteredImage = NonAdaptiveUSM::Filter(originalImage, this->lambda);
};

/**
 * @brief
 * 
 */
Mat DeWAFF::deceivedBilateralFilter(int windowSize, double spatialSigma, int rangeSigma){
    // Pre-compute Gaussian domain weights.
    Mat1f X, Y, XYdiffSquared, spatialGaussianKernel;
    double spatialVariance = pow(spatialSigma, 2);
    Tools::meshGrid(Range(-windowSize, windowSize), Range(-windowSize, windowSize), X, Y);
    pow(X-Y, 2, XYdiffSquared);
    exp(XYdiffSquared * (-1 / (2 * spatialVariance)), spatialGaussianKernel);

    // Apply bilateral filter.
    double rangeVariance = pow(rangeSigma, 2);
    Mat BFImage = Mat(originalImage.size(), originalImage.type());
    Mat BilateralFilterKernel, rangeGaussianKernel, localRegion, LaplacianImageRegion, dL, da, db;
    int iMin, iMax, jMin, jMax;
    std::vector<Mat> channels(3), LaplacianChannels(3);
	Vec3f pixel;
	double normBF;

    #pragma omp parallel for\
    private(localRegion, iMin, iMax, jMin, jMax, pixel, channels, LaplacianChannels, dL, da, db, rangeGaussianKernel, BilateralFilterKernel, normBF, LaplacianImageRegion)\
	shared(originalImage, USMFilteredImage, BFImage, spatialGaussianKernel, windowSize, spatialSigma, rangeSigma)
    for(int i = 0; i < originalImage.rows; i++){
        for(int j = 0; j < originalImage.cols; j++){
            // Extract local region.
            iMin = max(i - windowSize, 0);
            iMax = min(i + windowSize, originalImage.rows-1);
            jMin = max(j - windowSize, 0);
            jMax = min(j + windowSize, originalImage.cols-1);
            localRegion = originalImage(Range(iMin, iMax), Range(jMin, jMax));

            // Compute Gaussian range weights in the three channels
            pixel = originalImage.at<Vec3f>(i, j);
            split(localRegion, channels);
            pow(channels[L] - pixel.val[L], 2, dL);
            pow(channels[a] - pixel.val[a], 2, da);
            pow(channels[b] - pixel.val[b], 2, db);
            exp((dL + da + db) / (-2 * rangeVariance), rangeGaussianKernel);
            
            // Calculate the bilateral filter kernel
            BilateralFilterKernel = rangeGaussianKernel.mul(spatialGaussianKernel(
            Range((iMin-i)+windowSize, iMax-i+windowSize),
            Range((jMin-j)+windowSize, (jMax-j)+windowSize))
            );
            normBF = sum(BilateralFilterKernel).val[0];        

            // The Laplacian deceive consists on weighting the Bilateral Filter kernel with the
            // original image values, but for image processing the Laplacian filtered image values are used
            LaplacianImageRegion = USMFilteredImage(Range(iMin, iMax), Range(jMin, jMax));
            split(LaplacianImageRegion, LaplacianChannels);
            BFImage.at<Vec3f>(i,j)[L] = (1/normBF) * sum(BilateralFilterKernel.mul(LaplacianChannels[L])).val[0];
            BFImage.at<Vec3f>(i,j)[a] = (1/normBF) * sum(BilateralFilterKernel.mul(LaplacianChannels[a])).val[0];
            BFImage.at<Vec3f>(i,j)[b] = (1/normBF) * sum(BilateralFilterKernel.mul(LaplacianChannels[b])).val[0];
        }
    }

    return BFImage;
}
