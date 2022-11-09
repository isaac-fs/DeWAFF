#include "Tools.hpp"

/**
 * @brief Generate a meshgrid from X and Y unidimensional coordinates
 * Example:
 * xRange = [0,1,2] and yRange = [0,1,2] will return the following X and Y coordinates
 * 
 * X = \n
 * 		[0, 1, 2; \n
 * 		 0, 1, 2; \n
 * 		 0, 1, 2] \n
 * 
 * Y = \n
 * 		[0, 0, 0; \n
 * 		 1, 1, 1; \n
 * 		 2, 2, 2] \n
 * 
 * Wich forms the mesh grid
 * 
 * XY = \n
 * 		[(0,0) (1,0), (2,0); \n
 * 		 (0,1) (1,1), (2,1); \n
 * 		 (0,2) (1,2), (2,2)]
 */
void Tools::meshGrid(const Range &xRange, const Range &yRange, Mat &X, Mat &Y){
	std::vector<int> xVector, yVector;
	for (int i = xRange.start; i <= xRange.end; i++)
		xVector.push_back(i);
	for (int i = yRange.start; i <= yRange.end; i++)
		yVector.push_back(i);

	repeat(Mat(xVector).reshape(1,1), yRange.size()+1, 1, X);
	repeat(Mat(yVector).reshape(1,1).t(), 1, xRange.size()+1, Y);
}

/**
 * @brief Gets the global min and max values of a 3 channel Mat
 * 
 * @param A Input matrix
 * @param minA Minimun value of the A matrix
 * @param maxA Maximun value of the A matrix
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
