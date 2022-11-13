#include "DeWAFF.hpp"

/**
 * @brief Construct a new DeWAFF::DeWAFF object
 * Initializes all relevant variables for the chosen filtering method
 * 
 */
DeWAFF::DeWAFF(){
    // Initial values. These are the main parameters for the DeWAFF
    this->windowSize = 14;
     // Check the windows size
    if (windowSize < 3) {
        std::cout
        << "Window size must be greater than 3!"
        << std::endl
        << "Check the window size in the DeWAFF constructor"
        << std::endl;
        exit(-1);
    }

     // Set processing values
    this->lambda = 2; // Lambda value for the USM
    this->spatialSigma = windowSize/1.5;
    this->rangeSigma = 10;


    // Variance calculations
    this->spatialVariance = pow(spatialSigma, 2);
    this->rangeVariance = pow(rangeSigma, 2);

    // Pre computation of meshgrid values
    this->range = Range(-(windowSize/2), (windowSize/2) + windowSize%2);
    Tools::meshGrid(range, this->X, this->Y);

    // Pre computation of the range (intensity) independant gaussian kernel values
    pow(this->X, 2, this->XX);
	pow(this->Y, 2, this->YY);
    this->exponentialFactor = GaussianExponentialFactor();
    this->gaussianKernel = GaussianKernel();
};

/**
 * @brief A deceived Bilateral Filter implementatio for the DeWAFF
 * @param inputImage A CIELab image to be processed by the framework
 * @return Mat A deceived Bilateral Filter processed image
 * 
 */
Mat DeWAFF::DeceivedBilateralFilter(const Mat &inputImage){
    // Pre-process the laplacian masked image
    this->inputImage = inputImage;
    laplacianFilteredImage = LaplacianFilter(inputImage);

    /**
    * Compute a spatial Gaussian kernel \f$ G_{\text{spatial}}(U, m, p) = \exp\left(-\frac{ ||m - p||^2 }{ 2 {\sigma_s^2} } \right) \f$ 
    * with the spatial values (pixel positions) from an image region \f$ \Omega \in U \f$.
    * The spatial kernel uses the \f$ m_i \in \Omega \f$ pixels coordinates as weighting values for the pixel \f$ p = (x, y) \f$
    * 
    */
    Mat1f spatialGaussianKernel = gaussianKernel;

    // Prepare variables for the bilateral filtering
    Mat outputImage = Mat(inputImage.size(), inputImage.type());
    Mat bilateralFilterKernel, rangeGaussianKernel, localRegion, laplacianImageRegion, dL, da, db;
	double bilateralFilterNorm;
    int iMin, iMax, jMin, jMax;
    Range xRange, yRange;
	Vec3f pixel;
    std::vector<Mat> channels(3), laplacianChannels(3);

    // Set the parallelization pragma for OpenMP
    #pragma omp parallel for\
    private(iMin, iMax, jMin, jMax,\
            xRange, yRange,\
            localRegion,\
            pixel,\
            channels, dL, da, db,\
            rangeGaussianKernel, bilateralFilterKernel,\
            bilateralFilterNorm,\
            laplacianImageRegion, laplacianChannels)\
	shared( inputImage,\
            windowSize, \
            spatialSigma,\
            rangeSigma,\
            laplacianFilteredImage,\
            outputImage)
    for(int i = 0; i < inputImage.rows; i++){
        iMin = max(i - windowSize, i);
        iMax = min(i + windowSize, inputImage.rows);
        xRange = Range(iMin, iMax);

        for(int j = 0; j < inputImage.cols; j++){
            jMin = max(j - windowSize, j);
            jMax = min(j + windowSize, inputImage.cols);
            yRange = Range(jMin, jMax);

            // Extract local region based on the window size
            localRegion = inputImage(xRange, yRange);

            // Add padding to the region to fit the convolution if necessary
            if(localRegion.size() != spatialGaussianKernel.size())
                Tools::addPadding(localRegion, spatialGaussianKernel.rows, spatialGaussianKernel.cols);
 

            /**
            * Compute a range Gaussian kernel \f$ G_{\text{range}}(U, m, p) = \exp\left( -\frac{ ||U(m) - U(p)||^2 }{ 2{\sigma_s^2} } \right) \f$
            * with the range values (pixel intensities) from an image region \f$ \Omega \in U \f$.
            * The range kernel uses the \f$ m_i \in \Omega \f$ pixels intensities as weighting values for the pixel \f$ p = (x, y) \f$ instead of their
            * locations as in the spatial kernel computation. In this case a the image \f$ U \f$ is separated into the three CIELab channels and each 
            * channel is processed as an individual image \f$ U_{\text{channel}} \f$
            */
            split(localRegion, channels);
            pixel = inputImage.at<Vec3f>(i, j);
            pow(channels[L] - pixel.val[L], 2, dL);
            pow(channels[a] - pixel.val[a], 2, da);
            pow(channels[b] - pixel.val[b], 2, db);
            exp((dL + da + db) / (-2 * rangeVariance), rangeGaussianKernel);
            rangeGaussianKernel *= 1 / (2 * CV_PI * rangeVariance);
            
            /**
            * Convolute the spatial and range gaussian kernels to obtain the bilateral filter kernel
            * \f$ \phi_{\text{BF}}(U, m, p) = G_{\text{spatial}}(|| m-p ||) \, G_{\text{range}}(|| U(m)-U(p) ||) \f$
            *
            */
            bilateralFilterKernel = spatialGaussianKernel.mul(rangeGaussianKernel);
            
            /**
            * Calculate the Bilateral filter's norm
            * \f$ \left( \sum_{m \in \Omega} \phi_{\text{BF}}(U, m, p) \right)^{-1} \f$
            */
            bilateralFilterNorm = sum(bilateralFilterKernel).val[0];

            /** 
             * Apply the bilateral filter kernel to the laplacian image. The Laplacian deceive consists on weighting the Bilateral Filter kernel with the
            * original image and use the USM image as input for the filter
            * \f$ Y_{\phi_{\text BF}}(p) = \left( \sum_{m \in \Omega} \phi_{\text{BF}}(U, m, p) \right)^{-1}
            * \left( \sum_{m \in \Omega} \phi_{\text BF}(U, p, m) \, \hat{f}_{\text USM}(m) \right) \f$
            */
            laplacianImageRegion = laplacianFilteredImage(xRange, yRange);
            if(laplacianImageRegion.size() != localRegion.size())
                Tools::addPadding(laplacianImageRegion, localRegion.rows, localRegion.cols);

            split(laplacianImageRegion, laplacianChannels);
            outputImage.at<Vec3f>(i,j)[L] = (1/bilateralFilterNorm) * sum(bilateralFilterKernel.mul(laplacianChannels[L])).val[0];
            outputImage.at<Vec3f>(i,j)[a] = (1/bilateralFilterNorm) * sum(bilateralFilterKernel.mul(laplacianChannels[a])).val[0];
            outputImage.at<Vec3f>(i,j)[b] = (1/bilateralFilterNorm) * sum(bilateralFilterKernel.mul(laplacianChannels[b])).val[0];
        }
    }

    return outputImage;
}

