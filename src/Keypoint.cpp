#include "Keypoint.h"
#include "general_helpers.h"
#include <iostream>
#include <assert.h>
#include <cmath>
#include "CycleTimer.h"


#define NUM_REGIONS 16
#define REGION_SIZE 4
#define NUM_BINS 8
#define REGION_LEN 8
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

	

    #pragma omp parallel
    {
    	std::vector<int> private_data;
    	#pragma omp for schedule(dynamic) nowait
    	for (int i = 0; i < rows * cols; i++) {

			c = i % cols;
			if (c == cols -1 && r != rows -1) r++;

			cur_val = cur_img.get(r, c);
			is_max = 1;
			is_min = 1;
			// avg_val += abs(cur_val);
			is_intense = (abs(cur_val) > intensity_thresh);

			// printf("%d \n", is_intense);
			if (!is_intense) {
				// res.data.push_back(0);
				private_data.push_back(0);
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

			private_data.push_back(cur_val * (is_max || is_min));
			
		}
		#pragma omp critical
		res.data.insert(res.data.end(), private_data.begin(), private_data.end());
    }
	
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
            {
            	getMaxes(differences[0], differences[1], differences[2], kp1);

            }
			#pragma omp task
			{
				getMaxes(differences[1], differences[2], differences[3], kp2);

			}
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

	#pragma omp parallel for schedule(static)
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


int rad_to_deg(float ang_rad) {
	int ang_deg = (int)(ang_rad * RAD_TO_DEG);
	// can't use modulo operator here since isn't python and won't
	// guarantee positive result
	if (ang_deg < 0) ang_deg += 360;
	return ang_deg;
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


	for (int i = 0; i < rows * cols; i++) {
		c = i % cols;
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
	for (x = -half_w; x < half_w; x++)  {
		for (y = -half_h; y < half_h; y++) {
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
	int r = (r_offset + REGION_LEN) / REGION_SIZE;
	int c = (c_offset + REGION_LEN) / REGION_SIZE;
	int index = r * REGION_SIZE + c;
	assert(0 <= index && index < NUM_REGIONS);
	return index;
}

int ang_to_bin(int ang_deg) {
	int bin = (int)((float)(ang_deg / 360.0) * NUM_BINS);
	assert(0 <= bin && bin < NUM_BINS);
	return bin;
}

void normalize_vector(float* vec, int N) {
	double total = 0;
	for (int i = 0; i < N; i++) {
		total += vec[i] * vec[i];
	}
	total = sqrt(total);
	for (int i = 0; i < N; i++) {
		vec[i] = vec[i] / total;
	}
}

double Keypoint::find_keypoint_orientations(std::vector<coord> & keypoints,
		std::vector<PointWithAngle> & all_points, 
		std::vector<KeypointFeature> & final_keypoints, 
		int rows, int cols, int size) {

	double startTime = CycleTimer::currentSeconds();


	int row, col, new_r, new_c, r_offset, c_offset, region_idx;
	int ang, ang_bin;
	double total_magnitude, weighted_angle_sum;
	double inv_dist;
	double mag, scale;
	double scaled_ang;
	double normalizer = dist_norm(NUM_REGIONS, NUM_REGIONS);

	// find weighted average of all adjacent angles and normalize
	for (int i = 0; i < keypoints.size(); i++){
		coord kp = keypoints[i];
		row = kp.first;
		col = kp.second;
		total_magnitude = 0;
		weighted_angle_sum = 0;
		KeypointFeature kp_feature;
		kp_feature.size = size;
		kp_feature.magnitude = all_points[row * cols + col].magnitude;
		float weights[FEATURE_VEC_SIZE] = {0};

		// used to apply gaussian 
		// Image neighbors(8, 8);

		// find gradients of pixels in this region around keypoint
		for (r_offset = -REGION_LEN; r_offset < REGION_LEN; r_offset++) {
			new_r = reflect(rows, row + r_offset);
			for (c_offset = -REGION_LEN; c_offset < REGION_LEN; c_offset++) {
				new_c = reflect(cols, col + c_offset);
				
				inv_dist = inv_distance_weight(r_offset, c_offset, normalizer);
				ang = all_points[new_r * cols + new_c].angle;
				ang_bin = ang_to_bin(ang);
				mag = all_points[new_r * cols + new_c].magnitude;

				scale = mag * inv_dist;
				scaled_ang = scale * ang;
				total_magnitude += scale;
				weighted_angle_sum += scaled_ang;
				region_idx = offset_to_region(r_offset, c_offset);
				kp_feature.grad_histogram[region_idx*NUM_BINS + ang_bin] += (
					scaled_ang);
				weights[region_idx*NUM_BINS + ang_bin] += scale;
			}
		}
		
		kp_feature.angle = (int)(weighted_angle_sum / total_magnitude);
		kp_feature.pos = kp;

		// normalize each bin by total weights to get avg angle
		for (int i = 0; i < FEATURE_VEC_SIZE; i++) {
			if (weights[i] == 0) continue;
			float weighted_bin_sum = kp_feature.grad_histogram[i];
			
			weighted_bin_sum /= weights[i];
			
			weighted_bin_sum -= kp_feature.angle;
			if (weighted_bin_sum < 0) weighted_bin_sum += 360;
			kp_feature.grad_histogram[i] = weighted_bin_sum;
		}
		normalize_vector(kp_feature.grad_histogram, FEATURE_VEC_SIZE);
		final_keypoints.push_back(kp_feature);
	}


	double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;
}


double Keypoint::store_features(std::vector<KeypointFeature> & kp_features,
		cv::Mat & descriptors) {

	double startTime = CycleTimer::currentSeconds();

	int rows = kp_features.size();
	int cols = FEATURE_VEC_SIZE;
	descriptors.create(rows, cols, CV_32F);
    int idx;
    int c, r;

    for (int i = 0; i < rows * cols; i++) {
		c = i % cols;
		if (c == cols -1 && r != rows -1) r++;
        descriptors.at<float>(r, c) = kp_features[r].grad_histogram[c];
    }

    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;
}

double Keypoint::store_keypoints(std::vector<KeypointFeature> & keypoints_src,
		std::vector<cv::KeyPoint> & cv_keypoints_dst, int octave, int cols) {

	double startTime = CycleTimer::currentSeconds();

	int id;
	// printf("keypoints in store: %d\n", keypoints_src.size());

	for (int i = 0; i < keypoints_src.size(); i++) {
		KeypointFeature kp = keypoints_src[i];
		cv::Point2f pos;
		pos.x = kp.pos.second;
		pos.y = kp.pos.first;
		id = pos.y * cols + pos.x;  // row * cols_per_row + col
		cv::KeyPoint cv_kp(pos, kp.size, kp.angle, kp.magnitude, id);
		cv_keypoints_dst.push_back(cv_kp);
	}

	double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;
}