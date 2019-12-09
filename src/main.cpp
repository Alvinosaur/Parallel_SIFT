#include <vector>
#include <iostream>
#include <string>
#include <getopt.h>
#include "mpi.h"

#include <opencv2/highgui/highgui.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "GaussianBlur.h"
#include "Keypoint.h"
#include "LoG.h"
#include "Image.h"
#include "general_helpers.h"
// #include <omp.h>


using namespace std;

bool debug = false;
int view_index = 0;
float grad_threshold = 0;
float intensity_threshold = 1;

void find_keypoint_features(Image & src, cv::Mat & result_features, 
        std::vector<cv::KeyPoint> & cv_keypoints, int num_tasks, int rank, 
        MPI_Request* reqs, MPI_Status* stats);

int main(int argc, char* argv[]){
    float variance = 1;
    if (argc <= 1) {
        cout << "Need to pass in image filepath!" << endl;
        exit(-1);
    }
    std::string img1_path, img2_path;
    if (!get_args(argc, argv, img1_path, img2_path, &variance, &debug, 
        &view_index, &grad_threshold, &intensity_threshold)) {
        std::cout << "Failed to pass in valid image path with -p1 and -p2";
        std::cout << std::endl;
        exit(-1);
    };

    cv::Mat src1_mat = cv::imread(img1_path.c_str(),
        CV_LOAD_IMAGE_GRAYSCALE);
    cv::Mat src2_mat = cv::imread(img2_path.c_str(),
        CV_LOAD_IMAGE_GRAYSCALE);
    Image src1(src1_mat);
    Image src2(src2_mat);

    cv::Mat res_output, features1, features2;
    std::vector<cv::KeyPoint> keypoints1, keypoints2;

    int num_tasks, tag, rank;

    // Init MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // two requests per task: one for recv, one for send
    // arranged as [0, num_tasks) for receive, [num_tasks, num_tasks*2) for send
    MPI_Request reqs[num_tasks * 2];
    MPI_Status stats[num_tasks * 2];

    find_keypoint_features(src1, features1, keypoints1, num_tasks, rank,
        reqs, stats);
    // find_keypoint_features(src2, features2, keypoints2, num_tasks, rank,
    //     reqs, stats);

    MPI_Finalize();

    // //-- Step 3: Matching descriptor vectors using FLANN matcher
    // cv::FlannBasedMatcher matcher;
    // std::vector< cv::DMatch > matches;
    // matcher.match( features1, features2, matches );

    // double max_dist = 0; double min_dist = 100;

    // // //-- Quick calculation of max and min distances between keypoints
    // for( int i = 0; i < features1.rows; i++ )
    // { double dist = matches[i].distance;
    //     if( dist < min_dist ) min_dist = dist;
    //     if( dist > max_dist ) max_dist = dist;
    // }

    // //-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
    // //-- or a small arbitary value ( 0.02 ) in the event that min_dist is very
    // //-- small)
    // //-- PS.- radiusMatch can also be used here.
    // std::vector< cv::DMatch > good_matches;

    // for( int i = 0; i < features1.rows; i++ )
    // { if( matches[i].distance <= max(2*min_dist, 0.02) )
    //     { good_matches.push_back( matches[i]); }
    // }

    // //-- Draw only "good" matches
    // drawMatches( src1_mat, keypoints1, src2_mat, keypoints2,
    //     good_matches, res_output, cv::Scalar::all(-1), cv::Scalar::all(-1),
    //     vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

    // imwrite( "after_blur_result.jpg", res_output);
    // imshow( "Blurred pikachu!", res_output );
    // cv::waitKey(0);
    return 0;
}

void find_keypoint_features(Image & src, cv::Mat & result_features, 
        std::vector<cv::KeyPoint> & cv_keypoints, int num_tasks, int rank, 
        MPI_Request* reqs, MPI_Status* stats) {
    ///////////////////////////////////// Algorithm BEGIN /////////////////////////////////////
    double SIFT_TIME = 0;

    ///////////////////////////////////// LoG BEGIN /////////////////////////////////////
    // Find Difference of Gaussian Images using LoG
    LoG LoG_processor(src, num_tasks, rank, reqs, stats);
    std::vector<Image> octave1_log, octave2_log, octave3_log, octave4_log;

    double log_time = 0;
    log_time = LoG_processor.find_LoG_images(
        octave1_log, octave2_log, octave3_log, octave4_log);
    printf("LoG process time: %.3f s\n", log_time);
    
    ///////////////////////////////////// LoG END /////////////////////////////////////


    ///////////////////////////////////// Keypoint begin /////////////////////////////////////
    // Find keypoint image-pairs between the DoG images
    Keypoint kp_finder(src, grad_threshold, intensity_threshold,
        rank, num_tasks, reqs, stats);
    std::vector<Image> octave1_kp, octave2_kp, octave3_kp, octave4_kp;

    double find_kp_time = 0;
    find_kp_time += kp_finder.find_keypoints(octave1_log, octave1_kp);
    find_kp_time += kp_finder.find_keypoints(octave1_log, octave1_kp);
    find_kp_time += kp_finder.find_keypoints(octave2_log, octave2_kp);
    find_kp_time += kp_finder.find_keypoints(octave3_log, octave3_kp);
    find_kp_time += kp_finder.find_keypoints(octave4_log, octave4_kp);
    printf("keypoint_find for ALL octaves time: %.3f s\n", find_kp_time);

    if (rank == 0) {
        cv::Mat res_output;
        octave1_kp[0].store_opencv(res_output);
        imshow( "Blurred pikachu!", res_output );
        cv::waitKey(0);
    }

    // Image gradx(src.rows, src.cols), grady(src.rows, src.cols);
    // std::vector<coord> keypoints;

    // std::vector<PointWithAngle> points_with_angle;
    // SIFT_TIME = kp_finder.find_corners_gradients(
    //     octave1_kp[view_index], keypoints, points_with_angle);
    // printf("corner detection for octave1 time: %.3f s\n", SIFT_TIME);

    // std::vector<float> kp_gradients;

    // std::vector<KeypointFeature> keypoint_features;
    // kp_finder.find_keypoint_orientations(keypoints, points_with_angle, 
    //     keypoint_features, src.rows, src.cols, standard_variances[2]);

    // kp_finder.store_keypoints(keypoint_features, cv_keypoints, 1, src.cols);
    
    // kp_finder.store_features(keypoint_features, result_features);


}
