#include "DeWAFF.hpp"

/**
  * @brief Construct a new DeWAFF::DeWAFF object
 * Initializes all relevant variables for the chosen filtering method
 *
 * @param inputImage
 * @param windowSize
 * @param spatialSigma
 * @param rangeSigma
 */
DeWAFF::DeWAFF(int windowSize, int lambda, double spatialSigma, int rangeSigma) {
    // Set Window size
    this->windowSize = windowSize;

     // Set processing values
    this->lambda = lambda; // Lambda value for the USM
    this->spatialSigma = spatialSigma;// windowSize/1.5;
    this->rangeSigma = rangeSigma;

    // Variance calculations
    spatialVariance = pow(spatialSigma, 2);
    rangeVariance = pow(rangeSigma, 2);

    // Set the paddingvalue
    padding = (windowSize - 1) / 2;

    // Pre computation of meshgrid values
    range = Range(-(windowSize/2), (windowSize/2) + 1);
    Tools::meshGrid(range, this->X, this->Y);

    /**
     * @brief Pre computates the spatial Gaussian kernel
     *  Calculate the kernel variable \f$ S = X^2 + Y^2 \f$
     */
    pow(this->X, 2, this->X);
    pow(this->Y, 2, this->Y);
    S = X + Y;

    /*
    * @brief Compute a spatial Gaussian kernel \f$ G_{\text spatial}(U, m, p) = \exp\left(-\frac{ ||m - p||^2 }{ 2 {\sigma_s^2} } \right) \f$
    * with the spatial values (pixel positions) from an image region \f$ \Omega \in U \f$.
    * The spatial kernel uses the \f$ m_i \in \Omega \f$ pixels coordinates as weighting values for the pixel \f$ p = (x, y) \f$.
    * Normalize it with \f$\fract{1}{\sum G_{\text kernel}}\f$ so it sums to 1 for low pass filter behavior consistency
    */
    exp(S * (-1 / (2 * spatialVariance)), spatialGaussianKernel);
    spatialGaussianKernel /= sum(spatialGaussianKernel).val[0];

    /**
     * @brief Compute a Laplacian of Gaussian kernel. Same as fspecial('log',..) in Matlab.
     * \f$ \text{LoG}_{\text kernel} = - \frac{1}{\pi \sigma^4} \left[ 1 - \frac{X^2 + Y^2}{2 \sigma^2} \right] \exp\left(-\frac{X^2 + Y^2}{2 \sigma^2}\right) \f$
     * and normalize it with \f$ \frac{\sum \text{LoG} }{|\text{LoG}|}\f$ where \f$ |\text{LoG}| \f$ is the number of elements in \f$ \text{LoG} \f$ so it
     * sums to 0 for high pass filter behavior consistency
     */
    LoGkernel = (-1 / (CV_PI * pow(spatialVariance, 2))) * (1 - (S / (2 * spatialVariance))).mul(spatialGaussianKernel);
    double deltaFactor = sum(LoGkernel).val[0] / pow(windowSize, 2);
	LoGkernel = LoGkernel - deltaFactor;
}

/**
 * @brief A deceived Bilateral Filter implementatio for the DeWAFF
 * @param inputImage A CIELab image to be processed by the framework
 * @return Mat A deceived Bilateral Filter processed image
 *
 */
