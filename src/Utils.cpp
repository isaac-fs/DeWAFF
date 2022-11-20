#include "Utils.hpp"

/**
 * @brief Generates a meshgrid from \f$X\f$ and \f$Y\f$ unidimensional coordinates
 * Example:
 * xRange = [0,3[ and yRange = [0,3[ will return the following \f$X\f$ and \f$Y\f$ coordinates
 *
 * \f$X\f$ = \n
 * 		[0, 0, 0;	\n
 *		 1, 1, 1;	\n
 *		 2, 2, 2]	\n
 *
 * \f$Y\f$ = \n
 * 		[0, 1, 2;	\n
 *		 0, 1, 2;	\n
 *		 0, 1, 2]	\n
 * 					\n
 * Wich would form the following mesh grid
 *
 * \f$(X,Y)\f$ = \n
 * 		[(0,0) (0,1), (0,2); \n
 * 		 (0,1) (1,1), (2,1); \n
 * 		 (2,0) (2,1), (2,2)]
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
}

/**
 * @brief Gets the global min and max values of a 3 channel Matrix
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

Mat Utils::GaussianFunction(Mat input, double sigma){
	Mat output;
	double variance = pow(sigma, 2);

	exp(input * (-1 / (2 * variance)), output);

	return output;
}

/**
 * @brief Computes a spatial Gaussian kernel \f$ G(X, Y) = \exp\left(-\frac{|X + Y|^2 }{ 2 {\sigma_s^2} } \right) \f$
 * where X + Y are the horizontal and vertical coordinates on a \f$ \text{windowSize} \times \text{windowSize} \f$ 2D plane.
 * The result can be interpreted as looking at a Gaussian distribution from a top view
 *
 * @param windowSize 2D plane dimension
 * @param sigma standar deviation for the Gaussian distribution
 * @return Mat A Gaussian kernel
 */
Mat Utils::GaussianKernel(int windowSize, double sigma) {
	// Pre computation of meshgrid values
    Mat1f X, Y, S;
    Range range = Range(-(windowSize/2), (windowSize/2) + 1);
    MeshGrid(range, X, Y);

    /**
     * @brief Pre computates the spatial Gaussian kernel
     *  Calculate the kernel variable \f$ S = X^2 + Y^2 \f$
     */
    pow(X, 2, X);
    pow(Y, 2, Y);
    S = X + Y;

	double variance = pow(sigma, 2);

	Mat gaussianKernel = (1 / (2 * CV_PI * variance)) * GaussianFunction(S, sigma);
	gaussianKernel /= sum(gaussianKernel).val[0];

	return gaussianKernel;
}

/**
 * @brief Computes a Laplacian of Gaussian kernel. Same as fspecial('log',..) in Matlab.
 * \f$ \text{LoG}_{\text kernel} = - \frac{1}{\pi \sigma^4} \left[ 1 - \frac{X^2 + Y^2}{2 \sigma^2} \right] \exp\left(-\frac{X^2 + Y^2}{2 \sigma^2}\right) \f$
 * and normalize it with \f$ \frac{\sum \text{LoG} }{|\text{LoG}|}\f$ where \f$ |\text{LoG}| \f$ is the number of elements in \f$ \text{LoG} \f$ so it
 * sums to 0 for high pass filter behavior consistency
 */
Mat Utils::LoGKernel(int windowSize, double sigma) {
	// Pre computation of meshgrid values
    Mat1f X, Y, S;
    Range range = Range(-(windowSize/2), (windowSize/2) + 1);
    MeshGrid(range, X, Y);

    /**
     * @brief Pre computates the spatial Gaussian kernel
     *  Calculate the kernel variable \f$ S = X^2 + Y^2 \f$
     */
    pow(X, 2, X);
    pow(Y, 2, Y);
    S = X + Y;

	// Gaussian kernel
	Mat gaussianKernel = GaussianKernel(windowSize, sigma);

	// Variance
	double variance = pow(sigma, 2);

	// (-1 / (CV_PI * pow(variance, 2))) * (1 - (S / (2 * variance))).mul(gaussianKernel);
    Mat logKernel = (-2 / variance) * (1 - (S/(2*variance))) * gaussianKernel;
    double deltaFactor = sum(logKernel).val[0] / pow(windowSize, 2);
	logKernel = logKernel - deltaFactor;

	return logKernel;
}

