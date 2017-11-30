/*
 * tools.cpp
 *
 *  Created on: Nov 5, 2015
 *      Author: davidp
 */

#include "tools.hpp"

//Creates a Laplacian of Gaussian mask, same as fspecial('log',..) in Matlab
Mat Tools::fspecialLoG(int size, double sigma){
	Mat1f x, y, xy, h, h2;

	// first calculate Gaussian
	int siz = (size-1)/2;
	double eps = pow(2,-52);
	double minH, maxH;
	double std2 = pow(sigma,2);

	meshgrid(Range(-siz,siz),Range(-siz,siz),x,y);
	pow(x,2,x);
	pow(y,2,y);
	xy = x + y;

	exp(xy/(-2*std2),h);
	minMax(h,&minH,&maxH);
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

void Tools::meshgrid(const Range &xgv, const Range &ygv, Mat &X, Mat &Y){
	vector<int> t_x, t_y;
	for (int i = xgv.start; i <= xgv.end; i++) t_x.push_back(i);
	for (int i = ygv.start; i <= ygv.end; i++) t_y.push_back(i);

	repeat(Mat(t_x).reshape(1,1), ygv.size()+1, 1, X);
	repeat(Mat(t_y).reshape(1,1).t(), 1, xgv.size()+1, Y);
}

//Gets the global min and max values of a 3 channel Mat
void Tools::minMax(const Mat& A, double* minA, double* maxA){
	double minT, maxT;
	vector<Mat> channels(3);
	split(A,channels);
	minMaxLoc(channels[0],minA,maxA);
	minMaxLoc(channels[1],&minT,&maxT);
	*minA = min(*minA,minT);
	*maxA = max(*maxA,maxT);
	minMaxLoc(channels[2],&minT,&maxT);
	*minA = min(*minA,minT);
	*maxA = max(*maxA,maxT);
}

void Tools::showImg(const Mat& A){
	namedWindow( "Display window", WINDOW_AUTOSIZE );
	imshow( "Display window", A );                   // Show our image inside it.
	waitKey(0);
}
