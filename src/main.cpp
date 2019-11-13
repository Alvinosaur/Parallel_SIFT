#include <vector>
#include <iostream>
#include <string>
#include <getopt.h>
#include <opencv2/highgui/highgui.hpp>

#include "GaussianBlur.h"
#include "LoG.h"
#include "Image.h"
#include "cv_helpers.h"
#include "general_helpers.h"

using namespace std;

bool debug = false;
int view_index = 0;

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

    LoG LoG_processor(src);
    std::vector<Image> octave1, octave2, octave3, octave4;
    LoG_processor.find_LoG_images(octave1, octave2, octave3, octave4);
    
    if (debug) cout << "Storing result" << endl;
    octave1[view_index].store_opencv(res_output);
    imwrite( "after_blur_result.jpg", res_output);
    cv::namedWindow( "Gray image", CV_WINDOW_AUTOSIZE );
    imshow( "Blurred pikachu!", res_output );
    cv::waitKey(0);
    return 0;
}
