#include "Filters.hpp"

/**
 * @brief Apply a Bilateral Filter to an image. This is the decoupled version of this filter, this means
 * that the weighting image for the filter can be different from the input image
 *
 * @param weightingImage image used to calculate the kernel's weight
 * @param inputImage image used as input for the filter
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param spatialSigma spatial standard deviation
 * @param rangeSigma range or radiometric standard deviation
 * @return Mat output image
 */
Mat Filters::BilateralFilter(const Mat &weightingImage, const Mat &inputImage, const int windowSize, const double spatialSigma, const int rangeSigma) {
    // Set the padding value
    int padding = (windowSize - 1) / 2;

    // Working images
    Mat input, weight;

    // Add padding to the input for kernel consistency
    copyMakeBorder(inputImage, input, padding, padding, padding, padding, BORDER_CONSTANT);
    copyMakeBorder(weightingImage, weight, padding, padding, padding, padding, BORDER_CONSTANT);

    // Pre computation of meshgrid values
    Mat1f X, Y, S;
    Range range = Range(-(windowSize/2), (windowSize/2) + 1);
    lib.MeshGrid(range, X, Y);

    pow(X, 2, X);
    pow(Y, 2, Y);
    S = X + Y;

    /**
    * This filter uses two Gaussian kernels, one of them is the spatial Gaussian kernel
    * \f$ G_{\text spatial}(U, m, p) = \exp\left(-\frac{ ||m - p||^2 }{ 2 {\sigma_s^2} } \right) \f$
    * with the spatial values from an image region \f$ \Omega \subseteq U \f$.
    * The spatial kernel uses the \f$ m_i \subseteq \Omega \f$ pixels coordinates as weighting values for the pixel \f$ p = (x, y) \f$
    */
    Mat spatialGaussian = lib.GaussianFunction(S, spatialSigma);

    // Variance calculation
    int rangeVariance = std::pow(rangeSigma, 2);

    // Prepare variables for the bilateral filtering
    Mat output(input.size(), input.type());
    Mat weightRegion, inputRegion;
    Mat dL, da, db, bilateralFilterKernel, rangeGaussian;
	double bilateralFilterKernelNorm;
    int iMin, iMax, jMin, jMax;
    Range xRange, yRange;
	Vec3f pixel;
    Mat weightChannels[3], inputChannels[3];

    // Set the parallelization pragma for OpenMP
    #pragma omp parallel for\
    private(iMin, iMax, jMin, jMax, xRange, yRange,\
            weightRegion, weightChannels, inputRegion, inputChannels,\
            pixel, dL, da, db, rangeGaussian,\
            bilateralFilterKernel, bilateralFilterKernelNorm)\
	shared( input, weight, output,\
            windowSize, spatialSigma, rangeSigma)
    for(int i = padding; i < input.rows - padding; i++) {
        iMin = i - padding;
        iMax = iMin + windowSize;
        xRange = Range(iMin, iMax);

        for(int j = padding; j < input.cols - padding; j++) {
            jMin = j -  padding;
            jMax = jMin + windowSize;
            yRange = Range(jMin, jMax);

            // Extract local weightRegion based on the window size
            weightRegion = weight(xRange, yRange);
            cv::split(weightRegion, weightChannels);

            /**
            * The other Gaussian kernel is the range Gaussian kernel \f$ G_{\text range}(U, m, p) = \exp\left( -\frac{ ||U(m) - U(p)||^2 }{ 2{\sigma_s^2} } \right) \f$
            * with the intensity (range) values from an image region \f$ \Omega \subseteq U \f$.
            * The range kernel uses the \f$ m_i \subseteq \Omega \f$ pixels intensities as weighting values for the pixel \f$ p = (x, y) \f$ instead of their
            * locations as in the spatial kernel computation. In this case a the input \f$ U \f$ is separated into the three CIELab weightChannels and each
            * channel is processed as an individual input \f$ U_{\text channel} \f$
            */
            pixel = weight.at<Vec3f>(i, j);
            cv::pow(weightChannels[L] - pixel.val[L], 2, dL);
            cv::pow(weightChannels[a] - pixel.val[a], 2, da);
            cv::pow(weightChannels[b] - pixel.val[b], 2, db);
            cv::exp((dL + da + db) / (-2 * rangeVariance), rangeGaussian);

            /**
            * The two kernels convolve to obtain the Bilateral Filter kernel
            * \f$ \phi_{\text SBF}(U, m, p) = G_{\text spatial}(||m-p||) \, G_{\text range}(||U(m)-U(p)||) \f$
            *
            */
            bilateralFilterKernel = spatialGaussian.mul(rangeGaussian);

            /**
            * The Bilateral filter's norm corresponds to
            * \f$ \left( \sum_{m \subseteq \Omega} \phi_{\text{SBF}}(U, m, p) \right)^{-1} \f$
            */
            bilateralFilterKernelNorm = sum(bilateralFilterKernel).val[0];

            /**
            * Finally the bilateral filter kernel can be convolved with the input as follows
            * \f$ Y_{\phi_{\text SBF}}(p) = \left( \sum_{m \subseteq \Omega} \phi_{\text SBF}(U, m, p) \right)^{-1}
            * \left( \sum_{m \subseteq \Omega} \phi_{\text SBF}(U, p, m) \, \hat{f}_{\text USM}(m) \right) \f$
            */
            inputRegion = input(xRange, yRange);
            cv::split(inputRegion, inputChannels);
            output.at<Vec3f>(i,j)[L] = (1/bilateralFilterKernelNorm) * sum(bilateralFilterKernel.mul(inputChannels[L])).val[0];
            output.at<Vec3f>(i,j)[a] = (1/bilateralFilterKernelNorm) * sum(bilateralFilterKernel.mul(inputChannels[a])).val[0];
            output.at<Vec3f>(i,j)[b] = (1/bilateralFilterKernelNorm) * sum(bilateralFilterKernel.mul(inputChannels[b])).val[0];
        }
    }

    // Discard the padding
    xRange = Range(padding, padding + inputImage.rows);
    yRange = Range(padding, padding + inputImage.cols);

    return output(xRange, yRange);
}

