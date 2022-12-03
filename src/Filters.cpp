#include "Filters.hpp"

/**
 * @brief Apply a Bilateral Filter to an image. This is the decoupled version of this filter, this means
 * that the weighting image for the filter can be different from its input image.
 *
 * @param weightingImage_ image used to calculate the kernel weight
 * @param inputImage_ image used as input for the filter
 * @param windowSize processing window size
 * @param spatialSigma spatial standard deviation
 * @param rangeSigma range or radiometric standard deviation
 * @return Mat output image
 */
Mat Filters::BilateralFilter(const Mat &inputImage_, const Mat &weightingImage_, int windowSize, double spatialSigma, double rangeSigma)
{
    // Set the padding value
    int padding = (windowSize - 1) / 2;

    // Working images
    Mat inputImage, weightingImage;

    // Add padding to the input for kernel consistency
    copyMakeBorder(inputImage_, inputImage, padding, padding, padding, padding, BORDER_CONSTANT);
    copyMakeBorder(weightingImage_, weightingImage, padding, padding, padding, padding, BORDER_CONSTANT);

    // Pre compute the m - p = |m-p| factors
    Mat X, Y;
    Range range = Range((-windowSize / 2), (windowSize / 2) + 1);
    lib.MeshGrid(range, X, Y);
    pow(X, 2.0, X);
    pow(Y, 2.0, Y);
    Mat euclideanDistances = X + Y;

    /**
     * This filter uses two Gaussian kernels, one of them is the spatial Gaussian kernel:
     * \f[ G_{\text spatial}(U, m, p) = \exp\left(-\frac{ ||m - p||^2 }{ 2 {\sigma_s^2} } \right) \f]
     * with the spatial values from an image region \f$ \Omega \subseteq U \f$.
     * The spatial kernel uses the \f$ m_i \subset \Omega \f$ pixels coordinates as weighting values for the pixel \f$ p = (x, y) \f$.
     */
    Mat spatialGaussian = lib.GaussianFunction(euclideanDistances, spatialSigma);

    // Prepare variables for the bilateral filtering
    Mat outputImage(inputImage.size(), inputImage.type());
    Mat weightingRegion, inputRegion;
    Mat dL, da, db, bilateralFilter, rangeGaussian;
    double bilateralFilterNorm;
    int iMin, iMax, jMin, jMax;
    Range xRange, yRange;
    Vec3f pixel;
    Mat weightingChannels[3], inputChannels[3];

// Set the parallelization pragma for OpenMP
#pragma omp parallel for private(iMin, iMax, jMin, jMax, xRange, yRange,                         \
                                 weightingRegion, weightingChannels, inputRegion, inputChannels, \
                                 pixel, dL, da, db, rangeGaussian,                               \
                                 bilateralFilter, bilateralFilterNorm)                           \
    shared(inputImage, weightingImage, outputImage,                                              \
           windowSize, spatialSigma, rangeSigma)
    for (int i = padding; i < inputImage.rows - padding; i++)
    {
        iMin = i - padding;
        iMax = iMin + windowSize;
        xRange = Range(iMin, iMax);
        for (int j = padding; j < inputImage.cols - padding; j++)
        {
            jMin = j - padding;
            jMax = jMin + windowSize;
            yRange = Range(jMin, jMax);

            // Extract local weightRegion based on the window size
            weightingRegion = weightingImage(xRange, yRange);
            cv::split(weightingRegion, weightingChannels);

            /**
             * The other used kernel is the range Gaussian kernel:
             * \f[ G_{\text range}(U, m, p) = \exp\left( -\frac{ ||U(m) - U(p)||^2 }{ 2{\sigma_r^2} } \right) \f]
             * with the intensity (range) values from an image region \f$ \Omega \subseteq U \f$.
             * The range kernel uses the \f$ m_i \subset \Omega \f$ pixels intensities as weighting values for the pixel \f$ p = (x, y) \f$ instead of their
             * locations as in the spatial kernel computation. In this case a the input \f$ U \f$ is separated into the three CIELab weightChannels and each
             * channel is processed as an individual input \f$ U_{\text channel} \f$.
             */
            pixel = weightingImage.at<Vec3f>(i, j);
            cv::pow(weightingChannels[L] - pixel.val[L], 2.0, dL);
            cv::pow(weightingChannels[a] - pixel.val[a], 2.0, da);
            cv::pow(weightingChannels[b] - pixel.val[b], 2.0, db);
            rangeGaussian = lib.GaussianFunction(dL + da + db, rangeSigma);

            /**
             * The two kernels are multiplied to obtain the Bilateral Filter kernel:
             * \f[ \psi_{\text BF}(U, m, p) = G_{\text spatial}(U, m, p) \, G_{\text range}(U, m, p) \f]
             *
             */
            ;
            bilateralFilter = spatialGaussian.mul(rangeGaussian);

            /**
             * The Bilateral filter's norm corresponds to:
             * \f[ \left( \sum_{m \subset \Omega} \psi_{\text{BF}}(U, m, p) \right)^{-1} \f]
             */
            bilateralFilterNorm = sum(bilateralFilter).val[0];

            /**
             * Finally the bilateral filter kernel can be convolved with the input as follows:
             * \f[ Y_{\psi_{\text BF}}(p) = \left( \sum_{m \subset \Omega} \psi_{\text BF}(U, m, p) \right)^{-1}
             * \left( \sum_{m \subset \Omega} \psi_{\text BF}(U, p, m) \, U(m) \right) \f]
             */
            inputRegion = inputImage(xRange, yRange);
            cv::split(inputRegion, inputChannels);
            outputImage.at<Vec3f>(i, j)[L] = (1 / bilateralFilterNorm) * sum(bilateralFilter.mul(inputChannels[L])).val[0];
            outputImage.at<Vec3f>(i, j)[a] = (1 / bilateralFilterNorm) * sum(bilateralFilter.mul(inputChannels[a])).val[0];
            outputImage.at<Vec3f>(i, j)[b] = (1 / bilateralFilterNorm) * sum(bilateralFilter.mul(inputChannels[b])).val[0];
        }
    }

    // Discard the padding
    xRange = Range(padding, inputImage.rows - padding);
    yRange = Range(padding, inputImage.cols - padding);

    return outputImage(xRange, yRange);
}

