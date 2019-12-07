#include <vector>
#include <iostream>
#include <string>
#include <opencv2/highgui/highgui.hpp>

#include "GaussianBlur.h"
#include "Image.h"
#include "general_helpers.h"
#include "LoG.h"

using namespace std;
using namespace cv;

bool debug = false;
int view_index = 0;
float grad_threshold = 0;

void shrink_half(Image & src, Image & dst) {
    int total_val;
    for (int i = 0; i < src.rows-1; i+=2) {
        for (int j = 0; j < src.cols-1; j+=2) {
            total_val = (src.get(i, j) + src.get(i+1, j) +
                src.get(i, j+1) + src.get(i+1, j+1));
            dst.set(i/2, j/2, (int)((float)total_val / 4.0));
        }
    }
}

int main(int argc, char* argv[]) {
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

    cv::Mat res_output;
    cv::Mat src1_mat = cv::imread(img1_path.c_str(),
        CV_LOAD_IMAGE_GRAYSCALE);
    Image src1(src1_mat);
    Image half(src1.rows/2, src1.cols/2);
    Image quarter(src1.rows/4, src1.cols/4);
    Image eighth(src1.rows/8, src1.cols/8);
    shrink(src1, half, 2);
    // shrink(half, quarter, 2);

    if (debug) cout << "Storing result" << endl;
    half.store_opencv(res_output);
    imwrite( "after_blur_result.jpg", res_output);
    namedWindow( "Gray image", CV_WINDOW_AUTOSIZE );
    imshow( "Blurred pikachu!", res_output );
    waitKey(0);

    return 0;

}
