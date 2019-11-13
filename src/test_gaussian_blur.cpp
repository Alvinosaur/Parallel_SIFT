#include <vector>
#include <iostream>
#include <string>
#include <opencv2/highgui/highgui.hpp>

#include "GaussianBlur.h"
#include "Image.h"
#include "cv_helpers.h"
#include "general_helpers.h"
#include "LoG.h"

using namespace std;
using namespace cv;

bool debug = false;
int view_index = 0;

int main(int argc, char* argv[]) {
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

    Gaussian_Blur gb;
    Mat res_output, src_mat = imread(img_path.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
    Image src(src_mat);
    Image res_var1(src_mat.rows, src_mat.cols);
    Image res_var2(src_mat.rows, src_mat.cols);
    Image res_var3(src_mat.rows, src_mat.cols);
    Image res_var4(src_mat.rows, src_mat.cols);
    Image res_var5(src_mat.rows, src_mat.cols);
    gb.convolve(src, res_var1, standard_variances[0]);
    gb.convolve(src, res_var2, standard_variances[1]);
    gb.convolve(src, res_var3, standard_variances[2]);
    gb.convolve(src, res_var4, standard_variances[3]);
    gb.convolve(src, res_var5, standard_variances[4]);

    cout << src_mat.rows << ", " << src_mat.cols << endl;
    Image shrunk(src_mat.rows / 2, src_mat.cols / 2);
    shrink_half(res_var1, shrunk);

    Image diff = res_var1 - res_var2;
    std::vector<Image> test{diff};
    
    if (debug) cout << "Storing result" << endl;
    test[view_index].store_opencv(res_output);
    imwrite( "after_blur_result.jpg", res_output);
    namedWindow( "Gray image", CV_WINDOW_AUTOSIZE );
    imshow( "Blurred pikachu!", res_output );
    waitKey(0);

    return 0;

}