/**
 * @brief Apply a Scaled Bilateral Filter to an image. This is the decoupled version of this filter, this means
 * that the weighting image for the filter can be different from its input image. The difference between this filter and the
 * not scaled version is that the weighting image is pre scaled through a Gaussian low pass filter for better performance
 * under heavy AWGN
 *
 * @param weightingImage image used to calculate the kernel's weight
 * @param inputImage image used as input for the filter
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param spatialSigma spatial standard deviation
 * @param rangeSigma range or radiometric standard deviation
 * @return Mat output image
 */
Mat Filters::ScaledBilateralFilter(const Mat &inputImage, const Mat &weightingImage, int windowSize, double spatialSigma, double rangeSigma)
{
    /* This filter uses a low pass filtered version of the input image as part of the weighting input.
     *  In this case with a Gaussian blur as LPF.
     */

    /**
     * It uses the two same Gaussian kernels as the bilateral filter, but it has an extra input, the scaled image \f$ U^s \f$.
     * The kernels are the following:
     * \f[ G_{\text spatial}(U^s, U, m, p) = \exp\left(-\frac{ ||m - p||^2 }{ 2 {\sigma_s^2} } \right) \f]
     * \f[ G_{\text range}(U^s, U, m, p) = \exp\left( -\frac{ ||U^s(m) - U(p)||^2 }{ 2{\sigma_r^2} } \right) \f]
     * Notice the diferrence on the range kernel calculation where the scaled image is used.
     */

    /**
     * The two kernels are multiplied to obtain the Bilateral Filter kernel:
     * \f[ \psi_{\text SBF}(U^s, U, m, p) = G_{\text spatial}(U^s, U, m, p) \, G_{\text range}(U^s, U, m, p) \f]
     *
     */
    ;

    /**
     * The Bilateral filter's norm corresponds to:
     * \f[ \left( \sum_{m \subset \Omega} \psi_{\text{SBF}}(U^s, U, m, p) \right)^{-1} \f]
     */

    /**
     * Finally the bilateral filter kernel can be convolved with the input as follows:
     * \f[ Y_{\psi_{\text SBF}}(p) = \left( \sum_{m \subset \Omega} \psi_{\text SBF}(U^s, U, m, p) \right)^{-1}
     * \left( \sum_{m \subset \Omega} \psi_{\text SBF}(U^s, U, m, p) \, U(m) \right) \f]
     */
    Mat scaledImage(weightingImage.size(), weightingImage.type());
    GaussianBlur(weightingImage, scaledImage, Size(windowSize, windowSize), spatialSigma, 0.0, BORDER_CONSTANT);
    return Filters::BilateralFilter(inputImage, scaledImage, windowSize, spatialSigma, rangeSigma);
}

