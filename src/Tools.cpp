#include "Tools.hpp"

/**
 * @brief Generate a meshgrid from X and Y unidimensional coordinates
 * Example:
 * xRange = [0,3[ and yRange = [0,3[ will return the following X and Y coordinates
 * 
 * X = \n
 * 		[0, 0, 0;	\n
 *		 1, 1, 1;	\n
 *		 2, 2, 2]	\n
 * 
 * Y = \n
 * 		[0, 1, 2;	\n
 *		 0, 1, 2;	\n
 *		 0, 1, 2]	\n
 * 					\n
 * Wich would form the following mesh grid
 * 
 * (X,Y) = \n
 * 		[(0,0) (0,1), (0,2); \n
 * 		 (0,1) (1,1), (2,1); \n
 * 		 (2,0) (2,1), (2,2)]
 * 
 * @param xRange X OpenCV Range variable
 * @param yRange Y OpenCV Range variable
 * @param X x axis values matrix
 * @param Y y axis values matrix
 */
void Tools::meshGrid(const Range &range, Mat &X, Mat &Y){
	std::vector<int> x;
	for (int i = range.start; i < range.end; i++) x.push_back(i);
	repeat(Mat(x).reshape(1,1), range.size(), 1, X);
	Y = X.t();
}

/**
 * @brief Gets the global min and max values of a 3 channel Matrix
 * 
 * @param A Input matrix
 * @param minA Minimun value of the matrix A
 * @param maxA Maximun value of the matrix A
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

/**
 * @brief Adds padding to a region of interest or local region. 
 * Lets say you have a [5x5] kernel, but your RoI to convolute is smaller be it [5x3], [3x5] or [3x3].
 * This function allows to add padding to the RoI so it fits the convolution. The padding is calculated
 * with the image that the RoI belongs to
 * 
 * @param RoI Region of interest to add padding to 
 * @param xRangeSize x range size to fit
 * @param yRangeSize y range size to fit
 */
void Tools::addPadding(Mat &RoI, int xRangeSize, int yRangeSize) {
    if (xRangeSize == yRangeSize)
        copyMakeBorder(RoI, RoI, 0, xRangeSize - RoI.rows, 0, xRangeSize - RoI.cols, BORDER_REPLICATE);
    else if(xRangeSize < yRangeSize)
        copyMakeBorder(RoI, RoI, 0, yRangeSize - xRangeSize, 0, 0, BORDER_REPLICATE);
    else
        copyMakeBorder(RoI, RoI, 0, 0, 0, xRangeSize - yRangeSize, BORDER_REPLICATE);

}