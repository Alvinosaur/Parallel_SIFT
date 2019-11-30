#ifndef _KEY_
#define _KEY_

#include "Image.h"
#include "general_helpers.h"

#include <vector>
#include <utility>

// discrete bins of histogram as a discrete range of angles
extern const std::vector<float> angle_bins;

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
		std::vector<coord> & keypoints, float* grad_magnitudes, 
		float* grad_orientations);

	bool is_corner(int grad_x, int grad_y);

	void mark_keypoints(Image & src, std::vector<coord> & keypoints);

	void find_keypoint_orientations(std::vector<coord> & keypoints,
		float* grad_magnitudes, float* grad_orientations, 
		std::vector<float> & kp_gradients, int rows, int cols);
};
#endif 