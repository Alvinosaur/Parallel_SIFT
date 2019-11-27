#include "Keypoint.h"
#include "general_helpers.h"
#include <iostream>
#include <assert.h>
#include "CycleTimer.h"

const int MAX_VAL = 255;

Keypoint::Keypoint(Image & src_x, float grad_thresh_x) : 
		src(src_x), grad_thresh(grad_thresh_x) {}

void Keypoint::getMaxes(Image & prev_img, Image & cur_img, Image & next_img, 
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
			res.data.push_back(cur_val * is_max);
		}
	}
}


double Keypoint::find_keypoints(std::vector<Image> & differences, 
		std::vector<Image> & keypoint_results) {
	// two keypoint images
    double startTime = CycleTimer::currentSeconds();

	Image kp1(differences[0].rows, differences[0].cols);
	Image kp2(differences[0].rows, differences[0].cols);
	getMaxes(differences[0], differences[1], differences[2], kp1);
	getMaxes(differences[1], differences[2], differences[3], kp2);

	keypoint_results.push_back(kp1);
	keypoint_results.push_back(kp2);

	double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;
}



bool Keypoint::is_edge(int grad_x, int grad_y) {
	return (grad_x > grad_thresh) && (grad_y  > grad_thresh);
}

double Keypoint::find_xy_gradient(Image & remove_target, Image & grad_x_res, 
		Image & grad_y_res, bool is_remove, std::vector<coord> keypoints) {
	int r, c;
	// previous and next adjacent pixel values
	double startTime = CycleTimer::currentSeconds();

	int prev_rp, next_rp, prev_cp, next_cp;
	int grad_x, grad_y;
	int rows = remove_target.rows, cols = remove_target.cols;
	double avg_gradx = 0, avg_grady = 0;
	int count = 0;
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			prev_rp = remove_target.get(reflect(rows, r-1), c);
			next_rp = remove_target.get(reflect(rows, r+1), c);
			prev_cp = remove_target.get(r, reflect(cols, c-1));
			next_cp = remove_target.get(r, reflect(cols, c+1));
			grad_x = next_cp - prev_cp;
			grad_y = next_rp - prev_rp;

			grad_x_res.data.push_back(abs(grad_x));
			grad_y_res.data.push_back(abs(grad_y));
			if (abs(grad_x) > 0 || abs(grad_y) > 0) {
				// printf("grad: %d, %d\n", grad_x, grad_y);
				avg_gradx += grad_x;
				avg_grady += grad_y;
				count++;
			}
			if (is_remove && (!is_edge(abs(grad_x), abs(grad_y)))) {
				remove_target.set(r, c, 0);
				// printf("Removed a keypoint(%d, %d) with grad(%d, %d)\n", 
					// r, c, grad_x, grad_y);
			} else {
				coord new_kp(r, c);
				keypoints.push_back(new_kp);
			}
		}
	}
	std::cout << (avg_gradx / (double)count);
	std::cout << ", " << (avg_grady / (double)count);
	std::cout << std::endl;

	double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;
}

// void Keypoint::remove_non_keypoints(Image & target, Image & grad_x, 
// 		Image & grad_y) {
// 	int r, c;
// 	int rows = src.rows, cols = src.cols;
// 	for (r = 0; r < rows; r++) {
// 		for (c = 0; c < cols; c++) {
// 			if (!is_edge(grad_))
// 		}
// 	}
// }


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