/**
 * @brief Apply a Non Local Means Filter to an image. This is the decoupled version of this filter, this means
 * that the weighting image for the filter can be different from its input image. The algorithm used for this filter
 * is computationally demanding
 *
 * @param weightingImage_ image used to calculate the kernel's weight
 * @param inputImage_ image used as input for the filter
 * @param windowSize processing window size
 * @param neighborhoodSize subwindow size
 * @param rangeSigma range or radiometric standard deviation. Used to calculate the parameter \f$ h^2 = 2 \sigma_r^2 \f$
 * @return Mat output image
 */
Mat Filters::NonLocalMeansFilter(const Mat &inputImage_, const Mat &weightingImage_, int windowSize, int neighborhoodSize, double rangeSigma)
{
    // Set the padding value
    int padding = (windowSize - 1) / 2;

    // Working images
    Mat inputImage, weightingImage;

    // Add padding to the input for kernel consistency
    copyMakeBorder(inputImage_, inputImage, padding, padding, padding, padding, BORDER_CONSTANT);
    copyMakeBorder(weightingImage_, weightingImage, padding, padding, padding, padding, BORDER_CONSTANT);

    // NML standard deviation h
    double h = sqrt(2.0 * pow(rangeSigma, 2.0));

    // Prepare variables for the bilateral filtering
    Mat outputImage(inputImage.size(), inputImage.type());
    Mat inputRegion, weightRegion;
    Mat dL, da, db;
    Mat nonLocalMeansFilter;
    double nonLocalMeansFilterNorm;
    Range xRange, yRange;
    Vec3f pixel;
    Mat inputChannels[3], weightChannels[3], nlmChannels[3];
    int iMin, iMax, jMin, jMax;

// Set the parallelization pragma for OpenMP
#pragma omp parallel for private(iMin, iMax, jMin, jMax, xRange, yRange,                    \
                                 pixel, dL, da, db,                                         \
                                 nlmChannels, nonLocalMeansFilter, nonLocalMeansFilterNorm, \
                                 weightRegion, weightChannels, inputRegion, inputChannels)  \
    shared(inputImage, weightingImage, outputImage,                                         \
           windowSize, neighborhoodSize, rangeSigma)
    for (int i = padding; i < inputImage.rows - padding; i++)
    {
        iMin = i - padding;
        iMax = iMin + windowSize;
        xRange = Range(iMin, iMax);
        for (int j = padding; j < inputImage.cols - padding; j++)
        {
            jMin = j - padding;
            jMax = jMin + windowSize;
            yRange = Range(jMin, jMax);

            // Extract local region based on the window size
            weightRegion = weightingImage(xRange, yRange);

            /**
             * The discrete representation of the Non Local Means Filter is as follows:
             * \f[ \psi_{\text {NLM}}(U, m, p) = \sum_{B(m) \subseteq U} \exp\left( \frac{||B(m) - B(p)||^2}{h^2} \right)\f]
             * where  \f$B(p)\f$ is a patch part of the window \f$\Omega\f$ centered at pixel \f$p\f$. \f$B(m)\f$ represents all of the
             * patches at \f$\Omega\f$ centered in each \f$m\f$ pixel. The Non Local Means Filter calculates the Euclidean distance
             * between  each patch \f$B(m)\f$ and \f$B(p)\f$ for each window \f$\Omega \subseteq U\f$. This is why this algorithm is
             * demanding in computational terms. Each Euclidean distance matrix obtained from each patch pair is the input for
             * a Gaussian decreasing function with standard deviation \f$h\f$ that generates the new pixel \f$p\f$ value.
             */
            cv::split(weightRegion, weightChannels);
            nlmChannels[L] = lib.GaussianFunction(lib.RegionDistancesMatrix(weightChannels[L], neighborhoodSize), h);
            nlmChannels[a] = lib.GaussianFunction(lib.RegionDistancesMatrix(weightChannels[a], neighborhoodSize), h);
            nlmChannels[b] = lib.GaussianFunction(lib.RegionDistancesMatrix(weightChannels[b], neighborhoodSize), h);
            nonLocalMeansFilter = (nlmChannels[L] + nlmChannels[a] + nlmChannels[b]);

            /**
             * The Non Local Means filter's norm is calculated with:
             * \f[\left( \sum_{m \subset \Omega} \psi_{\text{NLM}}(U, m, p) \right)^{-1} \f]
             */
            nonLocalMeansFilterNorm = sum(nonLocalMeansFilter).val[0];

            /**
             * The NLM filter kernel is applied to the laplacian image:
             * \f[ Y_{\psi_{\text NLM}}(p) = \left( \sum_{m \subset \Omega} \psi_{\text NLM}(U, m, p) \right)^{-1}
             * \left( \sum_{m \subset \Omega} \psi_{\text NLM}(U, p, m) \, U(m) \right) \f]
             */
            inputRegion = inputImage(xRange, yRange);
            cv::split(inputRegion, inputChannels);
            outputImage.at<Vec3f>(i, j)[L] = (1 / nonLocalMeansFilterNorm) * sum(nonLocalMeansFilter.mul(inputChannels[L])).val[0];
            outputImage.at<Vec3f>(i, j)[a] = (1 / nonLocalMeansFilterNorm) * sum(nonLocalMeansFilter.mul(inputChannels[a])).val[0];
            outputImage.at<Vec3f>(i, j)[b] = (1 / nonLocalMeansFilterNorm) * sum(nonLocalMeansFilter.mul(inputChannels[b])).val[0];
        }
    }

    // Discard the padding
    xRange = Range(padding, inputImage.rows - padding);
    yRange = Range(padding, inputImage.cols - padding);
    return outputImage(xRange, yRange);
}