Mat DeWAFF::DeceivedBilateralFilter(const Mat &inputImage) {
    // Add padding to the image for kernel consistency
    copyMakeBorder(inputImage, this->image, padding, padding, padding, padding, BORDER_REPLICATE);

    // Pre-process the laplacian masked image
    laplacianImage = LaplacianFilter(image);

    // Prepare variables for the bilateral filtering
    Mat outputImage = Mat(image.size(), image.type());
    Mat bilateralKernel, rangeGaussianKernel, imageRegion, laplacianImageRegion, dL, da, db;
	double bilateralKernelNorm;
    int iMin, iMax, jMin, jMax;
    Range xRange, yRange;
	Vec3f pixel;
    Mat channels[3], laplacianChannels[3];

    // Set the parallelization pragma for OpenMP
    #pragma omp parallel for\
    private(iMin, iMax, jMin, jMax,\
            xRange, yRange,\
            imageRegion,\
            pixel,\
            channels, dL, da, db,\
            rangeGaussianKernel, bilateralKernel,\
            bilateralKernelNorm,\
            laplacianImageRegion, laplacianChannels)\
	shared( image,\
            windowSize, \
            spatialSigma,\
            rangeSigma,\
            laplacianImage,\
            outputImage)
    for(int i = padding; i < image.rows - padding; i++) {
        iMin = i - padding;
        iMax = iMin + windowSize;
        xRange = Range(iMin, iMax);

        for(int j = padding; j < image.cols - padding; j++) {
            jMin = j -  padding;
            jMax = jMin + windowSize;
            yRange = Range(jMin, jMax);

            // Extract local region based on the window size
            imageRegion = image(xRange, yRange);

            /**
            * Compute a range Gaussian kernel \f$ G_{\text range}(U, m, p) = \exp\left( -\frac{ ||U(m) - U(p)||^2 }{ 2{\sigma_s^2} } \right) \f$
            * with the range values (pixel intensities) from an image region \f$ \Omega \in U \f$.
            * The range kernel uses the \f$ m_i \in \Omega \f$ pixels intensities as weighting values for the pixel \f$ p = (x, y) \f$ instead of their
            * locations as in the spatial kernel computation. In this case a the image \f$ U \f$ is separated into the three CIELab channels and each
            * channel is processed as an individual image \f$ U_{\text channel} \f$
            */
            split(imageRegion, channels);
            pixel = image.at<Vec3f>(i, j);
            pow(channels[L] - pixel.val[L], 2, dL);
            pow(channels[a] - pixel.val[a], 2, da);
            pow(channels[b] - pixel.val[b], 2, db);
            exp((dL + da + db) / (-2 * rangeVariance), rangeGaussianKernel);

            /**
            * Convolute the spatial and range gaussian kernels to obtain the bilateral filter kernel
            * \f$ \phi_{\text BF}(U, m, p) = G_{\text spatial}(|| m-p ||) \, G_{\text range}(|| U(m)-U(p) ||) \f$
            *
            */
            bilateralKernel = spatialGaussianKernel.mul(rangeGaussianKernel);

            /**
            * Calculate the Bilateral filter's norm
            * \f$ \left( \sum_{m \in \Omega} \phi_{\text{BF}}(U, m, p) \right)^{-1} \f$
            */
            bilateralKernelNorm = sum(bilateralKernel).val[0];

            /**
             * Apply the bilateral filter kernel to the laplacian image. The Laplacian deceive consists on weighting the Bilateral Filter kernel with the
            * original image and use the USM image as input for the filter
            * \f$ Y_{\phi_{\text BF}}(p) = \left( \sum_{m \in \Omega} \phi_{\text BF}(U, m, p) \right)^{-1}
            * \left( \sum_{m \in \Omega} \phi_{\text BF}(U, p, m) \, \hat{f}_{\text USM}(m) \right) \f$
            */
            laplacianImageRegion = laplacianImage(xRange, yRange);
            split(laplacianImageRegion, laplacianChannels);
            outputImage.at<Vec3f>(i,j)[L] = (1/bilateralKernelNorm) * sum(bilateralKernel.mul(laplacianChannels[L])).val[0];
            outputImage.at<Vec3f>(i,j)[a] = (1/bilateralKernelNorm) * sum(bilateralKernel.mul(laplacianChannels[a])).val[0];
            outputImage.at<Vec3f>(i,j)[b] = (1/bilateralKernelNorm) * sum(bilateralKernel.mul(laplacianChannels[b])).val[0];
        }
    }

    return outputImage(Range(padding, inputImage.rows + padding), Range(padding, inputImage.cols + padding));
}

