/*
 * noAdaptiveLaplacian.cpp
 *
 *  Created on: Oct 28, 2015
 *      Author: davidp
 */

#include "noAdaptiveLaplacian.hpp"

void NoAdaptiveLaplacian::setMask(Mat theMask){
	theMask.copyTo(this->mask);
}

Mat NoAdaptiveLaplacian::noAdaptiveLaplacian(const Mat& U, int lambda){
	Mat U2;

	//Convert input image from default BGR to CIELab
	cvtColor(U,U2,CV_BGR2Lab);

	return filterUM_laplacianLAB(U2, lambda);
}

//Regular non adaptive unsharped masking with laplacian filter
Mat NoAdaptiveLaplacian::filterUM_laplacianLAB(const Mat& U, int lambda1){
	Mat Z = this->filterLaplacian2(U);

	//normalization
	double minZ,maxZ;
	Tools::minMax(abs(Z),&minZ,&maxZ);

	double minU,maxU;
	Tools::minMax(U,&minU,&maxU);

	Z = maxU * (Z / maxZ);

	//Unsharp masking
	return (U + lambda1 * Z);
}

//Applies a NxN laplacian mask
Mat NoAdaptiveLaplacian::filterLaplacian2(const Mat& U){
	Point anchor( -1 , -1 );
	double delta = 0;
	int ddepth = -1;

	Mat F(U.size(),U.type());

	filter2D(U, F, ddepth , this->mask, anchor, delta, BORDER_DEFAULT );
	return F;
}
