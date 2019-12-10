#include <vector>
#include <iostream>
#include <string>
#include <getopt.h>

#include <opencv2/highgui/highgui.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "GaussianBlur.h"
#include "Keypoint.h"
#include "LoG.h"
#include "Image.h"
#include "general_helpers.h"
#include "CycleTimer.h"
// #include <omp.h>


using namespace std;

bool debug = false;
int view_index = 0;
float grad_threshold = 0;
float intensity_threshold = 1;

void find_keypoint_features(Image & src, cv::Mat & result_features, 
    std::vector<cv::KeyPoint> & cv_keypoints);

int main(int argc, char* argv[]){
    float variance = 1;
    if (argc <= 1) {
        cout << "Need to pass in image filepath!" << endl;
        exit(-1);
    }
    std::string img1_path, img2_path;
    if (!get_args(argc, argv, img1_path, img2_path, &variance, &debug, 
        &view_index, &grad_threshold)) {
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

    double total_time_start = CycleTimer::currentSeconds();
    find_keypoint_features(src1, features1, keypoints1);
    find_keypoint_features(src2, features2, keypoints2);
    double total_time_end = CycleTimer::currentSeconds();
    double total_time = total_time_end - total_time_start;
    printf("Total parallel time: %lf\n", total_time);

    // //-- Step 3: Matching descriptor vectors using FLANN matcher
    cv::FlannBasedMatcher matcher;
    std::vector< cv::DMatch > matches;
    matcher.match( features1, features2, matches );

    double max_dist = 0; double min_dist = 100;

    // //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < features1.rows; i++ )
    { double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }

    //-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
    //-- or a small arbitary value ( 0.02 ) in the event that min_dist is very
    //-- small)
    //-- PS.- radiusMatch can also be used here.
    std::vector< cv::DMatch > good_matches;

    for( int i = 0; i < features1.rows; i++ )
    { if( matches[i].distance <= max(2*min_dist, 0.02) )
        { good_matches.push_back( matches[i]); }
    }

    //-- Draw only "good" matches
    drawMatches( src1_mat, keypoints1, src2_mat, keypoints2,
        good_matches, res_output, cv::Scalar::all(-1), cv::Scalar::all(-1),
        vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

    imwrite( "after_blur_result.jpg", res_output);
    cv::namedWindow( "Gray image", CV_WINDOW_AUTOSIZE );
    imshow( "Blurred pikachu!", res_output );
    cv::waitKey(0);
    return 0;
}

void find_keypoint_features(Image & src, cv::Mat & result_features, 
        std::vector<cv::KeyPoint> & cv_keypoints) {
    ///////////////////////////////////// Algorithm BEGIN /////////////////////////////////////
    double SIFT_TIME;

    ///////////////////////////////////// LoG BEGIN /////////////////////////////////////
    // Find Difference of Gaussian Images using LoG
    LoG LoG_processor(src);
    std::vector<Image> octave1_log, octave2_log, octave3_log, octave4_log;

    double log_time_start = CycleTimer::currentSeconds();
    SIFT_TIME = LoG_processor.find_LoG_images(
        octave1_log, octave2_log, octave3_log, octave4_log);
    double log_time_end = CycleTimer::currentSeconds();
    printf("LoG process time: %.3f\n", log_time_end - log_time_start);
    ///////////////////////////////////// LoG END /////////////////////////////////////


    ///////////////////////////////////// Keypoint begin /////////////////////////////////////
    // Find keypoint image-pairs between the DoG images
    Keypoint kp_finder(src, grad_threshold, intensity_threshold);
    std::vector<Image> octave1_kp, octave2_kp, octave3_kp, octave4_kp;

    SIFT_TIME = std::min(SIFT_TIME, kp_finder.find_keypoints(octave1_log, octave1_kp));
    kp_finder.find_keypoints(octave1_log, octave1_kp);
    kp_finder.find_keypoints(octave2_log, octave2_kp);
    kp_finder.find_keypoints(octave3_log, octave3_kp);
    kp_finder.find_keypoints(octave4_log, octave4_kp);

    if (debug) cout << "Storing result" << endl;
    printf("%lu, %d\n", octave1_kp.size(), view_index);

    Image gradx(src.rows, src.cols), grady(src.rows, src.cols);
    std::vector<coord> keypoints1, keypoints2, keypoints3, keypoints4;

    double corner_time_start = CycleTimer::currentSeconds();
    std::vector<PointWithAngle> points_with_angle1, points_with_angle2,
        points_with_angle3, points_with_angle4;
    SIFT_TIME = std::min(SIFT_TIME, kp_finder.find_corners_gradients(
        octave1_kp[view_index], keypoints1, points_with_angle1));
    SIFT_TIME = std::min(SIFT_TIME, kp_finder.find_corners_gradients(
        octave2_kp[view_index], keypoints2, points_with_angle2));
    SIFT_TIME = std::min(SIFT_TIME, kp_finder.find_corners_gradients(
        octave3_kp[view_index], keypoints3, points_with_angle3));
    SIFT_TIME = std::min(SIFT_TIME, kp_finder.find_corners_gradients(
        octave4_kp[view_index], keypoints4, points_with_angle4));
    double corner_time_end = CycleTimer::currentSeconds();
    printf("corner detection time: %.3f s\n", 
        corner_time_end - corner_time_start);

    std::vector<float> kp_gradients;

    double orient_time_start = CycleTimer::currentSeconds();
    std::vector<KeypointFeature> keypoint_features1, keypoint_features2,
        keypoint_features3, keypoint_features4;
    kp_finder.find_keypoint_orientations(keypoints1, points_with_angle1, 
        keypoint_features1, octave1_kp[view_index].rows, octave1_kp[view_index].cols, standard_variances[2]);
    kp_finder.find_keypoint_orientations(keypoints2, points_with_angle2, 
        keypoint_features2, octave2_kp[view_index].rows, octave2_kp[view_index].cols, standard_variances[2]);
    kp_finder.find_keypoint_orientations(keypoints3, points_with_angle3, 
        keypoint_features3, octave3_kp[view_index].rows, octave3_kp[view_index].cols, standard_variances[2]);
    kp_finder.find_keypoint_orientations(keypoints4, points_with_angle4, 
        keypoint_features4, octave4_kp[view_index].rows, octave4_kp[view_index].cols, standard_variances[2]);
    double orient_time_end = CycleTimer::currentSeconds();
    printf("orient time: %.3f s\n", 
        orient_time_end - orient_time_start);

    kp_finder.store_keypoints(keypoint_features1, cv_keypoints, 1, src.cols);
    
    kp_finder.store_features(keypoint_features1, result_features);
}
