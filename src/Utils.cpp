#include "Utils.hpp"

/**
 * @brief Generates a meshgrid from \f$X\f$ and \f$Y\f$ unidimensional coordinates.
 * Example:
 * xRange = [0,3[ and yRange = [0,3[ will return the following \f$X\f$ and \f$Y\f$ coordinates:
 *
 * 	X =			Y =
 * 	[0, 0, 0;	[0, 1, 2;
 *		 1, 1, 1;	 0, 1, 2;
 *		 2, 2, 2]	 0, 1, 2]
 *
 * Wich would form the following mesh grid:
 *
 * 	(X,Y) =
 * 		[(0,0) (0,1), (0,2);
 * 		 (0,1) (1,1), (2,1);
 * 		 (2,0) (2,1), (2,2)]
 *
 * Although the values of \f$X\f$ and \f$Y\f$ can be manipulated to form useful patterns.
 *
 * @param range range to form the 2D grid
 * @param X x axis values for the mesh grid
 * @param Y y axis values for the mesh grid
 */
void Utils::MeshGrid(const Range &range, Mat &X, Mat &Y) {
	std::vector<int> x;
	for (int i = range.start; i < range.end; i++) x.push_back(i);
	repeat(Mat(x).reshape(1,1), range.size(), 1, X);
	Y = X.t();
	X.convertTo(X, CV_32F);
	Y.convertTo(Y, CV_32F);
}

/**
 * @brief Gets the global min and max values of a 3 channel Matrix.
 *
 * @param A Input matrix
 * @param minA Minimun value of the matrix A
 * @param maxA Maximun value of the matrix A
 */
