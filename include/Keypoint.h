#ifndef _KEY_
#define _KEY_

#include "Image.h"
#include "general_helpers.h"

#include <opencv2/highgui/highgui.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <vector>
#include <utility>

#define FEATURE_VEC_SIZE 128

// discrete bins of histogram as a discrete range of angles
extern const std::vector<float> angle_bins;

class KeypointFeature {
public:
	coord pos;
	int size;
	int angle;
	int magnitude;
	float grad_histogram[FEATURE_VEC_SIZE] = {0};
};

class PointWithAngle {
public:
	coord pos;
	int angle;  // angle in range [0, 360)
	float magnitude;
};

class Keypoint {
    Image src;
	float grad_thresh;
	float intensity_thresh;
public:
    Keypoint(Image & src, float grad_thresh_x, float intensity_thresh);
	void getMaxes(Image & prev_img, Image & cur_img, Image & next_img, 
	Image & res);

	double find_keypoints(std::vector<Image> & differences, 
		std::vector<Image> & keypoint_results);

	double find_corners_gradients(const Image & src, 
		std::vector<coord> & keypoints,
		std::vector<PointWithAngle> & points_with_angle);

	bool is_corner(int grad_x, int grad_y);

	double mark_keypoints(Image & src, std::vector<coord> & keypoints);

	double find_keypoint_orientations(std::vector<coord> & keypoints,
		std::vector<PointWithAngle> & all_points, 
		std::vector<KeypointFeature> & keypoints_with_angles, 
		int rows, int cols, int size);

	double store_features(std::vector<KeypointFeature> & kp_features,
		cv::Mat & descriptors);

	double store_keypoints(std::vector<KeypointFeature> & keypoints_src,
		std::vector<cv::KeyPoint> & cv_keypoints_dst, int octave, int cols);
};
#endif 