/**
 * @file GuidedFilter.hpp
 * @author Atılım Çetin
 * @author Nikolai Poliarnyi
 * @brief Guided filter implementation from https://github.com/atilimcetin/guided-filter
 * @brief An open source OpenCV guided filter implementation under the MIT license
 * @date 2020-06-1
 *
 */

#ifndef GUIDED_FILTER_HPP_
#define GUIDED_FILTER_HPP_


#include <opencv2/opencv.hpp>
#include "opencv2/core/core.hpp"

class GuidedFilterImpl;

class GuidedFilter {
public:
	GuidedFilter(const cv::Mat &I, int r, double eps);
	~GuidedFilter();

	cv::Mat filter(const cv::Mat &p, int depth = -1) const;

private:
	GuidedFilterImpl *impl_;
};

cv::Mat guidedFilter(const cv::Mat &I, const cv::Mat &p, int r, double eps, int depth = -1);

#endif /* GUIDED_FILTER_HPP_ */