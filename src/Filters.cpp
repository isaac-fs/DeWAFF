#include "Filters.hpp"

Mat Filters::BilateralFilter(const Mat &weightingImage, const Mat &inputImage, const int windowSize, const double spatialSigma, const int rangeSigma) {
    // Set the paddingvalue
    int padding = (windowSize - 1) / 2;

    // Working images
    Mat input, weight;

    // Add padding to the input for kernel consistency
    copyMakeBorder(inputImage, input, padding, padding, padding, padding, BORDER_CONSTANT);
    copyMakeBorder(weightingImage, weight, padding, padding, padding, padding, BORDER_CONSTANT);

    // Pre computation of meshgrid values
    Mat1f X, Y, S;
    Range range = Range(-(windowSize/2), (windowSize/2) + 1);
    Utils::MeshGrid(range, X, Y);

    /**
     * @brief Pre computates the spatial Gaussian kernel
     *  Calculate the kernel variable \f$ S = X^2 + Y^2 \f$
     */
    cv::pow(X, 2, X);
    cv::pow(Y, 2, Y);
    S = X + Y;

    Mat spatialGaussian = Utils::GaussianFunction(S, spatialSigma);

    // Variance calculations
    int rangeVariance = std::pow(rangeSigma, 2);

    // Prepare variables for the bilateral filtering
    Mat output(input.size(), input.type());
    Mat weightRegion, inputRegion;
    Mat dL, da, db;
    Mat bilateralFilterKernel, rangeGaussian;
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
            * Compute a range Gaussian kernel \f$ G_{\text range}(U, m, p) = \exp\left( -\frac{ ||U(m) - U(p)||^2 }{ 2{\sigma_s^2} } \right) \f$
            * with the range values (pixel intensities) from an input weightRegion \f$ \Omega \in U \f$.
            * The range kernel uses the \f$ m_i \in \Omega \f$ pixels intensities as weighting values for the pixel \f$ p = (x, y) \f$ instead of their
            * locations as in the spatial kernel computation. In this case a the input \f$ U \f$ is separated into the three CIELab weightChannels and each
            * channel is processed as an individual input \f$ U_{\text channel} \f$
            */
            pixel = weight.at<Vec3f>(i, j);
            cv::pow(weightChannels[L] - pixel.val[L], 2, dL);
            cv::pow(weightChannels[a] - pixel.val[a], 2, da);
            cv::pow(weightChannels[b] - pixel.val[b], 2, db);
            cv::exp((dL + da + db) / (-2 * rangeVariance), rangeGaussian);

            /**
            * Convolute the spatial and range gaussian kernels to obtain the bilateral filter kernel
            * \f$ \phi_{\text BF}(U, m, p) = G_{\text spatial}(|| m-p ||) \, G_{\text range}(|| U(m)-U(p) ||) \f$
            *
            */
            bilateralFilterKernel = spatialGaussian.mul(rangeGaussian);

            /**
            * Calculate the Bilateral filter's norm
            * \f$ \left( \sum_{m \in \Omega} \phi_{\text{BF}}(U, m, p) \right)^{-1} \f$
            */
            bilateralFilterKernelNorm = sum(bilateralFilterKernel).val[0];

            /**
             * Apply the bilateral filter kernel to the laplacian input. The Laplacian deceive consists on weighting the Bilateral Filter kernel with the
            * original input and use the USM input as input for the filter
            * \f$ Y_{\phi_{\text BF}}(p) = \left( \sum_{m \in \Omega} \phi_{\text BF}(U, m, p) \right)^{-1}
            * \left( \sum_{m \in \Omega} \phi_{\text BF}(U, p, m) \, \hat{f}_{\text USM}(m) \right) \f$
            */
            inputRegion = input(xRange, yRange);
            cv::split(inputRegion, inputChannels);
            output.at<Vec3f>(i,j)[L] = (1/bilateralFilterKernelNorm) * sum(bilateralFilterKernel.mul(inputChannels[L])).val[0];
            output.at<Vec3f>(i,j)[a] = (1/bilateralFilterKernelNorm) * sum(bilateralFilterKernel.mul(inputChannels[a])).val[0];
            output.at<Vec3f>(i,j)[b] = (1/bilateralFilterKernelNorm) * sum(bilateralFilterKernel.mul(inputChannels[b])).val[0];
        }
    }

    xRange = Range(padding, padding + inputImage.rows);
    yRange = Range(padding, padding + inputImage.cols);

    return output(xRange, yRange);
}

