#include "Keypoint.h"
#include "general_helpers.h"
#include <iostream>
#include <assert.h>
#include <cmath>
#include "CycleTimer.h"


#define NUM_REGIONS 16
#define NUM_REGIONS_SQRT 4
#define NUM_BINS 8
#define RAD_TO_DEG ((float)180 / M_PI)

const int MAX_VAL = 255;

Keypoint::Keypoint(Image & src_x, float grad_thresh_x, float intensity_thresh_x)
	: src(src_x), grad_thresh(grad_thresh_x), 
	intensity_thresh(intensity_thresh_x) {}

void Keypoint::getMaxes(Image & prev_img, Image & cur_img, Image & next_img, 
		Image & res) {
	int r = 0, c, r_offset, c_offset, new_r, new_c, cur_val;
	int is_max, is_min, is_intense;
	int rows = cur_img.rows, cols = cur_img.cols;
	float avg_val = 0;

	// TODO: Possibly try to check if adjacent pixels are edge, if so mark cur 
	// as edge and don't include this as keypoint
	// int extrema[rows * cols];
	// memset(extrema, 0, rows * cols);


    #pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < rows * cols; i++) {

		int c = i % cols;
		if (c == cols -1 && r != rows -1) r++;

		cur_val = cur_img.get(r, c);
		is_max = 1;
		is_min = 1;
		// avg_val += abs(cur_val);
		is_intense = (abs(cur_val) > intensity_thresh);

		// printf("%d \n", is_intense);
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

	printf("Average intensity: %f\n", avg_val / (float)cur_img.data.size());
}


