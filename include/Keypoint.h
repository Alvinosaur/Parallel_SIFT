#ifndef _KEY_
#define _KEY_

#include "Image.h"
#include <vector>
#include <utility>

// discrete bins of histogram as a discrete range of angles
extern const std::vector<float> angle_bins;

class Keypoint {
    Image src;
	float grad_thresh;
	typedef std::pair<int, int> coord;
public:
    Keypoint(Image & src, float grad_thresh_x);
	void getMaxes(Image & prev_img, Image & cur_img, Image & next_img, 
	Image & res);

	void find_keypoints(std::vector<Image> & differences, 
		std::vector<Image> & keypoint_results);

	void find_xy_gradient(Image & remove_target, Image & grad_x_res, 
		Image & grad_y_res, bool is_remove, std::vector<coord> keypoints);

	bool is_edge(int grad_x, int grad_y);
};
#endif 