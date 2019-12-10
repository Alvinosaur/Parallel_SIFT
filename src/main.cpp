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
#include <omp.h>


using namespace std;

bool debug = false;
int view_index = 0;
float grad_threshold = 7.0;
float intensity_threshold = 0;

double find_keypoint_features(Image & src, cv::Mat & result_features, 
    std::vector<cv::KeyPoint> & cv_keypoints, int imgID);

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

    double TIME = 0.0;

    cv::Mat src1_mat = cv::imread(img1_path.c_str(),
        CV_LOAD_IMAGE_GRAYSCALE);
    cv::Mat src2_mat = cv::imread(img2_path.c_str(),
        CV_LOAD_IMAGE_GRAYSCALE);
    Image src1(src1_mat);
    Image src2(src2_mat);

    cv::Mat res_output, features1, features2;
    std::vector<cv::KeyPoint> keypoints1, keypoints2;
    TIME += find_keypoint_features(src1, features1, keypoints1, 1);
    TIME += find_keypoint_features(src2, features2, keypoints2, 2);

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

    printf("OVERALL TIME:                          %.3f ms\n", 1000.f * TIME);

    imwrite( "after_blur_result.jpg", res_output);
    imshow( "Blurred pikachu!", res_output );
    cv::waitKey(0);
    return 0;
}

double find_keypoint_features(Image & src, cv::Mat & result_features, 
        std::vector<cv::KeyPoint> & cv_keypoints, int imgID) {
    ///////////////////////////////////// Algorithm BEGIN /////////////////////////////////////
    double SIFT_TIME = 50000.;
    double TOTAL_TIME = 0.0;
    double ADD_TIME = 0.0;

    printf("/////////////////////////////////////////////////////////////////////////////////\n");
    printf("SIFT algorithm for image%d\n", imgID);


    ///////////////////////////////////// LoG BEGIN /////////////////////////////////////
    // Find Difference of Gaussian Images using LoG
    LoG LoG_processor(src);
    std::vector<Image> octave1_log, octave2_log, octave3_log, octave4_log;

    ADD_TIME = SIFT_TIME = LoG_processor.find_LoG_images(
        octave1_log, octave2_log, octave3_log, octave4_log);
    printf("LoG process time:                      %.3f ms\n", 1000.f * SIFT_TIME);
    TOTAL_TIME += ADD_TIME;
    ///////////////////////////////////// LoG END /////////////////////////////////////


    ///////////////////////////////////// Keypoint begin /////////////////////////////////////
    // Find keypoint image-pairs between the DoG images
    Keypoint kp_finder(src, grad_threshold, intensity_threshold);
    std::vector<Image> octave1_kp, octave2_kp, octave3_kp, octave4_kp;

    ADD_TIME = SIFT_TIME = kp_finder.find_keypoints(octave1_log, octave1_kp);
    TOTAL_TIME += ADD_TIME;
    printf("keypoint_find for octave1 time:        %.3f ms\n", 1000.f * SIFT_TIME);
    ADD_TIME = SIFT_TIME = kp_finder.find_keypoints(octave2_log, octave2_kp);
    TOTAL_TIME += ADD_TIME;
    ADD_TIME = SIFT_TIME = kp_finder.find_keypoints(octave3_log, octave3_kp);
    TOTAL_TIME += ADD_TIME;
    ADD_TIME = SIFT_TIME = kp_finder.find_keypoints(octave4_log, octave4_kp);
    TOTAL_TIME += ADD_TIME;

    if (debug) cout << "Storing result" << endl;
    printf("%lu, %d\n", octave1_kp.size(), view_index);

    Image gradx(src.rows, src.cols), grady(src.rows, src.cols);
    std::vector<coord> keypoints;

    std::vector<PointWithAngle> points_with_angle;
    ADD_TIME = SIFT_TIME = kp_finder.find_corners_gradients(
        octave1_kp[view_index], keypoints, points_with_angle);
    TOTAL_TIME += ADD_TIME;
    printf("corner detection for octave1 time:     %.3f ms\n", 1000.f * SIFT_TIME);
    printf("keypoints: %d\n", keypoints.size());

    std::vector<float> kp_gradients;

    std::vector<KeypointFeature> keypoint_features;
    ADD_TIME = SIFT_TIME = kp_finder.find_keypoint_orientations(keypoints, points_with_angle, 
        keypoint_features, src.rows, src.cols, standard_variances[2]);
    TOTAL_TIME += ADD_TIME;
    printf("keypoint orientation for octave1 time: %.3f ms\n", 1000.f * SIFT_TIME);

    ADD_TIME = SIFT_TIME = kp_finder.store_keypoints(keypoint_features, cv_keypoints, 1, src.cols);
    TOTAL_TIME += ADD_TIME;
    printf("keypoint storing for octave1 time:     %.3f ms\n", 1000.f * SIFT_TIME);

    ADD_TIME = SIFT_TIME = kp_finder.store_features(keypoint_features, result_features);
    TOTAL_TIME += ADD_TIME;
    printf("feature generation for octave1 time:   %.3f ms\n", 1000.f * SIFT_TIME);

    printf("TOTAL_TIME:                            %.3f ms\n", 1000.f * TOTAL_TIME);
    printf("/////////////////////////////////////////////////////////////////////////////////\n");

    return TOTAL_TIME;
}
