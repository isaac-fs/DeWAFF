#include "NonAdaptiveUSM.hpp"

/**
 * @file NonAdaptiveUSM.cpp
 * @brief
 * @author David Prado{davidp}
 * @date 10/28/2015
 * @author Isaac Fonseca Segura {isaac-fs}
 * @date 10/28/2022
 */

/**
 * @brief Creates a Laplacian of Gaussia  kernel same as fspecial('log',..) in Matlab
 * 
 */
Mat NonAdaptiveUSM::LoGkernel(int filterSize, double sigma){
	Mat_<float> X, Y, X2, Y2, X2Y2, e, G, LoG;

	// First calculate the Gaussian kernel
	int size = (filterSize-1)/2;
	double eps = pow(2,-52);
	double std2 = pow(sigma,2);
	Tools::meshGrid(Range(-size,size),Range(-size,size),X,Y);
	pow(X,2,X2); // X^2
	pow(Y,2,Y2); // Y^2
	X2Y2 = X2 + Y2; // X^2 + Y^2
	exp(X2Y2/(-2*std2),e); // e = e^(-(X^2 + Y^2)/(2*std2))
	e.convertTo(G,CV_32F,1,1/(2*CV_PI*std2)); // G = (1/(2*PI*std2))*e^(-(X^2 + Y^2)/(2*std2)

	double minG, maxG;
	Tools::getMinMax(G,&minG,&maxG);
	threshold(G,G,eps*maxG,1.0,THRESH_TOZERO);

	Scalar sumG = sum(G);
	if(sumG.val[0] != 0)
		G  /= sumG.val[0];

	// Calculate the Laplacian of the Gaussian kernel
	LoG = e-G.mul(X2Y2);
	LoG /= -1 * CV_PI * pow(std2,2); // -(1/(PI*std2^2))(e-G.mul(X2Y2))
	LoG  = (LoG - sum(LoG).val[0]/pow(filterSize,2)); // Make the filter sum to zero

	return LoG;
}

//Regular non adaptive unsharped masking with laplacian filter
Mat NonAdaptiveUSM::nonAdaptiveUSM(const Mat& inputImage, const int lambda){
	Mat kernel = -1 * NonAdaptiveUSM::LoGkernel(17, 0.005);

	Point anchor(-1,-1);
	double delta = 0.0;
	int ddepth = -1;

	Mat LoGImage(inputImage.size(),inputImage.type());

	filter2D(inputImage, LoGImage, ddepth, kernel, anchor, delta, BORDER_DEFAULT);

	//normalization
	double minZ,maxZ;
	Tools::getMinMax(abs(LoGImage),&minZ,&maxZ);

	double minA,maxA;
	Tools::getMinMax(inputImage,&minA,&maxA);

	LoGImage = maxA * (LoGImage / maxZ);

	//Unsharp masking
	return (inputImage + lambda * LoGImage);
}
