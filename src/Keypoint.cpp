#include "Keypoint.h"
#include "general_helpers.h"
#include <iostream>
#include <assert.h>
#include <cmath>
#include "CycleTimer.h"
#include "mpi.h"


#define NUM_REGIONS 16
#define REGION_SIZE 4
#define NUM_BINS 8
#define REGION_LEN 8
#define RAD_TO_DEG ((float)180 / M_PI)

#define MASTER 0

const int MAX_VAL = 255;

Keypoint::Keypoint(Image & src_x, float grad_thresh_x, float intensity_thresh_x,
	int rank_x, int num_tasks_x, MPI_Request* reqs_x, MPI_Status* stats_x)
	: src(src_x), grad_thresh(grad_thresh_x), 
	intensity_thresh(intensity_thresh_x),
	rank(rank_x), num_tasks(num_tasks_x), reqs(reqs_x), stats(stats_x){}


void Keypoint::getMaxes(Image & prev_img, Image & cur_img, Image & next_img,
		int* result, const range & start_end) {
	int k, r, c, r_offset, c_offset, new_r, new_c, cur_val;
	int start = start_end.first, end = start_end.second;
	int is_max, is_min, is_intense;
	int rows = cur_img.rows, cols = cur_img.cols;

	for (k = start; k < end; k++) {
		r = k / cols;
		c = k % cols;
		cur_val = cur_img.get(r, c);
		is_max = 1;
		is_min = 1;
		// avg_val += abs(cur_val);
		is_intense = (abs(cur_val) > intensity_thresh);

		if (!is_intense) {
			// assumes result array already memset to 0's, so just continue
			continue;
		}

		// Iterate through a 3x3 window of neighboring pixels
		for (r_offset = -1; (r_offset <= 1) && is_max; r_offset++) {
			new_r = reflect(rows, r + r_offset);
			for (c_offset = -1; (c_offset <= 1) && is_max; c_offset++) {
				new_c = reflect(cols, c + c_offset);

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
		result[k] = cur_val * (is_max || is_min);
	}
}


double Keypoint::find_keypoints(std::vector<Image> & differences, 
		std::vector<Image> & keypoint_results) {

	double startTime = CycleTimer::currentSeconds();

	// two keypoint images
	int rows = differences[0].rows, cols = differences[0].cols;
	// use calloc to ensure all values 0 initially b/c ignored pixels have val = 0
	// can't static init b/c variable size depends on rows * cols
	int* kp1 = (int*)calloc(rows*cols, sizeof(int));
	int* kp2 = (int*)calloc(rows*cols, sizeof(int));

	const int kp1_id = 1;
	const int kp2_id = 2;

	std::vector<range> assignments;
	allocate_work_pix_mpi(rows, cols, num_tasks, assignments);

	// use separate ID's to let these two processes run without any barrier
	getMaxes(differences[0], differences[1], differences[2], 
		kp1, assignments[rank]);
	send_to_others(kp1, reqs, rank, assignments[rank], kp1_id, num_tasks);
    receive_from_others(kp1, reqs, assignments, rank, kp1_id);
	getMaxes(differences[1], differences[2], differences[3], 
		kp2, assignments[rank]);
	send_to_others(kp2, reqs, rank, assignments[rank], kp2_id, num_tasks);
    receive_from_others(kp2, reqs, assignments, rank, kp2_id);

    mpi_barrier(rank, num_tasks, reqs, stats);

	// store into keypoints images, then free temporary arrays
	Image kp1_img(rows, cols, kp1);
	Image kp2_img(rows, cols, kp2);
	free(kp1);
	free(kp2); 
	keypoint_results.push_back(kp1_img);
	keypoint_results.push_back(kp2_img);

	double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;
}


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

int rad_to_deg(float ang_rad) {
	int ang_deg = (int)(ang_rad * RAD_TO_DEG);
	// can't use modulo operator here since isn't python and won't
	// guarantee positive result
	if (ang_deg < 0) ang_deg += 360;
	return ang_deg;
}

void Keypoint::find_corners_gradients(
		Image & src, std::vector<coord> & keypoints,
		int* grad_angs, int* magnitudes) {
	int r, c, k;
	// previous and next adjacent pixel values
	int prev_rp, next_rp, prev_cp, next_cp;
	int grad_x, grad_y;
	int rows = src.rows, cols = src.cols;
	int kp_count = 0;
	int kp_locs[rows * cols];  // potentially sparse
	int kp_counts[num_tasks];  // how many keypoints each task sent

	enum msg_id { GRAD=0, MAG=1, KP=2 };

	std::vector<range> assignments;
	allocate_work_pix_mpi(rows, cols, num_tasks, assignments);
	int start = assignments[rank].first, end = assignments[rank].second;

	MPI_Request mag_reqs[num_tasks * 2];
	MPI_Request ang_reqs[num_tasks * 2];

	for (k = start; k < end; k++) {
		r = k / cols;
		c = k % cols;
		prev_rp = src.get(reflect(rows, r-1), c);
		next_rp = src.get(reflect(rows, r+1), c);
		prev_cp = src.get(r, reflect(cols, c-1));
		next_cp = src.get(r, reflect(cols, c+1));
		grad_x = next_cp - prev_cp;
		grad_y = next_rp - prev_rp;

		printf("gradx, grady: %f, %f\n", grad_x, grad_y);

		PointWithAngle newp;
		newp.angle = rad_to_deg(atan2f(grad_y, grad_x));
		grad_angs[k] = newp.angle;

		newp.magnitude = grad_x * grad_x + grad_y * grad_y;
		magnitudes[k] = newp.magnitude;

		if (is_corner(abs(grad_x), abs(grad_y))) {
			coord new_kp(r, c);
			keypoints.push_back(new_kp);
			kp_locs[start + kp_count] = r * c;
			kp_count++;
			
		}
	}

	// transfer all keypoints first so can receive them first without waiting
	// for other data
	// tasks will share part of array that contains valid keypoint locations
	// so not full task assignment range, only up to number of keypoints found
	range found_kp_range;
	found_kp_range.first = start;
	found_kp_range.second = start + kp_count;
	send_to_others(kp_locs, reqs, rank, found_kp_range, KP, num_tasks);

	// share gradient angles, magnitudes, and keypoint positions
	send_to_others(magnitudes, mag_reqs, rank, assignments[rank], MAG, 
		num_tasks);
    receive_from_others(magnitudes, mag_reqs, assignments, rank, MAG);

	send_to_others(grad_angs, ang_reqs, rank, assignments[rank], GRAD, 
		num_tasks);
    receive_from_others(grad_angs, ang_reqs, assignments, rank, GRAD);

	// need to do post-processing on this array
	range other_task_r;
	int max_kp;
	for (int task = 0; task < num_tasks; task++) {
        if (task == rank) continue;  // don't need to request from self
        other_task_r = assignments[task];
        max_kp = other_task_r.second - other_task_r.first;
		// Need blocking receive to track the number of items received
		MPI_Recv(&kp_locs[other_task_r.first], max_kp, MPI_INT, task, KP, 
			MPI_COMM_WORLD, &stats[task]);
		MPI_Get_count(&stats[task], MPI_INT, &kp_counts[task]);

		// now store all these in local keypoints vector
		kp_count = kp_counts[task];
		for (int kp_i = 0; kp_i < kp_count; kp_i++) {
			// locations of keypoints, stored starting at offset
			k = kp_locs[other_task_r.first + kp_i];
			r = k / cols;
			c = k % cols;
			coord pos(r, c);
			keypoints.push_back(pos);
		}
    }

	// make sure other non-blocking communication has finished
	mpi_barrier(rank, num_tasks, mag_reqs, NULL);
	mpi_barrier(rank, num_tasks, ang_reqs, NULL);

	// if (rank == MASTER) {
	// 	for (int i = 0; i < rows * cols; i++) {
	// 		printf("Mag: %f, Ang: %f\n", magnitudes[i], grad_angs[i]);
	// 	}
	// }
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

void Keypoint::find_keypoint_orientations(std::vector<coord> & keypoints,
		int* grad_angs, int* magnitudes,
		std::vector<KeypointFeature> & final_keypoints, 
		int rows, int cols, int size) {

	int row, col, new_r, new_c, r_offset, c_offset, region_idx;
	int ang, ang_bin;
	double total_magnitude, weighted_angle_sum;
	double inv_dist;
	double mag, scale;
	double scaled_ang;
	double normalizer = dist_norm(NUM_REGIONS, NUM_REGIONS);

	// find weighted average of all adjacent angles and normalize
	for (coord kp : keypoints) {
		row = kp.first;
		col = kp.second;
		total_magnitude = 0;
		weighted_angle_sum = 0;
		KeypointFeature kp_feature;
		kp_feature.size = size;
		kp_feature.magnitude = magnitudes[row * cols + col];
		float weights[FEATURE_VEC_SIZE] = {0};

		// find gradients of pixels in this region around keypoint
		for (r_offset = -REGION_LEN; r_offset < REGION_LEN; r_offset++) {
			new_r = reflect(rows, row + r_offset);
			for (c_offset = -REGION_LEN; c_offset < REGION_LEN; c_offset++) {
				new_c = reflect(cols, col + c_offset);
				
				inv_dist = inv_distance_weight(r_offset, c_offset, normalizer);
				ang = grad_angs[new_r * cols + new_c];
				ang_bin = ang_to_bin(ang);
				mag = magnitudes[new_r * cols + new_c];

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
}

void Keypoint::store_features(std::vector<KeypointFeature> & kp_features,
		cv::Mat & descriptors) {
	int rows = kp_features.size();
	int cols = FEATURE_VEC_SIZE;
	descriptors.create(rows, cols, CV_32F);
    int idx;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            descriptors.at<float>(r, c) = kp_features[r].grad_histogram[c];
        }
    }
}

void Keypoint::store_keypoints(std::vector<KeypointFeature> & keypoints_src,
		std::vector<cv::KeyPoint> & cv_keypoints_dst, int octave, int cols) {
	int id;
	for (KeypointFeature kp : keypoints_src) {
		cv::Point2f pos;
		pos.x = kp.pos.second;
		pos.y = kp.pos.first;
		id = pos.y * cols + pos.x;  // row * cols_per_row + col
		cv::KeyPoint cv_kp(pos, kp.size, kp.angle, kp.magnitude, id);
		cv_keypoints_dst.push_back(cv_kp);
	}
}