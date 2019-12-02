#ifndef _KEY_
#define _KEY_

#include "Image.h"
#include "general_helpers.h"

#include <vector>
#include <utility>

// discrete bins of histogram as a discrete range of angles
extern const std::vector<float> angle_bins;

class PointWithAngle {
public:
	coord pos;
	float angle;
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

	void find_keypoints(std::vector<Image> & differences, 
		std::vector<Image> & keypoint_results);

	void find_corners_gradients(const Image & src, 
		std::vector<coord> & keypoints,
		std::vector<PointWithAngle> & points_with_angle);

	bool is_corner(int grad_x, int grad_y);

	void mark_keypoints(Image & src, std::vector<coord> & keypoints);

	void find_keypoint_orientations(std::vector<coord> & keypoints,
		std::vector<PointWithAngle> all_points, 
		std::vector<PointWithAngle> keypoints_with_angles, 
		int rows, int cols);
};
#endif 