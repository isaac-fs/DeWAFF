/*
 * noAdaptiveLaplacian.cpp
 *
 *  Created on: Oct 28, 2015
 *      Author: davidp
 */

#include "laplacian.hpp"

//Regular non adaptive unsharped masking with laplacian filter
Mat Laplacian::noAdaptive(const Mat& A, const Mat& mask, const int lambda){
	Point anchor( -1 , -1 );
	double delta = 0;
	int ddepth = -1;

	Mat Z(A.size(),A.type());

	filter2D(A, Z, ddepth , mask, anchor, delta, BORDER_DEFAULT );

	//normalization
	double minZ,maxZ;
	Tools::minMax(abs(Z),&minZ,&maxZ);

	double minA,maxA;
	Tools::minMax(A,&minA,&maxA);

	Z = maxA * (Z / maxZ);

	//Unsharp masking
	return (A + lambda * Z);
}

//Creates a Laplacian of Gaussian mask, same as fspecial('log',..) in Matlab
Mat Laplacian::logKernel(int size, double sigma){
	Mat1f x, y, xy, h, h2;

	// first calculate Gaussian
	int siz = (size-1)/2;
	double eps = pow(2,-52);
	double minH, maxH;
	double std2 = pow(sigma,2);

	Tools::meshgrid(Range(-siz,siz),Range(-siz,siz),x,y);
	pow(x,2,x);
	pow(y,2,y);
	xy = x + y;

	exp(xy/(-2*std2),h);
	Tools::minMax(h,&minH,&maxH);
	threshold(h,h,eps*maxH,1.0,THRESH_TOZERO);

	Scalar sumH = sum(h);
	if(sumH.val[0] != 0)
		h  /= sumH.val[0];

	// now calculate Laplacian
	xy.convertTo(h2,CV_32F,1,-2*std2);
	h2 /= pow(std2,2);
	h2 = h2.mul(h);
	h  = h2 - sum(h2).val[0]/pow(size,2); // make the filter sum to zero

	return h;
}
