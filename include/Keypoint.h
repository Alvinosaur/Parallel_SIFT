#ifndef _KEY_
#define _KEY_

#include "Image.h"
#include "general_helpers.h"

#include <opencv2/highgui/highgui.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <vector>
#include <utility>
#include "mpi.h"

#define FEATURE_VEC_SIZE 128

// discrete bins of histogram as a discrete range of angles
extern const std::vector<float> angle_bins;

class KeypointFeature {
public:
	coord pos;
	int size;
	int angle;
	int magnitude;
	float grad_histogram[FEATURE_VEC_SIZE] = {0};
};

class PointWithAngle {
public:
	coord pos;
	int angle;  // angle in range [0, 360)
	float magnitude;
};

class Keypoint {
    Image src;
	float grad_thresh;
	float intensity_thresh;
	int rank, num_tasks;
    MPI_Request* reqs;
    MPI_Status* stats;

public:
    Keypoint(Image & src_x, float grad_thresh_x, float intensity_thresh_x,
		int rank_x, int num_tasks_x, MPI_Request* reqs_x, MPI_Status* stats_x);
	void getMaxes(Image & prev_img, Image & cur_img, Image & next_img,
		int* result, const range & start_end);

	double find_keypoints(std::vector<Image> & differences, 
		std::vector<Image> & keypoint_results);

	void find_corners_gradients(Image & src, 
		std::vector<coord> & keypoints, int* grad_angs, int* magnitudes);

	bool is_corner(int grad_x, int grad_y);

	void mark_keypoints(Image & src, std::vector<coord> & keypoints);

	void find_keypoint_orientations(std::vector<coord> & keypoints,
		int* grad_angs, int* magnitudes,
		std::vector<KeypointFeature> & final_keypoints, 
		int rows, int cols, int size);

	void store_features(std::vector<KeypointFeature> & kp_features,
		cv::Mat & descriptors);

	void store_keypoints(std::vector<KeypointFeature> & keypoints_src,
		std::vector<cv::KeyPoint> & cv_keypoints_dst, int octave, int cols);
};
#endif 