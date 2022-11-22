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
Mat Filters::BilateralFilter(const Mat &weightingImage_, const Mat &inputImage_, const int windowSize, const double spatialSigma, const int rangeSigma) {
    // Set the padding value
    int padding = (windowSize - 1) / 2;

    // Working images
    Mat inputImage, weightingImage;

    // Add padding to the input for kernel consistency
    copyMakeBorder(inputImage_, inputImage, padding, padding, padding, padding, BORDER_REPLICATE);
    copyMakeBorder(weightingImage_, weightingImage, padding, padding, padding, padding, BORDER_REPLICATE);

    // Pre compute the m - p = |m-p| factors
    Mat X, Y;
    Range range = Range((-windowSize/2), (windowSize/2) + 1);
    lib.MeshGrid(range, X, Y);
    pow(X, 2, X);
    pow(Y, 2, Y);
    Mat euclideanDistances = X + Y;

    /**
    * This filter uses two Gaussian kernels, one of them is the spatial Gaussian kernel
    * \f$ G_{\text spatial}(U, m, p) = \exp\left(-\frac{ ||m - p||^2 }{ 2 {\sigma_s^2} } \right) \f$
    * with the spatial values from an image region \f$ \Omega \subseteq U \f$.
    * The spatial kernel uses the \f$ m_i \subseteq \Omega \f$ pixels coordinates as weighting values for the pixel \f$ p = (x, y) \f$
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
    #pragma omp parallel for\
    private(iMin, iMax, jMin, jMax, xRange, yRange,\
            weightingRegion, weightingChannels, inputRegion, inputChannels,\
            pixel, dL, da, db, rangeGaussian,\
            bilateralFilter, bilateralFilterNorm)\
	shared( inputImage, weightingImage, outputImage,\
            windowSize, spatialSigma, rangeSigma)
    for(int i = padding; i < inputImage.rows - padding; i++) {
        iMin = i - padding;
        iMax = iMin + windowSize;
        xRange = Range(iMin, iMax);
        for(int j = padding; j < inputImage.cols - padding; j++) {
            jMin = j -  padding;
            jMax = jMin + windowSize;
            yRange = Range(jMin, jMax);

            // Extract local weightRegion based on the window size
            weightingRegion = weightingImage(xRange, yRange);
            cv::split(weightingRegion, weightingChannels);

            /**
            * The other Gaussian kernel is the range Gaussian kernel \f$ G_{\text range}(U, m, p) = \exp\left( -\frac{ ||U(m) - U(p)||^2 }{ 2{\sigma_s^2} } \right) \f$
            * with the intensity (range) values from an image region \f$ \Omega \subseteq U \f$.
            * The range kernel uses the \f$ m_i \subseteq \Omega \f$ pixels intensities as weighting values for the pixel \f$ p = (x, y) \f$ instead of their
            * locations as in the spatial kernel computation. In this case a the input \f$ U \f$ is separated into the three CIELab weightChannels and each
            * channel is processed as an individual input \f$ U_{\text channel} \f$
            */
            pixel = weightingImage.at<Vec3f>(i, j);
            cv::pow(weightingChannels[L] - pixel.val[L], 2, dL);
            cv::pow(weightingChannels[a] - pixel.val[a], 2, da);
            cv::pow(weightingChannels[b] - pixel.val[b], 2, db);
            rangeGaussian = lib.GaussianFunction(dL + da + db, rangeSigma);

            /**
            * The two kernels convolve to obtain the Bilateral Filter kernel \n
            * \f$ \phi_{\text BF}(U, m, p) = G_{\text spatial}(||m-p||) \, G_{\text range}(||U(m)-U(p)||) \f$
            *
            */;
            bilateralFilter = spatialGaussian.mul(rangeGaussian);

            /**
            * The Bilateral filter's norm corresponds to
            * \f$ \left( \sum_{m \subseteq \Omega} \phi_{\text{BF}}(U, m, p) \right)^{-1} \f$
            */
            bilateralFilterNorm = sum(bilateralFilter).val[0];

            /**
            * Finally the bilateral filter kernel can be convolved with the input as follows \n
            * \f$ Y_{\phi_{\text BF}}(p) = \left( \sum_{m \subseteq \Omega} \phi_{\text BF}(U, m, p) \right)^{-1}
            * \left( \sum_{m \subseteq \Omega} \phi_{\text BF}(U, p, m) \, \hat{f}_{\text USM}(m) \right) \f$
            */
            inputRegion = inputImage(xRange, yRange);
            cv::split(inputRegion, inputChannels);
            outputImage.at<Vec3f>(i,j)[L] = (1/bilateralFilterNorm) * sum(bilateralFilter.mul(inputChannels[L])).val[0];
            outputImage.at<Vec3f>(i,j)[a] = (1/bilateralFilterNorm) * sum(bilateralFilter.mul(inputChannels[a])).val[0];
            outputImage.at<Vec3f>(i,j)[b] = (1/bilateralFilterNorm) * sum(bilateralFilter.mul(inputChannels[b])).val[0];
        }
    }

    // Discard the padding
    xRange = Range(padding, inputImage_.rows + padding);
    yRange = Range(padding, inputImage_.cols + padding);
    return outputImage(xRange, yRange);
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
    /* This filter uses a low pass filtered version of the input image as part of the weighting input.
    *  In this case with a Gaussian blur as LPF
    */

    /**
    * It uses the two same Gaussian kernels as the bilateral filter, but it has an extra input, the scaled image \f$ U^s \f$.\n
    * The kernels are the following: \n
    * \f$ G_{\text spatial}(U^s, U, m, p) = \exp\left(-\frac{ ||m - p||^2 }{ 2 {\sigma_s^2} } \right) \f$ \n
    * \f$ G_{\text range}(U^s, U, m, p) = \exp\left( -\frac{ ||U^s(m) - U(p)||^2 }{ 2{\sigma_s^2} } \right) \f$ \n
    * Notice the diferrence on the range kernel calculation where the scaled image is used
    */

   /**
    * The two kernels convolve to obtain the Bilateral Filter kernel \n
    * \f$ \phi_{\text SBF}(U^s, U, m, p) = G_{\text spatial}(||m-p||) \, G_{\text range}(||U^s(m)-U(p)||) \f$
    *
    */;

     /**
    * The Bilateral filter's norm corresponds to
    * \f$ \left( \sum_{m \subseteq \Omega} \phi_{\text{SBF}}(U^s, U, m, p) \right)^{-1} \f$
    */

   /**
    * Finally the bilateral filter kernel can be convolved with the input as follows \n
    * \f$ Y_{\phi_{\text SBF}}(p) = \left( \sum_{m \subseteq \Omega} \phi_{\text SBF}(U^s, U, m, p) \right)^{-1}
    * \left( \sum_{m \subseteq \Omega} \phi_{\text SBF}(U^s, U, m, p) \, \hat{f}_{\text USM}(m) \right) \f$
    */
    Mat scaledImage(weightingImage.size(), weightingImage.type());
    GaussianBlur(weightingImage, scaledImage, Size(windowSize, windowSize), spatialSigma, 0.0, BORDER_CONSTANT);
    return Filters::BilateralFilter(scaledImage, inputImage, windowSize, spatialSigma, rangeSigma);
}