Mat Filters::ScaledBilateralFilter(const Mat &weightingImage, const Mat &inputImage, const int windowSize, const double spatialSigma, const int rangeSigma) {
     // Set the paddingvalue
    int padding = (windowSize - 1) / 2;

    // Working images
    Mat input, weight;

    // Add padding to the input for kernel consistency
    copyMakeBorder(inputImage, input, padding, padding, padding, padding, BORDER_CONSTANT);
    copyMakeBorder(weightingImage, weight, padding, padding, padding, padding, BORDER_CONSTANT);

    // Pre computation of meshgrid values
    Mat1f X, Y, S;
    Range range = Range(-(windowSize/2), (windowSize/2) + 1);
    Utils::MeshGrid(range, X, Y);

    /**
     * @brief Pre computates the spatial Gaussian kernel
     *  Calculate the kernel variable \f$ S = X^2 + Y^2 \f$
     */
    cv::pow(X, 2, X);
    cv::pow(Y, 2, Y);
    S = X + Y;

    Mat spatialGaussian = Utils::GaussianFunction(S, spatialSigma);

    // Variance calculations
    int rangeVariance = std::pow(rangeSigma, 2);

    // Compute a low pass filtered version of the input image
    // In this case use a Gaussian blur as LPF
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
            * Compute a range Gaussian kernel \f$ G_{\text range}(U, m, p) = \exp\left( -\frac{ ||U(m) - U(p)||^2 }{ 2{\sigma_s^2} } \right) \f$
            * with the range values (pixel intensities) from an image region \f$ \Omega \in U \f$.
            * The range kernel uses the \f$ m_i \in \Omega \f$ pixels intensities as weighting values for the pixel \f$ p = (x, y) \f$ instead of their
            * locations as in the spatial kernel computation. In this case a the image \f$ U \f$ is separated into the three CIELab scaledChannels and each
            * channel is processed as an individual image \f$ U_{\text channel} \f$
            */
            pixel = weight.at<Vec3f>(i, j);
            pow(scaledChannels[L] - pixel.val[L], 2, dL);
            pow(scaledChannels[a] - pixel.val[a], 2, da);
            pow(scaledChannels[b] - pixel.val[b], 2, db);
            exp((dL + da + db) / (-2 * rangeVariance), rangeGaussian);

            /**
            * Convolute the spatial and range gaussian kernels to obtain the bilateral filter kernel
            * \f$ \phi_{\text BF}(U, m, p) = G_{\text spatial}(|| m-p ||) \, G_{\text range}(|| U(m)-U(p) ||) \f$
            *
            */
            scaledBilateralKernel = spatialGaussian.mul(rangeGaussian);

            /**
            * Calculate the Bilateral filter's norm
            * \f$ \left( \sum_{m \in \Omega} \phi_{\text{BF}}(U, m, p) \right)^{-1} \f$
            */
            scaledBilateralKernelNorm = sum(scaledBilateralKernel).val[0];

            /**
             * Apply the bilateral filter kernel to the laplacian image. The Laplacian deceive consists on weighting the Bilateral Filter kernel with the
            * original image and use the USM image as input for the filter
            * \f$ Y_{\phi_{\text BF}}(p) = \left( \sum_{m \in \Omega} \phi_{\text BF}(U, m, p) \right)^{-1}
            * \left( \sum_{m \in \Omega} \phi_{\text BF}(U, p, m) \, \hat{f}_{\text USM}(m) \right) \f$
            */
            inputRegion = input(xRange, yRange);
            cv::split(inputRegion, inputChannels);
            output.at<Vec3f>(i,j)[L] = (1/scaledBilateralKernelNorm) * sum(scaledBilateralKernel.mul(inputChannels[L])).val[0];
            output.at<Vec3f>(i,j)[a] = (1/scaledBilateralKernelNorm) * sum(scaledBilateralKernel.mul(inputChannels[a])).val[0];
            output.at<Vec3f>(i,j)[b] = (1/scaledBilateralKernelNorm) * sum(scaledBilateralKernel.mul(inputChannels[b])).val[0];
        }
    }

    xRange = Range(padding, padding + inputImage.rows);
    yRange = Range(padding, padding + inputImage.cols);

    return output(xRange, yRange);
}