Mat DeWAFF::DeceivedNonLocalMeansFilter(const Mat &inputImage){
    // Add padding to the image for kernel consistency
    copyMakeBorder(inputImage, this->image, padding, padding, padding, padding, BORDER_REPLICATE);

    // Pre-process the laplacian masked image
    laplacianImage = LaplacianFilter(image);
    int subWindowSize = 3;

    // Prepare variables for the bilateral filtering
    Mat outputImage = Mat(image.size(), image.type());
    Mat nonLocalMeansKernel, euclideanDistanceKernel, imageRegion, laplacianImageRegion, dL, da, db;
	double nonLocalMeansKernelNorm;
    int iMin, iMax, jMin, jMax;
    Range xRange, yRange;
	Vec3f pixel;
    Mat channels[3], euclideanChannels[3], laplacianChannels[3];

    // Set the parallelization pragma for OpenMP
    #pragma omp parallel for\
    private(iMin, iMax, jMin, jMax,\
            xRange, yRange,\
            imageRegion,\
            pixel,\
            channels, dL, da, db,\
            euclideanDistanceKernel, nonLocalMeansKernel,\
            nonLocalMeansKernelNorm,\
            laplacianImageRegion, euclideanChannels, laplacianChannels)\
	shared( image,\
            windowSize, \
            spatialSigma,\
            rangeSigma,\
            laplacianImage,\
            outputImage)
    for(int i = padding; i < image.rows - padding; i++) {
        iMin = i - padding;
        iMax = iMin + windowSize;
        xRange = Range(iMin, iMax);

        for(int j = padding; j < image.cols - padding; j++) {
            jMin = j -  padding;
            jMax = jMin + windowSize;
            yRange = Range(jMin, jMax);

            // Extract local region based on the window size
            imageRegion = image(xRange, yRange);

            /**
            * Compute a range Gaussian kernel \f$ G_{\text range}(U, m, p) = \exp\left( -\frac{ ||U(m) - U(p)||^2 }{ 2{\sigma_s^2} } \right) \f$
            * with the range values (pixel intensities) from an image region \f$ \Omega \in U \f$.
            * The range kernel uses the \f$ m_i \in \Omega \f$ pixels intensities as weighting values for the pixel \f$ p = (x, y) \f$ instead of their
            * locations as in the spatial kernel computation. In this case a the image \f$ U \f$ is separated into the three CIELab channels and each
            * channel is processed as an individual image \f$ U_{\text channel} \f$
            */
            split(imageRegion, channels);
            euclideanChannels[L] = EuclideanDistanceKernel(channels[L], subWindowSize);
            euclideanChannels[a] = EuclideanDistanceKernel(channels[a], subWindowSize);
            euclideanChannels[b] = EuclideanDistanceKernel(channels[b], subWindowSize);
            euclideanDistanceKernel = euclideanChannels[L] + euclideanChannels[a] + euclideanChannels[b];

            /**
            * Convolute the spatial and range gaussian kernels to obtain the bilateral filter kernel
            * \f$ \phi_{\text BF}(U, m, p) = G_{\text spatial}(|| m-p ||) \, G_{\text range}(|| U(m)-U(p) ||) \f$
            *
            */
            nonLocalMeansKernel = spatialGaussianKernel.mul(euclideanDistanceKernel);

            /**
            * Calculate the Bilateral filter's norm
            * \f$ \left( \sum_{m \in \Omega} \phi_{\text{BF}}(U, m, p) \right)^{-1} \f$
            */
            nonLocalMeansKernelNorm = sum(nonLocalMeansKernel).val[0];

            /**
             * Apply the bilateral filter kernel to the laplacian image. The Laplacian deceive consists on weighting the Bilateral Filter kernel with the
            * original image and use the USM image as input for the filter
            * \f$ Y_{\phi_{\text BF}}(p) = \left( \sum_{m \in \Omega} \phi_{\text BF}(U, m, p) \right)^{-1}
            * \left( \sum_{m \in \Omega} \phi_{\text BF}(U, p, m) \, \hat{f}_{\text USM}(m) \right) \f$
            */
            laplacianImageRegion = laplacianImage(xRange, yRange);
            split(laplacianImageRegion, laplacianChannels);
            outputImage.at<Vec3f>(i,j)[L] = (1/nonLocalMeansKernelNorm) * sum(nonLocalMeansKernel.mul(laplacianChannels[L])).val[0];
            outputImage.at<Vec3f>(i,j)[a] = (1/nonLocalMeansKernelNorm) * sum(nonLocalMeansKernel.mul(laplacianChannels[a])).val[0];
            outputImage.at<Vec3f>(i,j)[b] = (1/nonLocalMeansKernelNorm) * sum(nonLocalMeansKernel.mul(laplacianChannels[b])).val[0];
        }
    }

    return outputImage(Range(padding, inputImage.rows + padding), Range(padding, inputImage.cols + padding));
}

