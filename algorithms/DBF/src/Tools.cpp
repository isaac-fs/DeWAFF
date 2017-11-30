/*
 * tools.cpp
 *
 *  Created on: Nov 5, 2015
 *      Author: davidp
 */

#include "Tools.hpp"

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