/**
 * @brief Apply a Guided Filter to an image. This is the decoupled version of this filter, this means
 * that the weighting image for the filter can be different from its input image. In this case the weighting
 * image is known as guiding image. This filter uses a linear algorithm, making it fast computationally
 *
 * @param inputImage image used as input for the filter
 * @param guidingImage image used to calculate the kernel's weight
 * @param windowSize processing window size
 * @param rangeSigma range or radiometric standard deviation. Used to calculate \f$ \epsilon = \sigma_r^2 \f$
 * @return Mat
 */
Mat Filters::GuidedFilter(const Mat &inputImage, const Mat &guidingImage, int windowSize, double rangeSigma)
{
    /**
     * The Guided Filter initialy has the same form as any WAF
     * \f[ q_i = \sum_j W_{ij}(I)p_j \f]
     * where \f$ q_i \f$ is the output if filtered pixel \f$ p_j \f$ with the
     * kernel \f$ W_{ij} \f$ guided by image \f$ I \f$. This is expected as the original idea for this
     * filter comes from the bilateral filter. The greatness in this filter lies
     * in its solution. It asumes a linear relationship:
     * \f[ q_i = a_k I_i  + b_k, \forall i \in \omega_k \f]
     * where \f$ \omega_k \f$ is a window centered at pixel \f$ k \f$.
     * The solution is in finding the linear coefficients \f$ a_k \f$ and \f$ b_k \f$.
     * This is achieved through the cost function
     * \f[ E(a_k, b_k) \sum_{i\in \omega_k} ((a_k I_i  + b_k - p_i)^2 + \epsilon a_k^2)\f]
     * which seeks to minimize the difference between the input \f$ p \f$ and the output \f$ q \f$.
     * The solution is found through a linear regresion
     * \f[ a_k = \frac{\frac{1}{|\omega|} \sum_{i\in \omega_k} I_i p_i - \mu_k \bar{p}_k}{\omega_k^2 + \epsilon}\f]
     * \f[ b_k = \bar{p}_k - a_k \mu_k \f]
     * \f$ \mu_k \f$ and \f$\omega_k^2\f$ are the mean and the variance of \f$I\f$ in \f$ \omega_k \f$, respectively.
     * \f$ \frac{1}{|\omega|} \sum_{i\in \omega_k} p_i \f$ is the mean of \f$p\f$ in \f$ \omega_k \f$.
     * Finally the output of the filter is
     * \f[ q_i = \frac{1}{|\omega|} \sum_{k:i \in \omega_k} (a_k I_i + b_k )\f]
     */

    int widowRadius = windowSize / 2;
    double epsilon = pow((rangeSigma), 2.0);
    return guidedFilter(guidingImage, inputImage, widowRadius, epsilon, -1);
}