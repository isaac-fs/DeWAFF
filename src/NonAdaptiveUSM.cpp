#include "NonAdaptiveUSM.hpp"

/**
 * @brief Applies a regular non adaptive unsharped mask (USM) with a Laplacian of Gaussian kernel
 * @param inputImage Image to apply the mask
 * @param lambda multiplying factor for the USM
 * @return Masked image
 */
Mat NonAdaptiveUSM::Filter(const Mat &inputImage, const int lambda){
	// Generate the LoG kernel
	int filterSize = 5;
	double sigma = 0.005;
	Mat kernel = -1 * NonAdaptiveUSM::LaplacianKernel(filterSize, sigma);

	// Create a new image with the size and type of the input image
	Mat laplacianImage(inputImage.size(), inputImage.type());

	// Apply the LoG filter
	Point anchor(-1, -1);
	double delta = 0.0;
	int ddepth = -1;
	filter2D(inputImage, laplacianImage, ddepth, kernel, anchor, delta, BORDER_DEFAULT);

	// Normalize the LoG filtered image
	double minLoG, maxLoG;
	Tools::getMinMax(abs(laplacianImage), &minLoG, &maxLoG);
	double minIn, maxIn;
	Tools::getMinMax(inputImage, &minIn, &maxIn);
	laplacianImage = maxIn * (laplacianImage / maxLoG);

	// Apply the Unsharp mask
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

	// Calculate the Gaussian kernel
	int size = (filterSize - 1) / 2;
	Tools::meshGrid(Range(-size, size), Range(-size, size), X, Y);
	pow(X, 2, X2); // X^2
	pow(Y, 2, Y2); // Y^2
	double variance = pow(sigma, 2);
	exp((X2 + Y2) / (-2 * variance), exponentialFactor);
	double scaleFactor = (1 / (2 * CV_PI * variance));
	GaussianKernel =  scaleFactor * exponentialFactor; // GaussianKernel = (1/(2*PI*variance)) * e^(-(X^2 + Y^2)/(2*variance)

	// Normalization
	GaussianKernel /= sum(GaussianKernel).val[0];

	// Calculate the Laplacian of the Gaussian kernel
	scaleFactor = -1 * CV_PI * pow(variance, 2);
	LaplacianKernel = (exponentialFactor - GaussianKernel.mul(X2 + Y2)) / scaleFactor;
	
	// Scale the kernel so it sums to zero (High pass behavior of the derivative)
	scaleFactor = sum(LaplacianKernel).val[0] / pow(filterSize, 2);
	LaplacianKernel = LaplacianKernel - scaleFactor;

	return LaplacianKernel;
}