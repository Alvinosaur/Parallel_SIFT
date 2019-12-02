#include <vector>
#include <iostream>
#include <string>
#include <getopt.h>
#include <opencv2/highgui/highgui.hpp>

#include "GaussianBlur.h"
#include "Keypoint.h"
#include "LoG.h"
#include "Image.h"
#include "general_helpers.h"

using namespace std;

bool debug = false;
int view_index = 0;

int main(int argc, char* argv[]){
    float variance = 1;
    float grad_threshold = 0;
    float intensity_threshold = 1;
    if (argc <= 1) {
        cout << "Need to pass in image filepath!" << endl;
        exit(-1);
    }
    std::string img_path;
    if (!get_args(argc, argv, img_path, &variance, &debug, &view_index, 
            &grad_threshold)) {
        std::cout << "Failed to pass in valid image path with -p" << std::endl;
        exit(-1);
    };

    cv::Mat res_output, src_mat = cv::imread(img_path.c_str(), 
        CV_LOAD_IMAGE_GRAYSCALE);
    Image src(src_mat);

    // Find Difference of Gaussian Images using LoG
    LoG LoG_processor(src);
    std::vector<Image> octave1_log, octave2_log, octave3_log, octave4_log;
    LoG_processor.find_LoG_images(octave1_log, octave2_log, octave3_log, 
        octave4_log);

    // Find keypoint image-pairs between the DoG images
    Keypoint kp_finder(src, grad_threshold, intensity_threshold);
    std::vector<Image> octave1_kp, octave2_kp, octave3_kp, octave4_kp;
    kp_finder.find_keypoints(octave1_log, octave1_kp);
    // kp_finder.find_keypoints(octave2_log, octave2_kp);
    // kp_finder.find_keypoints(octave3_log, octave3_kp);
    // kp_finder.find_keypoints(octave4_log, octave4_kp);
    
    if (debug) cout << "Storing result" << endl;
    printf("%lu, %d\n", octave1_kp.size(), view_index);

    Image gradx(src.rows, src.cols), grady(src.rows, src.cols);
    std::vector<coord> keypoints;
    float grad_magnitudes[src.rows * src.cols];
    float grad_orientations[src.rows * src.cols];
    std::vector<PointWithAngle> points_with_angle;
    kp_finder.find_corners_gradients(octave1_kp[view_index], keypoints,
        points_with_angle);

    std::vector<float> kp_gradients;

    Image keypoints_img(src.rows, src.cols);
    kp_finder.mark_keypoints(keypoints_img, keypoints);
    printf("Num keypoints: %lu\n", keypoints.size());
    
    //     remove_target.set(r, c, 0);
    //     // printf("Removed a keypoint(%d, %d) with grad(%d, %d)\n", 
    //         // r, c, grad_x, grad_y);
    // } else {
    //     remove_target.set(r, c, remove_target.get(r, c) * 70);
    
    keypoints_img.store_opencv(res_output);
    imwrite( "after_blur_result.jpg", res_output);
    cv::namedWindow( "Gray image", CV_WINDOW_AUTOSIZE );
    imshow( "Blurred pikachu!", res_output );
    cv::waitKey(0);
    return 0;
}