/**
 * @brief Applies a regular non adaptive UnSharp mask (USM) with a Laplacian of Gaussian kernel
 * \f$ \hat{f}_{\text USM} = U + \lambda \mathcal{L} \text{ where } \mathcal{L} = l * g \f$.
 * Here \f$ g \f$ is a Gaussian kernel and \f$ l \f$ a Laplacian kernel, hence the name "Laplacian of Gaussian"
 * @param image Image to apply the mask
 * @return Filtered image
 *
 */
Mat DeWAFF::LaplacianFilter(const Mat &inputImage) {
	// Generate the Laplacian kernel
	Mat kernel = -1 * LoGkernel;

	// Create a new image with the size and type of the input image
	Mat laplacianImage(image.size(), image.type());

	// Apply the Laplacian filter
	Point anchor(-1, -1);
	double delta = 0.0;
	int ddepth = -1;
	filter2D(image, laplacianImage, ddepth, kernel, anchor, delta, BORDER_DEFAULT);

	// Normalize the Laplacian filtered image
	double minL, maxL;
	Tools::getMinMax(abs(laplacianImage), &minL, &maxL);
	double minI, maxI;
	Tools::getMinMax(image, &minI, &maxI);
	laplacianImage = maxI * (laplacianImage / maxL);

	return (image + lambda * laplacianImage);
}

Mat DeWAFF::EuclideanDistanceKernel(const Mat& imageRegion, int subWindowSize) {
    // Add padding to the image for kernel consistency
    int subPadding = (subWindowSize - 1) / 2;
    Mat paddedRegion;
    copyMakeBorder(imageRegion, paddedRegion, subPadding, subPadding*2, subPadding, subPadding*2, BORDER_REPLICATE);

    // Fixed pixel sub region
    int windowSizeRatio = windowSize / subWindowSize;

    Mat fixedSubRegion = imageRegion(Range(windowSizeRatio, subWindowSize + windowSizeRatio), Range(windowSizeRatio, subWindowSize + windowSizeRatio));

    // Initialize the output matrix
    Mat euclideanDistanceKernel = Mat(imageRegion.size(), imageRegion.type());

    // Moving pixel sub region
    Mat movingSubRegion = Mat(fixedSubRegion.size(), fixedSubRegion.type());

    // Parameter
    int h = 2 * pow(rangeSigma, 2);

    // Ranges
    Range xRange, yRange;

    // Visit each pixel in the image region
    for(int i = 0; i < imageRegion.rows; i++) {
        xRange = Range(i, i + subWindowSize);
        for(int j = 0; j < imageRegion.cols; j++) {
            yRange = Range(j, j + subWindowSize);
            //Extract pixel w neighborhood local region
            movingSubRegion = paddedRegion(xRange, yRange);

            // Calculate the norm between the fixed and moving region
            euclideanDistanceKernel.at<float>(i, j) = (float) norm(fixedSubRegion, movingSubRegion, NORM_L2SQR);
        }
    }

    cv::exp((-1 * euclideanDistanceKernel) / pow(h, 2), euclideanDistanceKernel);

    return euclideanDistanceKernel;
}

