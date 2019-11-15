#include "Keypoint.h"
#include "general_helpers.h"
#include <iostream>
#include <assert.h>

const int MAX_VAL = 255;

void getMaxes(Image & prev_img, Image & cur_img, Image & next_img, 
		Image & res) {
	int r, c, r_offset, c_offset, new_r, new_c, cur_val, is_max;
	int rows = cur_img.rows, cols = cur_img.cols;
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			cur_val = cur_img.get(r, c);
			is_max = 1;

			// Iterate through a 3x3 window of neighboring pixels
			for (r_offset = -1; (r_offset <= 1) && is_max; r_offset++) {
				new_r = reflect(rows, r + r_offset);
				for (c_offset = -1; (c_offset <= 1) && is_max; c_offset++) {
					new_c = reflect(cols, c + c_offset);

					assert(0 <= new_r < rows);
					assert(0 <= new_c < cols);
					// compare neighbors of prev, cur, and next scales
					is_max &= (cur_val >= prev_img.get(new_r, new_c));
					is_max &= (cur_val >= cur_img.get(new_r, new_c));
					is_max &= (cur_val >= next_img.get(new_r, new_c));
				}
			}
			// set as max value if max, else set 0
			res.data.push_back(MAX_VAL * is_max);
		}
	}
}


void find_keypoints(std::vector<Image> & differences, 
		std::vector<Image> & keypoint_results) {
	// two keypoint images
	Image kp1(differences[0].rows, differences[0].cols);
	Image kp2(differences[0].rows, differences[0].cols);
	getMaxes(differences[0], differences[1], differences[2], kp1);
	getMaxes(differences[1], differences[2], differences[3], kp2);
	keypoint_results.push_back(kp1);
	keypoint_results.push_back(kp2);
}

// void remove_keypoints(float threshhold, std::vector<Image> & allScales, std::vector<Point> & keypoint_list_total) {

// 	for (int i = 0; i < keypoint_list_total.size(); i++) {

// 		Image curScale = allScales[keypoint_list_total[i].scale_index];
// 		int curRow keypoint_list[i].row , curCol = keypoint_list[i].col;

// 		float dy = curScale.get(curRow - 1, curCol) - curScale.get(curRow + 1, curCol);
// 		float dx = curScale.get(curRow, curCol - 1) - curScale.get(curRow, curCol + 1);


// 		if (dy < threshhold | dx < threshhold) {
// 			keypoint_list_total.erase(keypoint_list_total.begin() + i);
// 		}

// 	}

// }