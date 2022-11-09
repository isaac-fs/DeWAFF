#include "NonAdaptiveUSM.hpp"

/**
 * @brief Applies a regular non adaptive UnSharp mask (USM) with a Laplacian of Gaussian kernel
 * @param inputImage Image to apply the mask
 * @param lambda multiplying lambda factor for the USM
 * @return Filtered image
 */
Mat NonAdaptiveUSM::Filter(const Mat &inputImage, const int lambda){
	// Generate the Laplacian kernel
	int filterSize = 5;
	double sigma = 0.005;
	Mat kernel = -1 * NonAdaptiveUSM::LaplacianKernel(filterSize, sigma);

	// Create a new image with the size and type of the input image
	Mat laplacianImage(inputImage.size(), inputImage.type());

	// Apply the Laplacian filter
	Point anchor(-1, -1);
	double delta = 0.0;
	int ddepth = -1;
	filter2D(inputImage, laplacianImage, ddepth, kernel, anchor, delta, BORDER_DEFAULT);

	// Normalize the Laplacian filtered image
	double minL, maxL;
	Tools::getMinMax(abs(laplacianImage), &minL, &maxL);
	double minI, maxI;
	Tools::getMinMax(inputImage, &minI, &maxI);
	laplacianImage = maxI * (laplacianImage / maxL);

	/**
	 * Apply the UnSharp mask \f$ \hat{f}_{\text USM} = U + \lambda \, \mathcal{L} \text{ where } \mathcal{L} = l \, g\f$
	 */
	return (inputImage + lambda * laplacianImage);
}

/**
 * @brief Creates a Laplacian of Gaussian kernel. Same as fspecial('log',..) in Matlab
 * @param filterSize N size of NxN filter matrix. In this case a bigger N reduces more noise, but blures more the edges
 * @param sigma Standard deviation for the Gaussian kernel
 * @return Laplacian of Gaussian kernel matrix
 */
Mat NonAdaptiveUSM::LaplacianKernel(int filterSize, double sigma){
	Mat1f X, Y, X2, Y2, exponentialFactor, GaussianKernel, LaplacianKernel;
	/**
	 * Calculate the Gaussian kernel
	 * \f$ G_{\text kernel} = \frac{1}{2 \pi \sigma^2} \exp\left(-\frac{X^2 + Y^2}{2 \sigma^2}\right) \f$
	 */
	int size = (filterSize - 1) / 2;
	Tools::meshGrid(Range(-size, size), Range(-size, size), X, Y);
	pow(X, 2, X2); // X^2
	pow(Y, 2, Y2); // Y^2
	double variance = pow(sigma, 2);
	exp((X2 + Y2) / (-2 * variance), exponentialFactor);
	double scaleFactor = (1 / (2 * CV_PI * variance));
	GaussianKernel =  scaleFactor * exponentialFactor;

	// Normalization
	GaussianKernel /= sum(GaussianKernel).val[0];
	
	/**
	 * Calculate the Laplacian of the Gaussian kernel
	 * \f$ LoG_{\text kernel} = - \frac{\exp\left(-\frac{X^2 + Y^2}{2 \sigma^2}\right) - \frac{X^2 + Y^2}{2 \pi \sigma^2} \exp\left(-\frac{X^2 + Y^2}{2 \sigma^2}\right)}{\pi \sigma^4} \f$
	 * which reduces to 
	 * \f$ LoG_{\text kernel} = - \frac{1}{\pi \sigma^4} \left[ 1 - \frac{X^2 + Y^2}{2 \sigma^2} \right] \exp\left(-\frac{X^2 + Y^2}{2 \sigma^2}\right) \f$
	 */
	scaleFactor = -1 * (CV_PI * pow(variance, 2));
	LaplacianKernel = (exponentialFactor - GaussianKernel.mul(X2 + Y2)) / scaleFactor;
	
	// Scale the kernel so it sums to zero (High pass behavior of the derivative)
	scaleFactor = sum(LaplacianKernel).val[0] / pow(filterSize, 2);
	LaplacianKernel = LaplacianKernel - scaleFactor;

	return LaplacianKernel;
}