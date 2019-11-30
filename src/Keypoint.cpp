#include "Keypoint.h"
#include "general_helpers.h"
#include <iostream>
#include <assert.h>
#include <cmath>
#include "CycleTimer.h"


#define NUM_REGIONS 16
#define NUM_BINS 8

const int MAX_VAL = 255;

Keypoint::Keypoint(Image & src_x, float grad_thresh_x, float intensity_thresh_x)
	: src(src_x), grad_thresh(grad_thresh_x), 
	intensity_thresh(intensity_thresh_x) {}

void Keypoint::getMaxes(Image & prev_img, Image & cur_img, Image & next_img, 
		Image & res) {
	int r, c, r_offset, c_offset, new_r, new_c, cur_val;
	int is_max, is_min, is_intense;
	int rows = cur_img.rows, cols = cur_img.cols;
	float avg_val = 0;

	// TODO: Possibly try to check if adjacent pixels are edge, if so mark cur 
	// as edge and don't include this as keypoint
	// int extrema[rows * cols];
	// memset(extrema, 0, rows * cols);

	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			cur_val = cur_img.get(r, c);
			is_max = 1;
			is_min = 1;
			// avg_val += abs(cur_val);
			is_intense = (abs(cur_val) > intensity_thresh);

			if (!is_intense) {
				res.data.push_back(0);
				continue;
			}

			// Iterate through a 3x3 window of neighboring pixels
			for (r_offset = -1; (r_offset <= 1) && is_max; r_offset++) {
				new_r = reflect(rows, r + r_offset);
				for (c_offset = -1; (c_offset <= 1) && is_max; c_offset++) {
					new_c = reflect(cols, c + c_offset);

					// don't compare pixel to itself

					assert(0 <= new_r < rows);
					assert(0 <= new_c < cols);
					// compare neighbors of prev, cur, and next scales
					is_max &= (cur_val > prev_img.get(new_r, new_c));
					is_max &= (cur_val > next_img.get(new_r, new_c));

					is_min &= (cur_val < prev_img.get(new_r, new_c));
					is_min &= (cur_val < next_img.get(new_r, new_c));

					if (new_r != r && new_c != c) {
						is_max &= (cur_val > cur_img.get(new_r, new_c));
						is_min &= (cur_val < cur_img.get(new_r, new_c));
					}
				}
			}
			// set as max value if max, else set 0
			// if (is_max) printf("Found max: %d\n", cur_val);
			res.data.push_back(cur_val * (is_max || is_min));
		}
	}
	// printf("Average intensity: %f\n", avg_val / (float)cur_img.data.size());
}


double Keypoint::find_keypoints(std::vector<Image> & differences, 
		std::vector<Image> & keypoint_results) {

	double startTime = CycleTimer::currentSeconds();

	// two keypoint images
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

// void Keypoint::filter_keypoints(std::)

bool Keypoint::is_corner(int grad_x, int grad_y) {
	return (grad_x > grad_thresh) && (grad_y  > grad_thresh);
}

void Keypoint::mark_keypoints(Image & src, std::vector<coord> & keypoints) {
	int row, col;
	// set all values to 0 initially
	src.data.assign(src.rows * src.cols, 0); 
	for (coord rc : keypoints) {
		row = rc.first;
		col = rc.second;
		src.data[row * src.cols + col] = MAX_VAL;
	}
}

double Keypoint::find_corners_gradients(
		const Image & src, std::vector<coord> & keypoints,
		float* grad_magnitudes, float* grad_orientations) {

	double startTime = CycleTimer::currentSeconds();


	int r, c;
	// previous and next adjacent pixel values
	int prev_rp, next_rp, prev_cp, next_cp;
	int grad_x, grad_y;
	int rows = src.rows, cols = src.cols;
	double avg_gradx = 0, avg_grady = 0;
	int count = 0;
	for (r = 0; r < rows; r++) {
		for (c = 0; c < cols; c++) {
			prev_rp = src.get(reflect(rows, r-1), c);
			next_rp = src.get(reflect(rows, r+1), c);
			prev_cp = src.get(r, reflect(cols, c-1));
			next_cp = src.get(r, reflect(cols, c+1));
			grad_x = next_cp - prev_cp;
			grad_y = next_rp - prev_rp;

			grad_magnitudes[r * cols + c] = (
				grad_x * grad_x + grad_y * grad_y);

			grad_orientations[r * cols + c] = (
				atan2f(grad_y, grad_x));
			
			if (abs(grad_x) > 0 && abs(grad_y) > 0) {
				avg_gradx += abs(grad_x);
				avg_grady += abs(grad_y);
				count++;
			}
			if (is_corner(abs(grad_x), abs(grad_y))) {
				coord new_kp(r, c);
				keypoints.push_back(new_kp);
			}
		}
	}
	std::cout << "Average gradients:" << std::endl;
	std::cout << (avg_gradx / (double)count);
	std::cout << ", " << (avg_grady / (double)count);
	std::cout << std::endl;

	double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;
}

int squared_dist(int x, int y) {
	return x * x + y * y;
}

/* For each keypoint, generate 16 separate histograms
*/
void Keypoint::find_keypoint_orientations(std::vector<coord> & keypoints,
	float* grad_magnitudes, float* grad_orientations, 
	std::vector<float> & kp_gradients, int rows, int cols) {

	// 16 x 8, each row corresponds to a (4 x 4) region around the keypoint
	// ordered in row-major format
	// each col represents the 8 
	float grad_histogram[NUM_REGIONS * NUM_BINS];
	int row, col, new_r, new_c, r_offset, c_offset;
	double total_magnitude, weighted_angle_sum;
	int dist;
	float mag, ang, scale;
	for (coord kp : keypoints) {
		row = kp.first;
		col = kp.second;
		total_magnitude = 0;
		weighted_angle_sum = 0;

		// find gradients of pixels in this region around keypoint
		for (r_offset = -4; r_offset <= 4; r_offset++) {
			new_r = row + r_offset;
			if (new_r < 0 || new_r >= rows) continue; 
			for (c_offset = -4; c_offset <= 4; c_offset++) {
				new_c = col + c_offset;
				if (new_c < 0 || new_c >= cols) continue;

				// don't count current keypoint's own gradient
				if(new_r == row && new_c == col) continue;
				
				dist = squared_dist(new_r - row, new_c - col);
				mag = grad_magnitudes[new_r * cols + new_c];
				ang = grad_orientations[new_r * cols + new_c];
				scale = mag / (float)dist;
				total_magnitude += scale;
				weighted_angle_sum += (scale * ang);
			}
		}

		kp_gradients.push_back(weighted_angle_sum / total_magnitude);
	}
}
