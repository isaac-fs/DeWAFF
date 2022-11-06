#include "Tools.hpp"

/**
 * @brief Generate a meshgrid from X and Y unidimensional coordinates
 * 
 */
void Tools::meshGrid(const Range &xRange, const Range &yRange, Mat &X, Mat &Y){
	std::vector<int> xVector, yVector;
	for (int i = xRange.start; i <= xRange.end; i++) xVector.push_back(i);
	for (int i = yRange.start; i <= yRange.end; i++) yVector.push_back(i);

	repeat(Mat(xVector).reshape(1,1), yRange.size()+1, 1, X);
	repeat(Mat(yVector).reshape(1,1).t(), 1, xRange.size()+1, Y);
}

/**
 * @brief Gets the global min and max values of a 3 channel Mat
 * 
 * @param A 
 * @param minA 
 * @param maxA 
 */
void Tools::getMinMax(const Mat& A, double* minA, double* maxA){
	double minT, maxT;
	std::vector<Mat> channels(3);
	split(A,channels);
	minMaxLoc(channels[0],minA,maxA);
	minMaxLoc(channels[1],&minT,&maxT);
	*minA = std::min(*minA,minT);
	*maxA = std::max(*maxA,maxT);
	minMaxLoc(channels[2],&minT,&maxT);
	*minA = std::min(*minA,minT);
	*maxA = std::max(*maxA,maxT);
}