double Keypoint::find_keypoints(std::vector<Image> & differences, 
		std::vector<Image> & keypoint_results) {

	double startTime = CycleTimer::currentSeconds();

	// two keypoint images
	Image kp1(differences[0].rows, differences[0].cols);
	Image kp2(differences[0].rows, differences[0].cols);

	#pragma omp parallel 
    {
        #pragma omp single
        {
            #pragma omp task
			getMaxes(differences[0], differences[1], differences[2], kp1);
			#pragma omp task
			getMaxes(differences[1], differences[2], differences[3], kp2);
		}
	}


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

double Keypoint::mark_keypoints(Image & src, std::vector<coord> & keypoints) {

	double startTime = CycleTimer::currentSeconds();

	int row, col;
	// set all values to 0 initially
	src.data.assign(src.rows * src.cols, 0); 

	for (int i = 0; i < keypoints.size(); i++) {
		coord rc = keypoints[i];
		row = rc.first;
		col = rc.second;
		src.data[row * src.cols + col] = MAX_VAL;
	}

	double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;
}

double Keypoint::find_corners_gradients(
		const Image & src, std::vector<coord> & keypoints,
		std::vector<PointWithAngle> & points_with_angle) {
	double startTime = CycleTimer::currentSeconds();
	int r = 0, c = 0;
	// previous and next adjacent pixel values
	int prev_rp, next_rp, prev_cp, next_cp;
	int grad_x, grad_y;
	int rows = src.rows, cols = src.cols;
	double avg_gradx = 0, avg_grady = 0;
	int count = 0;


	// #pragma omp parallel shared(points_with_angle)
	for (int i = 0; i < rows * cols; i++) {
		int c = i % cols;
		if (c == cols -1 && r != rows -1) r++;
		prev_rp = src.get(reflect(rows, r-1), c);
		next_rp = src.get(reflect(rows, r+1), c);
		prev_cp = src.get(r, reflect(cols, c-1));
		next_cp = src.get(r, reflect(cols, c+1));
		grad_x = next_cp - prev_cp;
		grad_y = next_rp - prev_rp;

		PointWithAngle newp;
		newp.angle = atan2f(grad_y, grad_x);
		newp.magnitude = grad_x * grad_x + grad_y * grad_y;
		coord pos(r, c);
		newp.pos = pos;
		points_with_angle.push_back(newp);

		if (is_corner(abs(grad_x), abs(grad_y))) {
			coord new_kp(r, c);
			keypoints.push_back(new_kp);
		}
	}

	double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;
}

int squared_dist(int x, int y) {
	return x * x + y * y;
}

double dist_norm(int width, int height) {
	assert(width % 2 == 0);
	assert(height % 2 == 0);
	int half_w = width/2;
	int half_h = height/2;
	int x, y;
	double total = 0;
	for (x = -half_w; x <= half_w; x++)  {
		for (y = -half_h; y <= half_h; y++) {
			total += (double)squared_dist(x, y);
		}
	}
	return total;
}

// discrete weighting pixel row and column distance
double inv_distance_weight(int r_offset, int c_offset, double norm) {
	return squared_dist(r_offset, c_offset) / norm;
}

int offset_to_region(int r_offset, int c_offset) {
	return r_offset * NUM_REGIONS_SQRT + c_offset;
}

/*
 * Return a bin index in range [0, NUM_BINS) given some angle.
 * Converts radian angle into degrees within the range [0, 360) 
 * and finds the index corresponding to that angle.
 */
int ang_to_bin(float ang_rad) {
	int ang_deg = (int)(ang_rad * RAD_TO_DEG);
	int bin = (int)(((float)(ang_deg % 360) / 360.0) * NUM_BINS);
	assert(0 <= bin && bin < NUM_BINS);
	return bin;
}

/* For each keypoint, generate 16 separate histograms
For each of the 16 4x4 regions to overall cover entire 16 x 16 window:
// each row for a region, each column for a histogram bin
float gradient_hist[16][8];
// sum of weights for each histogram, use this to normalize each histogram
float gradient_hist_norm[16];
// apply kernel to each adjacent neighbor weigh by distance
gaussian_kernel = [1, 3, 6, 9, 6, 3, 1]
create a temporary 16 x 16 image around this keypoint
Apply gaussian blur to it by scaling pixel values down based
on their distance from keypoint
// iterate through this temporary 16 x 16 image
for r_offset = -8; r_offset <= 8; r_offset++
	for c_offset = -8; c_offset <= 8; c_offset++
		// example:  -8, -8 corresponds to region 0
		region_idx = get_region_id(r_offset, c_offset)
		grad_ang = 
		hist_bin_idx = get_bin_idx(grad_ang)
		grad_mag = 
		dist_from_kp = get_dist(r_offset, c_offset)
		scale = grad_mag
		gradient_hist_norm[region_idx] += scale
		gradient_hist[region_idx][hist_bin_idx] +=
		
for i, j in rows, cols of gradient_hist
	// make 128 orientation feature vec relative to keypoint orientation
	// so rotation invariant
	gradient_hist[i][j] -= keypoint_grad_ang
*/
void Keypoint::find_keypoint_orientations(std::vector<coord> & keypoints,
		std::vector<PointWithAngle> all_points, 
		std::vector<PointWithAngle> keypoints_with_angles, 
		int rows, int cols) {

	// 16 x 8, each row corresponds to a (4 x 4) region around the keypoint
	// ordered in row-major format
	// each col represents the 8 
	float grad_histogram[NUM_REGIONS][NUM_BINS];
	int row, col, new_r, new_c, r_offset, c_offset, region_idx;
	double total_magnitude, weighted_angle_sum;
	double inv_dist;
	float mag, ang, scale;
	double normalizer = dist_norm(NUM_BINS, NUM_BINS);
	for (int i = 0; i < keypoints.size(); i++) {
		coord kp = keypoints[i];
		row = kp.first;
		col = kp.second;
		total_magnitude = 0;
		weighted_angle_sum = 0;

		// used to apply gaussian 
		// Image neighbors(8, 8);

		// find gradients of pixels in this region around keypoint
		for (r_offset = -4; r_offset <= 4; r_offset++) {
			new_r = row + r_offset;
			if (new_r < 0 || new_r >= rows) continue; 
			for (c_offset = -4; c_offset <= 4; c_offset++) {
				new_c = col + c_offset;
				if (new_c < 0 || new_c >= cols) continue;
				
				inv_dist = inv_distance_weight(r_offset, c_offset, normalizer);
				ang = all_points[new_r * cols + new_c].angle;
				mag = all_points[new_r * cols + new_c].magnitude;

				scale = mag * inv_dist;
				total_magnitude += scale;
				region_idx = offset_to_region(r_offset, c_offset);
				grad_histogram[region_idx][ang_to_bin(ang)] += scale;
			}
		}
	}
}