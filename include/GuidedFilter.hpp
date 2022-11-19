/**
 * @file Guidedfilter.hpp
 * @author Atılım Çetin
 * @author Nikolai Poliarnyi
 * @brief Guided filter implementation from https://github.com/atilimcetin/guided-filter
 * @date 2020-06-1
 * @date 2022-11-17
 *
 */

#ifndef GUIDED_FILTER_H
#define GUIDED_FILTER_H

#include <opencv2/opencv.hpp>

class GuidedFilterImpl;

/**
 * @brief An open source OpenCV guided filter implementation under the MIT license
 *
 */
class GuidedFilter
{
public:
    GuidedFilter(const cv::Mat &I, int r, double eps);
    ~GuidedFilter();

    cv::Mat filter(const cv::Mat &p, int depth = -1) const;

private:
    GuidedFilterImpl *impl_;
};

cv::Mat guidedFilter(const cv::Mat &I, const cv::Mat &p, int r, double eps, int depth = -1);

#endif