/**
 * @brief Apply a Scaled Bilateral Filter to an image. This is the decoupled version of this filter, this means
 * that the weighting image for the filter can be different from the input image. The difference between this filter and the
 * not scaled version is that the weighting image is pre scaled through a Gaussian low pass filter to have better performance
 * under heavy AWGN
 *
 * @param weightingImage image used to calculate the kernel's weight
 * @param inputImage image used as input for the filter
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param spatialSigma spatial standard deviation
 * @param rangeSigma range or radiometric standard deviation
 * @return Mat output image
 */
Mat Filters::ScaledBilateralFilter(const Mat &weightingImage, const Mat &inputImage, const int windowSize, const double spatialSigma, const int rangeSigma) {
     // Set the padding value
    int padding = (windowSize - 1) / 2;

    // Working images
    Mat input, weight;

    // Add padding to the input for kernel consistency
    copyMakeBorder(inputImage, input, padding, padding, padding, padding, BORDER_CONSTANT);
    copyMakeBorder(weightingImage, weight, padding, padding, padding, padding, BORDER_CONSTANT);

    // Pre computation of meshgrid values
    Mat1f X, Y, S;
    Range range = Range(-(windowSize/2), (windowSize/2) + 1);
    lib.MeshGrid(range, X, Y);

    pow(X, 2, X);
    pow(Y, 2, Y);
    S = X + Y;

    /**
    * This filter uses two Gaussian kernels, one of them is the spatial Gaussian kernel
    * \f$ G_{\text spatial}(U, m, p) = \exp\left(-\frac{ ||m - p||^2 }{ 2 {\sigma_s^2} } \right) \f$
    * with the spatial values from an image region \f$ \Omega \subseteq U \f$.
    * The spatial kernel uses the \f$ m_i \subseteq \Omega \f$ pixels coordinates as weighting values for the pixel \f$ p = (x, y) \f$
    */
    Mat spatialGaussian = lib.GaussianFunction(S, spatialSigma);

    // Variance calculation
    int rangeVariance = std::pow(rangeSigma, 2);

    /* This filter uses a low pass filtered version of the input image as part of the weighting input.
    *  In this case with a Gaussian blur as LPF
    */
    Mat scaled(weight.size(), weight.type());
    GaussianBlur(weight, scaled, Size(windowSize, windowSize), rangeSigma);

    // Prepare variables for the bilateral filtering
    Mat output(input.size(), input.type());
    Mat weightRegion, inputRegion, scaledRegion;
    Mat dL, da, db;
    Mat scaledBilateralKernel, rangeGaussian;
	double scaledBilateralKernelNorm;
    int iMin, iMax, jMin, jMax;
    Range xRange, yRange;
	Vec3f pixel;
    Mat scaledChannels[3], inputChannels[3];

    // Set the parallelization pragma for OpenMP
    #pragma omp parallel for\
    private(iMin, iMax, jMin, jMax, xRange, yRange,\
            weightRegion, inputRegion, inputChannels, scaledRegion, scaledChannels,\
            pixel, dL, da, db,\
            rangeGaussian, scaledBilateralKernel, scaledBilateralKernelNorm)\
	shared( input, weight, scaled, output,\
            windowSize, spatialSigma, rangeSigma)
    for(int i = padding; i < input.rows - padding; i++) {
        iMin = i - padding;
        iMax = iMin + windowSize;
        xRange = Range(iMin, iMax);

        for(int j = padding; j < input.cols - padding; j++) {
            jMin = j - padding;
            jMax = jMin + windowSize;
            yRange = Range(jMin, jMax);

            // Extract local region based on the window size
            scaledRegion = scaled(xRange, yRange);
            cv::split(scaledRegion, scaledChannels);

            /**
            * The other Gaussian kernel is the range Gaussian kernel \f$ G_{\text range}(U, U^s, m, p) = \exp\left( -\frac{ ||U^s(m) - U(p)||^2 }{ 2{\sigma_s^2} } \right) \f$
            * with the intensity (range) values from an image region \f$ \Omega \subseteq U \f$.
            * The range kernel uses the \f$ m_i \subseteq \Omega \f$ pixels intensities as weighting values for the pixel \f$ p = (x, y) \f$ instead of their
            * locations as in the spatial kernel computation. In this case a the image \f$ U \f$ is separated into the three CIELab scaledChannels and each
            * channel is processed as an individual image \f$ U_{\text channel} \f$
            */
            pixel = weight.at<Vec3f>(i, j);
            pow(scaledChannels[L] - pixel.val[L], 2, dL);
            pow(scaledChannels[a] - pixel.val[a], 2, da);
            pow(scaledChannels[b] - pixel.val[b], 2, db);
            exp((dL + da + db) / (-2 * rangeVariance), rangeGaussian);

            /**
            * The two kernels convolve to obtain the Scaled Bilateral Filter kernel
            * \f$ \phi_{\text SBF}(U, U^s, m, p) = G_{\text spatial}(||m-p||) \, G_{\text range}(||U^s(m)-U(p)||) \f$
            *
            */
            scaledBilateralKernel = spatialGaussian.mul(rangeGaussian);

            /**
            * The Scaled Bilateral filter's norm corresponds to
            * \f$ \left( \sum_{m \subseteq \Omega} \phi_{\text{SBF}}(U, U^s, m, p) \right)^{-1} \f$
            */
            scaledBilateralKernelNorm = sum(scaledBilateralKernel).val[0];

            /**
            * Finally the bilateral filter kernel can be convolved with the input as follows
            * \f$ Y_{\phi_{\text SBF}}(p) = \left( \sum_{m \subseteq \Omega} \phi_{\text SBF}(U, U^s, m, p) \right)^{-1}
            * \left( \sum_{m \subseteq \Omega} \phi_{\text SBF}(U, p, m) \, \hat{f}_{\text USM}(m) \right) \f$
            */
            inputRegion = input(xRange, yRange);
            cv::split(inputRegion, inputChannels);
            output.at<Vec3f>(i,j)[L] = (1/scaledBilateralKernelNorm) * sum(scaledBilateralKernel.mul(inputChannels[L])).val[0];
            output.at<Vec3f>(i,j)[a] = (1/scaledBilateralKernelNorm) * sum(scaledBilateralKernel.mul(inputChannels[a])).val[0];
            output.at<Vec3f>(i,j)[b] = (1/scaledBilateralKernelNorm) * sum(scaledBilateralKernel.mul(inputChannels[b])).val[0];
        }
    }

    // Discard the padding
    xRange = Range(padding, padding + inputImage.rows);
    yRange = Range(padding, padding + inputImage.cols);

    return output(xRange, yRange);
}