void Utils::MinMax(const Mat& A, double* minA, double* maxA) {
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
 * @brief Computes the Gaussian function of an input \f$ X \f$
 * \f[ G(X) = \frac{1}{\sigma\sqrt{2\pi}} \exp\left( -\frac{X}{2\sigma^2} \right) \f]
 * @param input Matrix input
 * @param sigma Desired standard deviation
 * @return Mat
 */
Mat Utils::GaussianFunction(Mat input, double sigma){
	Mat output;
	double variance = pow(sigma, 2.0);
	exp(input * (-1.0 / (2.0 * variance)), output);
	output *= 1.0/(sqrt(2.0 * CV_PI) * sigma);
	return output;
}

/**
 * @brief Computes a normalized Gaussian kernel
 * \f[ G(X, Y) =  \frac{1}{2\pi \sigma^2} \exp\left( -\frac{ X^2 + Y^2 }{ 2 \sigma_s^2} \right) \f]
 * where \f$X\f$ and \f$Y\f$ are the horizontal and vertical coordinates on a windowSize\f$ \times \f$windowSize 2D plane.
 * @param windowSize size of the window for the kernel
 * @param sigma standard deviation for the Gaussian distribution
 * @return Mat Gaussian kernel
 */
Mat Utils::GaussianKernel(int windowSize, double sigma) {
	// Pre computation of meshgrid values
	Mat1f X, Y;
	Range range = Range(-(windowSize/2), (windowSize/2) + 1);
	MeshGrid(range, X, Y);
	pow(X, 2.0, X);
	pow(Y, 2.0, Y);

	// Compute the Gaussian kernel
	Mat gaussianKernel = GaussianFunction(X, sigma).mul(GaussianFunction(Y, sigma));

	// Normalization
	gaussianKernel /= sum(gaussianKernel).val[0];

	return gaussianKernel;
}

/**
 * @brief Filters an image through a Laplacian of Gaussian filter
 * \f[ \text{LoG}(X,Y) = \frac{1}{2 \pi \sigma^2} \exp\left(-\frac{X^2 + Y^2}{2 \sigma^2}\right) \left( \frac{X^2 + Y^2}{\sigma^2} - 2 \right) \f]
 */
Mat Utils::LoGFilter(const Mat &image, int windowSize, double sigma) {
	// Get the Gaussian kernel
	Mat gaussianKernel = GaussianKernel(windowSize, sigma);

	// Pre computation of meshgrid values
	Mat X, Y, S;
	Range range = Range(-(windowSize/2), (windowSize/2) + 1);
	MeshGrid(range, X, Y);
	pow(X, 2.0, X);
	pow(Y, 2.0, Y);

	// Variance
	double variance = pow(sigma, 2.0);
	Mat laplacianOfGaussianKernel = (1.0 / (2.0 * CV_PI * variance)) * (((X+Y)/variance) - 2.0).mul(gaussianKernel);

	// Normalization
	laplacianOfGaussianKernel -= sum(laplacianOfGaussianKernel).val[0] / pow(windowSize, 2.0);

	// Create a new image with the size and type of the input image
	Mat LoGFilteredImage(image.size(), image.type());

	// Apply the Laplacian filter
	filter2D(image, LoGFilteredImage, -1, laplacianOfGaussianKernel, Point(-1,-1), 0, BORDER_CONSTANT);

	return LoGFilteredImage;
}

/**
 * @brief Applies a regular non adaptive UnSharp mask (USM) filter with a Laplacian of Gaussian filter
 * \f[ \hat{f}_{\text USM} = U + \lambda \ \text{LoG} \text{ where } \text{LoG} = l * g \f]
 * @param image Input image to filter
 * @param windowSize Size of the filter
 * @param lambda constant for the Laplacian deceive
 * @param sigma standard distribution
 * @return Filtered image
 */
Mat Utils::NonAdaptiveUSMFilter(const Mat &image, int windowSize, double lambda, double sigma) {
	// Generate the Laplacian kernel
	Mat LoGFilteredImage = LoGFilter(image, windowSize, sigma);

	// Normalize the Laplacian filtered image
	double minL, maxL, minI, maxI;
	Utils::MinMax(abs(LoGFilteredImage), &minL, &maxL);
	Utils::MinMax(image, &minI, &maxI);
	LoGFilteredImage = maxI * (LoGFilteredImage / maxL);

	// Return the filtered image
	return (image - lambda * LoGFilteredImage);
}

/**
 * @brief Computes the Euclidean distance between a fixed patch at the center of the input
 * image and a patch centered in every other pixel in the input image, mathematically, for
 * an input matrix \f$A = (a_{ij})\f$ every element will take the corresponding Euclidean distance value
 * \f$a_{ij} = d_{ij}^{2} = || x_{i}-x_{j} ||^{2}\f$
 * @param inputImage_ input image
 * @param neighborhoodSize size of the pixel neighborhood
 * @return Mat
 */
Mat Utils::EuclideanDistancesMatrix(const Mat& inputImage_, int windowSize, int neighborhoodSize) {
	// Fixed pixel sub region (must be a square region)
	Mat inputImage;
	int padding = (windowSize-1)/2;
	copyMakeBorder(inputImage_, inputImage, padding, padding, padding, padding, BORDER_REPLICATE);

	// Ranges
	Range xRange, yRange;
	xRange = Range(windowSize/2, windowSize/2 + neighborhoodSize);
	yRange = Range(windowSize/2, windowSize/2 + neighborhoodSize);

	// Fixed patch at the center of the image
	Mat fixedWindow = inputImage(xRange, yRange);

	// Moving pixel sub region
	Mat slidingWindow(fixedWindow.size(), fixedWindow.type());

	// Initialize the output matrix
	Mat euclideanDistancesMatrix(inputImage_.size(), inputImage_.type());

	// Visit each pixel in the image region
	for(int i = 0; i < windowSize; i++) {
		xRange = Range(i, i + neighborhoodSize);
		for(int j = 0; j < windowSize; j++) {
			yRange = Range(j, j + neighborhoodSize);

			//Extract pixel w neighborhood local region
			slidingWindow = inputImage(xRange, yRange);

			// Calculate the euclidean distance between patches
			euclideanDistancesMatrix.at<float>(i, j) = (float) norm(fixedWindow, slidingWindow, NORM_L2SQR) / ((float)neighborhoodSize);
        }
    }
    return euclideanDistancesMatrix;
}