/**
 * @brief Applies a regular non adaptive UnSharp mask (USM) with a Laplacian of Gaussian kernel
 * \f$ \hat{f}_{\text USM} = U + \lambda \mathcal{L} \text{ where } \mathcal{L} = l * g \f$.
 * Here \f$ g \f$ is a Gaussian kernel and \f$ l \f$ a Laplacian kernel, hence the name "Laplacian of Gaussian"
 * @param image Image to apply the mask
 * @return Filtered image
 */
Mat Utils::NonAdaptiveUSM(const Mat &image, int windowSize, int lambda, double sigma) {
	// Generate the Laplacian kernel
	Mat logKernel = -1 * LoGKernel(windowSize, sigma);

	// Create a new image with the size and type of the input image
	Mat LoG(image.size(), image.type());

	// Apply the Laplacian filter
	filter2D(image, LoG, -1, logKernel, Point(-1,-1), 0, BORDER_DEFAULT);

	// Normalize the Laplacian filtered image
	double minL, maxL, minI, maxI;
	Utils::MinMax(abs(LoG), &minL, &maxL);
	Utils::MinMax(image, &minI, &maxI);
	LoG = maxI * (LoG / maxL);

	return (image + lambda * LoG);
}

/**
 * @brief Computes an Euclidean distance matrix from an input matrix. This is achieved
 * by calculating the Euclidean distance between a fixed patch at the center of the input
 * image and a patch centered in every other pixel in the input image, mathematically, for
 * an input matrix \f$A = (a_{ij})\f$ every element will take the corresponding Euclidean distance value
 * \f$a_{ij} = d_{ij}^{2} = || x_{i}-x_{j} ||^{2}\f$
 * @param inputImage input image to obtain the patches to compute the matrix
 * @param patchSize size of the used patch. Analog to window size
 * @return Mat Euclidean distance matrix
 */
Mat Utils::EuclideanDistanceMatrix(const Mat& inputImage, int patchSize) {
    // Fixed pixel sub region (must be a square region)
	Mat image;
	int windowSize = inputImage.rows;
    int padding = (windowSize-1)/2;
	copyMakeBorder(inputImage, image, padding, padding, padding, padding, BORDER_CONSTANT);

	// Ranges
    Range xRange, yRange;
	xRange = Range(padding, padding + patchSize);
	yRange = Range(padding, padding + patchSize);

	// Fixed patch at the center of the image
    Mat fixedPatch = image(xRange, yRange);

	// Moving pixel sub region
    Mat movingPatch(fixedPatch.size(), fixedPatch.type());

    // Initialize the output matrix
    Mat euclideanDistanceMatrix(inputImage.size(), inputImage.type());

    // Visit each pixel in the image region
    for(int i = 0; i < windowSize; i++) {
        xRange = Range(i, i + patchSize);
        for(int j = 0; j < windowSize; j++) {
            yRange = Range(j, j + patchSize);

            //Extract pixel w neighborhood local region
            movingPatch = image(xRange, yRange);

            // Calculate the euclidean distance between patches
            euclideanDistanceMatrix.at<float>(i, j) = (float) norm(fixedPatch, movingPatch, NORM_L2);
        }
    }

    return euclideanDistanceMatrix;
}

/**
 * @brief Starts the timer and resets the elapsed time
 */
void Timer::start() {
	struct timeval timeOfDay;
	gettimeofday(&timeOfDay, nullptr);
	this->startTime = (double)timeOfDay.tv_sec + ((double)timeOfDay.tv_usec * 1.0e-6);
}

/**
 * @brief Stops the timer and returns the elapsed time
 * @return Elapsed time in seconds
 */
double Timer::stop() {
	struct timeval timeOfDay;
	gettimeofday(&timeOfDay, nullptr);
	return ((double)timeOfDay.tv_sec + ((double)timeOfDay.tv_usec * 1.0e-6)) - this->startTime;
}