/**
 * @brief Apply a Non Local Means Filter to an image. This is the decoupled version of this filter, this means
 * that the weighting image for the filter can be different from the input image. The algorithm used for this filter
 * is very computationally demanding
 *
 * @param weightingImage image used to calculate the kernel's weight
 * @param inputImage image used as input for the filter
 * @param windowSize processing window size, has to be odd numbered and greater or equal than 3
 * @param rangeSigma range or radiometric standard deviation. Used to calculate the parameter \f$ h = 2 \sigma^2 \f$
 * @return Mat output image
 */
Mat Filters::NonLocalMeansFilter(const Mat &weightingImage, const Mat &inputImage, const int windowSize, const int neighborhoodSize, const int rangeSigma) {
     // Set the padding value
    int padding = (windowSize - 1) / 2;

    // Working images
    Mat input, weight;

    // Add padding to the input for kernel consistency
    copyMakeBorder(inputImage, input, padding, padding, padding, padding, BORDER_CONSTANT);
    copyMakeBorder(weightingImage, weight, padding, padding, padding, padding, BORDER_CONSTANT);

    // NML standard deviation h
    double h = 2*pow(rangeSigma,2);

    // Prepare variables for the bilateral filtering
    Mat output(input.size(), input.type());
    Mat inputRegion, weightRegion;
    Mat dL, da, db;
    Mat nonLocalMeansFilter;
	double nonLocalMeansFilterNorm;
    Range xRange, yRange;
	Vec3f pixel;
    Mat inputChannels[3], weightChannels[3], nlmChannels[3];
    int iMin, iMax, jMin, jMax;

    // Set the parallelization pragma for OpenMP
    #pragma omp parallel for\
    private(iMin, iMax, jMin, jMax, xRange, yRange,\
            pixel, dL, da, db,\
            nlmChannels, nonLocalMeansFilter, nonLocalMeansFilterNorm,\
            weightRegion, weightChannels, inputRegion, inputChannels)\
	shared( input, weight, output,\
            windowSize, neighborhoodSize, rangeSigma)
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
            * The discrete representation of the Non Local Means Filter is as follows \n
            * \f$ \phi_{\text {NLM}}(U, m, p) = \sum_{B(m) \subseteq U} \exp\left( \frac{||B(m) - B(p)||^2}{h^2} \right)\f$
            * where  \f$B(p)\f$ is a patch part of the window \f$\Omega\f$ centered at pixel \f$p\f$. \f$B(m)\f$ represents all of the
            * patches at \f$\Omega\f$ centered in each \f$m\f$ pixel. The Non Local Means Filter calculates the Euclidean distance
            * between  each patch \f$B(m)\f$ and \f$B(p)\f$ for each window \f$\Omega \subseteq U\f$. This is why this algorithm is
            * highly demanding in computational terms. Each Euclidean distance matrix obtained from each patch pair is the input for
            * a Gaussian decreasing function with standard deviation \f$h\f$ that generates the new pixel \f$p\f$ value
            */
            cv::split(weightRegion, weightChannels);
            nlmChannels[L] = lib.GaussianFunction(lib.EuclideanDistanceMatrix(weightChannels[L], neighborhoodSize), h);
            nlmChannels[a] = lib.GaussianFunction(lib.EuclideanDistanceMatrix(weightChannels[a], neighborhoodSize), h);
            nlmChannels[b] = lib.GaussianFunction(lib.EuclideanDistanceMatrix(weightChannels[b], neighborhoodSize), h);
            nonLocalMeansFilter = (nlmChannels[L] + nlmChannels[a] + nlmChannels[b]);

            /**
            * The Non Local Means filter's norm is calculated with \n
            * \f$ \phi_{\text NLM}(U, m, p) \left( \sum_{m \subseteq \Omega} \phi_{\text{NLM}}(U, m, p) \right)^{-1} \f$
            */
            nonLocalMeansFilterNorm = sum(nonLocalMeansFilter).val[0];

            /**
             * The NLM filter kernel is applied to the laplacian image \n
            * \f$ Y_{\phi_{\text NLM}}(p) = \left( \sum_{m \subseteq \Omega} \phi_{\text NLM}(U, m, p) \right)^{-1}
            * \left( \sum_{m \subseteq \Omega} \phi_{\text NLM}(U, p, m) \, \hat{f}_{\text USM}(m) \right) \f$
            */
            inputRegion = input(xRange, yRange);
            cv::split(inputRegion, inputChannels);
            output.at<Vec3f>(i,j)[L] = (1/nonLocalMeansFilterNorm) * sum(nonLocalMeansFilter.mul(inputChannels[L])).val[0];
            output.at<Vec3f>(i,j)[a] = (1/nonLocalMeansFilterNorm) * sum(nonLocalMeansFilter.mul(inputChannels[a])).val[0];
            output.at<Vec3f>(i,j)[b] = (1/nonLocalMeansFilterNorm) * sum(nonLocalMeansFilter.mul(inputChannels[b])).val[0];
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

// Below this line is the Guided Filter implementation
/**
 * @author Atılım Çetin
 * @author Nikolai Poliarnyi
 * @brief Guided filter implementation from https://github.com/atilimcetin/guided-filter
 * @brief An open source OpenCV guided filter implementation under the MIT license
 * @date 2020-06-1
 *
 */

static cv::Mat boxfilter(const cv::Mat &I, int r)
{
    cv::Mat result;
    cv::blur(I, result, cv::Size(r, r), cv::Point(-1, -1), cv::BORDER_REPLICATE);
    return result;
}

static cv::Mat convertTo(const cv::Mat &mat, int depth)
{
    if (mat.depth() == depth)
        return mat;

    cv::Mat result;
    mat.convertTo(result, depth);
    return result;
}

class GuidedFilterImpl
{
public:
    virtual ~GuidedFilterImpl() {}

    cv::Mat filter(const cv::Mat &p, int depth);

protected:
    int Idepth;

private:
    virtual cv::Mat filterSingleChannel(const cv::Mat &p) const = 0;
};

class GuidedFilterMono : public GuidedFilterImpl
{
public:
    GuidedFilterMono(const cv::Mat &I, int r, double eps);

private:
    virtual cv::Mat filterSingleChannel(const cv::Mat &p) const;

private:
    int r;
    double eps;
    cv::Mat I, mean_I, var_I;
};

class GuidedFilterColor : public GuidedFilterImpl
{
public:
    GuidedFilterColor(const cv::Mat &I, int r, double eps);

private:
    virtual cv::Mat filterSingleChannel(const cv::Mat &p) const;

private:
    std::vector<cv::Mat> Ichannels;
    int r;
    double eps;
    cv::Mat mean_I_r, mean_I_g, mean_I_b;
    cv::Mat invrr, invrg, invrb, invgg, invgb, invbb;
};


cv::Mat GuidedFilterImpl::filter(const cv::Mat &p, int depth)
{
    cv::Mat p2 = convertTo(p, Idepth);

    cv::Mat result;
    if (p.channels() == 1)
    {
        result = filterSingleChannel(p2);
    }
    else
    {
        std::vector<cv::Mat> pc;
        cv::split(p2, pc);

        for (std::size_t i = 0; i < pc.size(); ++i)
            pc[i] = filterSingleChannel(pc[i]);

        cv::merge(pc, result);
    }

    return convertTo(result, depth == -1 ? p.depth() : depth);
}

GuidedFilterMono::GuidedFilterMono(const cv::Mat &origI, int r, double eps) : r(r), eps(eps)
{
    if (origI.depth() == CV_32F || origI.depth() == CV_64F)
        I = origI.clone();
    else
        I = convertTo(origI, CV_32F);

    Idepth = I.depth();

    mean_I = boxfilter(I, r);
    cv::Mat mean_II = boxfilter(I.mul(I), r);
    var_I = mean_II - mean_I.mul(mean_I);
}

cv::Mat GuidedFilterMono::filterSingleChannel(const cv::Mat &p) const
{
    cv::Mat mean_p = boxfilter(p, r);
    cv::Mat mean_Ip = boxfilter(I.mul(p), r);
    cv::Mat cov_Ip = mean_Ip - mean_I.mul(mean_p); // this is the covariance of (I, p) in each local patch.

    cv::Mat a = cov_Ip / (var_I + eps); // Eqn. (5) in the paper;
    cv::Mat b = mean_p - a.mul(mean_I); // Eqn. (6) in the paper;

    cv::Mat mean_a = boxfilter(a, r);
    cv::Mat mean_b = boxfilter(b, r);

    return mean_a.mul(I) + mean_b;
}

GuidedFilterColor::GuidedFilterColor(const cv::Mat &origI, int r, double eps) : r(r), eps(eps)
{
    cv::Mat I;
    if (origI.depth() == CV_32F || origI.depth() == CV_64F)
        I = origI.clone();
    else
        I = convertTo(origI, CV_32F);

    Idepth = I.depth();

    cv::split(I, Ichannels);

    mean_I_r = boxfilter(Ichannels[0], r);
    mean_I_g = boxfilter(Ichannels[1], r);
    mean_I_b = boxfilter(Ichannels[2], r);

    // variance of I in each local patch: the matrix Sigma in Eqn (14).
    // Note the variance in each local patch is a 3x3 symmetric matrix:
    //           rr, rg, rb
    //   Sigma = rg, gg, gb
    //           rb, gb, bb
    cv::Mat var_I_rr = boxfilter(Ichannels[0].mul(Ichannels[0]), r) - mean_I_r.mul(mean_I_r) + eps;
    cv::Mat var_I_rg = boxfilter(Ichannels[0].mul(Ichannels[1]), r) - mean_I_r.mul(mean_I_g);
    cv::Mat var_I_rb = boxfilter(Ichannels[0].mul(Ichannels[2]), r) - mean_I_r.mul(mean_I_b);
    cv::Mat var_I_gg = boxfilter(Ichannels[1].mul(Ichannels[1]), r) - mean_I_g.mul(mean_I_g) + eps;
    cv::Mat var_I_gb = boxfilter(Ichannels[1].mul(Ichannels[2]), r) - mean_I_g.mul(mean_I_b);
    cv::Mat var_I_bb = boxfilter(Ichannels[2].mul(Ichannels[2]), r) - mean_I_b.mul(mean_I_b) + eps;

    // Inverse of Sigma + eps * I
    invrr = var_I_gg.mul(var_I_bb) - var_I_gb.mul(var_I_gb);
    invrg = var_I_gb.mul(var_I_rb) - var_I_rg.mul(var_I_bb);
    invrb = var_I_rg.mul(var_I_gb) - var_I_gg.mul(var_I_rb);
    invgg = var_I_rr.mul(var_I_bb) - var_I_rb.mul(var_I_rb);
    invgb = var_I_rb.mul(var_I_rg) - var_I_rr.mul(var_I_gb);
    invbb = var_I_rr.mul(var_I_gg) - var_I_rg.mul(var_I_rg);

    cv::Mat covDet = invrr.mul(var_I_rr) + invrg.mul(var_I_rg) + invrb.mul(var_I_rb);

    invrr /= covDet;
    invrg /= covDet;
    invrb /= covDet;
    invgg /= covDet;
    invgb /= covDet;
    invbb /= covDet;
}

cv::Mat GuidedFilterColor::filterSingleChannel(const cv::Mat &p) const
{
    cv::Mat mean_p = boxfilter(p, r);

    cv::Mat mean_Ip_r = boxfilter(Ichannels[0].mul(p), r);
    cv::Mat mean_Ip_g = boxfilter(Ichannels[1].mul(p), r);
    cv::Mat mean_Ip_b = boxfilter(Ichannels[2].mul(p), r);

    // covariance of (I, p) in each local patch.
    cv::Mat cov_Ip_r = mean_Ip_r - mean_I_r.mul(mean_p);
    cv::Mat cov_Ip_g = mean_Ip_g - mean_I_g.mul(mean_p);
    cv::Mat cov_Ip_b = mean_Ip_b - mean_I_b.mul(mean_p);

    cv::Mat a_r = invrr.mul(cov_Ip_r) + invrg.mul(cov_Ip_g) + invrb.mul(cov_Ip_b);
    cv::Mat a_g = invrg.mul(cov_Ip_r) + invgg.mul(cov_Ip_g) + invgb.mul(cov_Ip_b);
    cv::Mat a_b = invrb.mul(cov_Ip_r) + invgb.mul(cov_Ip_g) + invbb.mul(cov_Ip_b);

    cv::Mat b = mean_p - a_r.mul(mean_I_r) - a_g.mul(mean_I_g) - a_b.mul(mean_I_b); // Eqn. (15) in the paper;

    return (boxfilter(a_r, r).mul(Ichannels[0])
          + boxfilter(a_g, r).mul(Ichannels[1])
          + boxfilter(a_b, r).mul(Ichannels[2])
          + boxfilter(b, r));  // Eqn. (16) in the paper;
}


GuidedFilter::GuidedFilter(const cv::Mat &I, int r, double eps)
{
    CV_Assert(I.channels() == 1 || I.channels() == 3);

    if (I.channels() == 1)
        impl_ = new GuidedFilterMono(I, 2 * r + 1, eps);
    else
        impl_ = new GuidedFilterColor(I, 2 * r + 1, eps);
}

GuidedFilter::~GuidedFilter()
{
    delete impl_;
}

cv::Mat GuidedFilter::filter(const cv::Mat &p, int depth) const
{
    return impl_->filter(p, depth);
}

cv::Mat guidedFilter(const cv::Mat &I, const cv::Mat &p, int r, double eps, int depth)
{
    return GuidedFilter(I, r, eps).filter(p, depth);
}