/**
 * @brief Applies a regular non adaptive UnSharp mask (USM) with a Laplacian of Gaussian kernel
 * \f$ \hat{f}_{\text USM} = U + \lambda \mathcal{L} \text{ where } \mathcal{L} = l * g\f$. 
 * Here \f$ g \f$ is a Gaussian kernel and \f$ l \f$ a Laplacian kernel, hence the name "Laplacian of Gaussian"
 * @param inputImage Image to apply the mask
 * @return Filtered image
 * 
 */
Mat DeWAFF::LaplacianFilter(const Mat &inputImage){
	// Generate the Laplacian kernel
	Mat laplacianKernel = -1 * DeWAFF::LaplacianKernel();
    
	// Create a new image with the size and type of the input image
	Mat laplacianImage(inputImage.size(), inputImage.type());

	// Apply the Laplacian filter
	Point anchor(-1, -1);
	double delta = 0.0;
	int ddepth = -1;
	filter2D(inputImage, laplacianImage, ddepth, laplacianKernel, anchor, delta, BORDER_DEFAULT);

	// Normalize the Laplacian filtered image
	double minL, maxL;
	Tools::getMinMax(abs(laplacianImage), &minL, &maxL);
	double minI, maxI;
	Tools::getMinMax(inputImage, &minI, &maxI);
	laplacianImage = maxI * (laplacianImage / maxL);

	return (inputImage + lambda * laplacianImage);
}

/**
 * @brief Creates a Laplacian of Gaussian kernel. Same as fspecial('log',..) in Matlab.
 * Calculates the Laplacian of the Gaussian kernel
 * \f$ LoG_{\text kernel} = - \frac{\exp\left(-\frac{X^2 + Y^2}{2 \sigma^2}\right) - \frac{X^2 + Y^2}{2 \pi \sigma^2} \exp\left(-\frac{X^2 + Y^2}{2 \sigma^2}\right)}{\pi \sigma^4} \f$
 * which reduces to 
 * \f$ LoG_{\text kernel} = - \frac{1}{\pi \sigma^4} \left[ 1 - \frac{X^2 + Y^2}{2 \sigma^2} \right] \exp\left(-\frac{X^2 + Y^2}{2 \sigma^2}\right) \f$
 *
 * @return Laplacian of Gaussian kernel matrix
 * 
 */
Mat1f DeWAFF::LaplacianKernel(){
    // Calculate the laplacian kernel
	Mat1f laplacianKernel = (-1 / (CV_PI * pow(spatialVariance, 2))) * (exponentialFactor - gaussianKernel.mul(XX + YY));

	// Scale the kernel so it sums to zero (High pass behavior of the derivative)
    double deltaFactor = sum(laplacianKernel).val[0] / pow(windowSize, 2);
	laplacianKernel = laplacianKernel - deltaFactor;

	return laplacianKernel;
};

/**
 * @brief Calculates the Gaussian kernel \f$ G_{\text kernel} = \frac{1}{2 \pi \sigma^2} \exp\left(-\frac{X^2 + Y^2}{2 \sigma^2}\right) \f$
 * @return Mat1f 
 * 
 */
Mat1f DeWAFF::GaussianKernel(){
    // Calculate the kernel
	Mat1f gaussianKernel = (1 / (2 * CV_PI * spatialVariance)) * exponentialFactor;

	// Normalize the kernel
	gaussianKernel /= sum(gaussianKernel).val[0];

    return gaussianKernel;
};

/**
 * @brief Calculate the exponential factor for the Gaussian kernel
 * \f$ e_{\text{Gaussian}} = \exp\left(-\frac{X^2 + Y^2}{2 \sigma^2}\right) \f$
 * @return Mat1f
 * 
 */
Mat1f DeWAFF::GaussianExponentialFactor() {
    Mat1f exponentialFactor;
	exp((XX + YY) * (-1 / (2 * spatialVariance)), exponentialFactor);

    return exponentialFactor;
};