/**
 * @brief Apply a Non Local Means Filter to an image. This is the decoupled version of this filter, this means
 * that the weighting image for the filter can be different from the input image. The algorithm used for this filter
 * is very computationally demanding
 *
 * @param weightingImage image used to calculate the kernel's weight
 * @param inputImage image used as input for the filter
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param rangeSigma range or radiometric standard deviation. Used to calculate the parameter \f$ h = \frac{\sqrt(\sigma)}{2} \f$
 * @return Mat output image
 */
Mat Filters::NonLocalMeansFilter(const Mat &weightingImage, const Mat &inputImage, const int windowSize, const int patchSize, const double rangeSigma) {
     // Set the padding value
    int padding = (windowSize - 1) / 2;

    // Working images
    Mat input, weight;

    // Add padding to the input for kernel consistency
    copyMakeBorder(inputImage, input, padding, padding, padding, padding, BORDER_CONSTANT);
    copyMakeBorder(weightingImage, weight, padding, padding, padding, padding, BORDER_CONSTANT);

    // NML standard deviation h
    double h = sqrt(rangeSigma)/2;

    // Prepare variables for the bilateral filtering
    Mat output(input.size(), input.type());
    Mat inputRegion, weightRegion;
    Mat dL, da, db;
    Mat nonLocalMeansKernel;
	double nonLocalMeansKernelNorm;
    Range xRange, yRange;
	Vec3f pixel;
    Mat inputChannels[3], weightChannels[3], nlmChannels[3];
    int iMin, iMax, jMin, jMax;

    // Set the parallelization pragma for OpenMP
    #pragma omp parallel for\
    private(iMin, iMax, jMin, jMax, xRange, yRange,\
            pixel, dL, da, db,\
            nlmChannels, nonLocalMeansKernel, nonLocalMeansKernelNorm,\
            weightRegion, weightChannels, inputRegion, inputChannels)\
	shared( input, weight, output,\
            windowSize, patchSize, rangeSigma)
    for(int i = padding; i < input.rows - padding; i++) {
        iMin = i - padding;
        iMax = iMin + windowSize;
        xRange = Range(iMin, iMax);

        for(int j = padding; j < input.cols - padding; j++) {
            jMin = j - padding;
            jMax = jMin + windowSize;
            yRange = Range(jMin, jMax);

            // Extract local region based on the window size
            weightRegion = weight(xRange, yRange);

            /**
            * The discrete representation of the Non Local Means Filter is as follows,
            * \f$ \phi_{\text {NLM}}(U, m, p) = \sum_{B(m) \subseteq U} \exp\left( \frac{||B(m) - B(p)||^2}{h^2} \right)\f$
            * where  \f$B(p)\f$ is a patch part of the window \f$\Omega\f$ centered at pixel \f$p\f$. \f$B(m)\f$ represents all of the
            * patches at \f$\Omega\f$ centered in each \f$m\f$ pixel. The Non Local Means Filter calculates the Euclidean distance
            * between  each patch \f$B(m)\f$ and \f$B(p)\f$ for each window \f$\Omega \subseteq U\f$. This is why this algorithm is
            * highly demanding in computational terms. Each Euclidean distance matrix obtained from each patch pair is the input for
            * a Gaussian decreasing function with standard deviation \f$h\f$ that generates the new pixel \f$p\f$ value
            */
            cv::split(weightRegion, weightChannels);
            nlmChannels[L] = lib.GaussianFunction(lib.EuclideanDistanceMatrix(weightChannels[L], patchSize), h);
            nlmChannels[a] = lib.GaussianFunction(lib.EuclideanDistanceMatrix(weightChannels[a], patchSize), h);
            nlmChannels[b] = lib.GaussianFunction(lib.EuclideanDistanceMatrix(weightChannels[b], patchSize), h);
            nonLocalMeansKernel = (nlmChannels[L] + nlmChannels[a] + nlmChannels[b]);

            /**
            * The Non Local Means filter's norm is calculated with
            * \f$ \phi_{\text NLM}(U, m, p) \left( \sum_{m \subseteq \Omega} \phi_{\text{NLM}}(U, m, p) \right)^{-1} \f$
            */
            nonLocalMeansKernelNorm = sum(nonLocalMeansKernel).val[0];

            /**
             * The NLM filter kernel is applied to the laplacian image
            * \f$ Y_{\phi_{\text NLM}}(p) = \left( \sum_{m \subseteq \Omega} \phi_{\text NLM}(U, m, p) \right)^{-1}
            * \left( \sum_{m \subseteq \Omega} \phi_{\text NLM}(U, p, m) \, \hat{f}_{\text USM}(m) \right) \f$
            */
            inputRegion = input(xRange, yRange);
            cv::split(inputRegion, inputChannels);
            output.at<Vec3f>(i,j)[L] = (1/nonLocalMeansKernelNorm) * sum(nonLocalMeansKernel.mul(inputChannels[L])).val[0];
            output.at<Vec3f>(i,j)[a] = (1/nonLocalMeansKernelNorm) * sum(nonLocalMeansKernel.mul(inputChannels[a])).val[0];
            output.at<Vec3f>(i,j)[b] = (1/nonLocalMeansKernelNorm) * sum(nonLocalMeansKernel.mul(inputChannels[b])).val[0];
        }
    }

    // Discard the padding
    xRange = Range(padding, padding + inputImage.rows);
    yRange = Range(padding, padding + inputImage.cols);

    return output(xRange, yRange);
}

/**
 * @brief Apply a Guided Filter to an image. This is the decoupled version of this filter, this means
 * that the weighting image for the filter can be different from the input image. In this case the weighting
 * image is known as guiding image. This filter uses a lineal algorithm, making it very fast computationally
 *
 * @param weightingImage image used to calculate the kernel's weight
 * @param inputImage image used as input for the filter
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param rangeSigma range or radiometric standard deviation. Used to calculate \f$ \epsilon = \sigma_r^2}
 * @return Mat output image
 */
Mat Filters::GuidedFilter(const Mat &inputImage, const Mat &guidingImage, const int windowSize, const int rangeSigma) {
    double epsilon = pow(rangeSigma, 2);
    /**
     * The Guided Filter follows the followin algorithm
     *
     */
    return guidedFilter(guidingImage, inputImage, windowSize, epsilon, -1);
}