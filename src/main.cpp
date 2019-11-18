#include <vector>
#include <iostream>
#include <string>
#include <getopt.h>
#include <opencv2/highgui/highgui.hpp>

#include "GaussianBlur.h"
#include "Keypoint.h"
#include "LoG.h"
#include "Image.h"
#include "cv_helpers.h"
#include "general_helpers.h"

using namespace std;

bool debug = false;
int view_index = 0;
float grad_threshold = 0;

int main(int argc, char* argv[]){
    if (argc <= 1) {
        cout << "Need to pass in image filepath!" << endl;
        exit(-1);
    }
    std::string img_path;
    float variance = 1;
    if (!get_args(argc, argv, img_path, &variance, &debug, &view_index)) {
        std::cout << "Failed to pass in valid image path with -i" << std::endl;
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
    Keypoint kp_finder(src, grad_threshold);
    std::vector<Image> octave1_kp, octave2_kp, octave3_kp, octave4_kp;
    kp_finder.find_keypoints(octave1_log, octave1_kp);
    kp_finder.find_keypoints(octave2_log, octave2_kp);
    kp_finder.find_keypoints(octave3_log, octave3_kp);
    kp_finder.find_keypoints(octave4_log, octave4_kp);
    
    if (debug) cout << "Storing result" << endl;
    printf("%lu, %d\n", octave1_kp.size(), view_index);

    Image gradx(src.rows, src.cols), grady(src.rows, src.cols);
    std::vector<std::pair<int, int>> keypoints;
    kp_finder.find_xy_gradient(octave1_kp[view_index], gradx, grady, 
        true, keypoints);
    
    octave1_kp[view_index].store_opencv(res_output);
    imwrite( "after_blur_result.jpg", res_output);
    cv::namedWindow( "Gray image", CV_WINDOW_AUTOSIZE );
    imshow( "Blurred pikachu!", res_output );
    cv::waitKey(0);
    return 0;
}
