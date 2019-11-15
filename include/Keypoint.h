#ifndef _KEY_
#define _KEY_

#include "Image.h"
#include <vector>

void getMaxes(Image & prev_img, Image & cur_img, Image & next_img, 
	Image & res);

void find_keypoints(std::vector<Image> & differences, 
	std::vector<Image> & keypoint_results);

#endif 