Mat Filters::NonLocalMeansFilter(const Mat &weightingImage, const Mat &inputImage, const int windowSize, const int patchSize, const double rangeSigma) {
     // Set the paddingvalue
    int padding = (windowSize - 1) / 2;

    // Working images
    Mat input, weight;

    // Add padding to the input for kernel consistency
    copyMakeBorder(inputImage, input, padding, padding, padding, padding, BORDER_CONSTANT);
    copyMakeBorder(weightingImage, weight, padding, padding, padding, padding, BORDER_CONSTANT);

    // Variable
    double h = sqrt(rangeSigma)/2;

    // Prepare variables for the bilateral filtering
    Mat output(input.size(), input.type());
    Mat inputRegion, weightRegion, extendedRegion;
    Mat dL, da, db;
    Mat nonLocalMeansKernel, edmGaussian;
	double nonLocalMeansKernelNorm;
    Range xRange, yRange;
	Vec3f pixel;
    Mat inputChannels[3], weightChannels[3], nlmChannels[3];
    int iMin, iMax, jMin, jMax;

    // Set the parallelization pragma for OpenMP
    #pragma omp parallel for\
    private(iMin, iMax, jMin, jMax, xRange, yRange,\
            pixel, dL, da, db, edmGaussian,\
            nonLocalMeansKernel, nonLocalMeansKernelNorm,\
            weightRegion, weightChannels, inputRegion, inputChannels, nlmChannels)\
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
            * Compute a range Gaussian kernel \f$ G_{\text range}(U, m, p) = \exp\left( -\frac{ ||U(m) - U(p)||^2 }{ 2{\sigma_s^2} } \right) \f$
            * with the range values (pixel intensities) from an image region \f$ \Omega \in U \f$.
            * The range kernel uses the \f$ m_i \in \Omega \f$ pixels intensities as weighting values for the pixel \f$ p = (x, y) \f$ instead of their
            * locations as in the spatial kernel computation. In this case a the image \f$ U \f$ is separated into the three CIELab channels and each
            * channel is processed as an individual image \f$ U_{\text channel} \f$
            */
            cv::split(weightRegion, weightChannels);
            nlmChannels[L] = Utils::GaussianFunction(Utils::EuclideanDistanceMatrix(weightChannels[L], windowSize, patchSize), h);
            nlmChannels[a] = Utils::GaussianFunction(Utils::EuclideanDistanceMatrix(weightChannels[a], windowSize, patchSize), h);
            nlmChannels[b] = Utils::GaussianFunction(Utils::EuclideanDistanceMatrix(weightChannels[b], windowSize, patchSize), h);

            //
            nonLocalMeansKernel = (nlmChannels[L] + nlmChannels[a] + nlmChannels[b]);

            /**
            * Calculate the Bilateral filter's norm
            * \f$ \left( \sum_{m \in \Omega} \phi_{\text{BF}}(U, m, p) \right)^{-1} \f$
            */
            nonLocalMeansKernelNorm = sum(nonLocalMeansKernel).val[0];

            /**
             * Apply the bilateral filter kernel to the laplacian image. The Laplacian deceive consists on weighting the Bilateral Filter kernel with the
            * original image and use the USM image as input for the filter
            * \f$ Y_{\phi_{\text BF}}(p) = \left( \sum_{m \in \Omega} \phi_{\text BF}(U, m, p) \right)^{-1}
            * \left( \sum_{m \in \Omega} \phi_{\text BF}(U, p, m) \, \hat{f}_{\text USM}(m) \right) \f$
            */
            inputRegion = input(xRange, yRange);
            cv::split(inputRegion, inputChannels);
            output.at<Vec3f>(i,j)[L] = (1/nonLocalMeansKernelNorm) * sum(nonLocalMeansKernel.mul(inputChannels[L])).val[0];
            output.at<Vec3f>(i,j)[a] = (1/nonLocalMeansKernelNorm) * sum(nonLocalMeansKernel.mul(inputChannels[a])).val[0];
            output.at<Vec3f>(i,j)[b] = (1/nonLocalMeansKernelNorm) * sum(nonLocalMeansKernel.mul(inputChannels[b])).val[0];
        }
    }

    xRange = Range(padding, padding + inputImage.rows);
    yRange = Range(padding, padding + inputImage.cols);

    return output(xRange, yRange);
}

Mat Filters::GuidedFilter(const Mat &inputImage, const Mat &guidingImage, const int windowSize, const int rangeSigma) {
    double epsilon = pow(rangeSigma, 2);//1 / pow(rangeSigma, 2);
    return guidedFilter(guidingImage, inputImage, windowSize, epsilon, -1);
}