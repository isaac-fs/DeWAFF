#include "NonAdaptiveUSM.hpp"

/**
 * @brief Applies a rgular non adaptive unsharped mask (USM) with a Laplacian of Gaussian kernel
 * @param inputImage Image to apply the mask
 * @param lambda multiplying factor for the USM
 * @return Masked image
 */
Mat NonAdaptiveUSM::nonAdaptiveUSM(const Mat &inputImage, const int lambda)
{
	// Generate the LoG kernel
	Mat kernel = -1 * NonAdaptiveUSM::LoGkernel(17, 0.005);
	// Create a new image with the size and type of the input image
	Mat LoGImage(inputImage.size(), inputImage.type());

	// Apply the LoG filter
	Point anchor(-1, -1);
	double delta = 0.0;
	int ddepth = -1;
	filter2D(inputImage, LoGImage, ddepth, kernel, anchor, delta, BORDER_DEFAULT);

	// Normalize the LoG filtered image
	double minLoG, maxLoG;
	Tools::getMinMax(abs(LoGImage), &minLoG, &maxLoG);
	double minIn, maxIn;
	Tools::getMinMax(inputImage, &minIn, &maxIn);
	LoGImage = maxIn * (LoGImage / maxLoG);

	// Apply the Unsharp mask
	return (inputImage + lambda * LoGImage);
}

/**
 * @brief Creates a Laplacian of Gaussian kernel. Same as fspecial('log',..) in Matlab
 * @param filterSize N size of NxN filter matrix. In this case a bigger N reduces more noise, but blures more the edges
 * @param sigma Standard deviation for the Gaussian kernel
 * @return Laplacian of Gaussian kernel matrix
 */
Mat NonAdaptiveUSM::LoGkernel(int filterSize, double sigma)
{
	Mat1f X, Y, X2, Y2, X2Y2, e, G, LoG;

	// Calculate the Gaussian kernel
	int size = (filterSize - 1) / 2;
	double eps = pow(2, -52);
	double std2 = pow(sigma, 2);
	Tools::meshGrid(Range(-size, size), Range(-size, size), X, Y);
	pow(X, 2, X2);									   // X^2
	pow(Y, 2, Y2);									   // Y^2
	X2Y2 = X2 + Y2;									   // X^2 + Y^2
	exp(X2Y2 / (-2 * std2), e);						   // e = e^(-(X^2 + Y^2)/(2*std2))
	e.convertTo(G, CV_32F, 1, 1 / (2 * CV_PI * std2)); // G = (1/(2*PI*std2))*e^(-(X^2 + Y^2)/(2*std2)

	double minG, maxG;
	Tools::getMinMax(G, &minG, &maxG);
	threshold(G, G, eps * maxG, 1.0, THRESH_TOZERO);

	Scalar sumG = sum(G);
	if (sumG.val[0] != 0)
		G /= sumG.val[0];

	// Calculate the Laplacian of the Gaussian kernel
	LoG = e - G.mul(X2Y2);
	LoG /= -1*CV_PI*pow(std2, 2);					// -(1/(PI*std2^2))(e-G.mul(X2Y2))
	LoG = LoG - (sum(LoG).val[0] / pow(filterSize, 2)); // Make the filter sum to zero

	return LoG;
}