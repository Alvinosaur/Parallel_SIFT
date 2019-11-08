#include "GaussianBlur.h"
#include "Image.h"
#include <vector>
#include <iostream>
#include <string>
#include <getopt.h>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;


bool debug = false;
const std::vector<float> standard_variances({
    (float)(pow(2, 0.5) / 2.0),
    (float)(pow(2, 1) / 2.0),
    (float)(pow(2, 1.5) / 2.0),
    (float)(pow(2, 2) / 2.0)
});

void usage() {
    /*
     * Prints out usage info for passing in commandline args for matrix
     * generation.
     */
    printf("Usage:\n");
    printf("   -h   print help message describing usage\n");
    printf("   -d   print additional diagnostic information\n");
    printf("   -i   path for input image\n");
    printf("   -v   desired variance level\n");
}

bool get_args(int argc, char** argv, string & img_path, float* variance) {
    /*
     * Reads in input args from commandline.
     */
    extern char* optarg;
    int option;
    bool set_img_path = false;
    while ((option = getopt(argc, argv, "hdi:v:")) != EOF) {
        switch (option) {
            case 'h':  // help
                usage();
                break;
            case 'd':
                debug = true;
                break;
            case 'i':
                img_path = optarg;
                set_img_path = true;
                if (debug) printf("Input Image Path: %s\n", img_path.c_str());
                break;
            case 'v':
                *variance = atof(optarg);
                if (debug) printf("Variance: %f\n", *variance);
                break;
            default:
                usage();
                break;
        }
    }
    return set_img_path;
}


int main(int argc, char* argv[]) {
    if (argc <= 1) {
        cout << "Need to pass in image filepath!" << endl;
        exit(-1);
    }
    std::string img_path;
    float variance = 1;
    if (!get_args(argc, argv, img_path, &variance)) {
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
    gb.convolve(src, res_var1, standard_variances[0]);
    gb.convolve(src, res_var2, standard_variances[1]);
    gb.convolve(src, res_var3, standard_variances[2]);
    gb.convolve(src, res_var4, standard_variances[3]);
    
    if (debug) cout << "Storing result" << endl;
    res_var4.store_opencv(res_output);
    imwrite( "after_blur_result.jpg", res_output);
    namedWindow( "Gray image", CV_WINDOW_AUTOSIZE );
    imshow( "Blurred pikachu!", res_output );
    waitKey(0);

    return 0;

}