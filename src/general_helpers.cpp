#include "general_helpers.h"
#include <stdio.h>

int almost_equal(float v1, float v2, float abs_error) {
    return (v1 - v2) <= abs_error;
}

// handle edge cases where kernel exceeds image bounds
// by having out-of-bounds pixel be pixel on opposite side of kernel
int reflect(int M, int x) {
    if(x < 0) return -x - 1;
    if(x >= M) return 2*M - x - 1;
    return x;
}


void shrink(Image & src, Image & dst, int scale) {
    int total_val;
    int r_offset, c_offset;
    int i, j, k;
    int cols = src.cols / 2;
    int max_k = rows * cols;
    for (int k = 0; k < max_k; k++) {
        j = k % cols * 2;
        i = k / cols * 2;
        total_val = 0;
        for (r_offset = 0; r_offset < scale; r_offset++) {
            for (c_offset = 0; c_offset < scale; c_offset++) {
                total_val += src.get(i+r_offset, j+c_offset);
            }
        }
        dst.set(i/scale, j/scale, (int)(
            (float)total_val / (float)(scale * scale)));
    }
}

void print_usage() {
    /*
     * Prints out usage info for passing in commandline args for matrix
     * generation.
     */
    printf("Usage:\n");
    printf("   -h   print help message describing usage\n");
    printf("   -d   print additional diagnostic information\n");
    printf("   -a   path for input image 1\n");
    printf("   -b   path for input image 2\n");
    printf("   -i   view_index for which gaussian difference to view\n");
    printf("   -v   desired variance level\n");
    printf("   -g   gradient threshold for keypoint filtering\n");
}


bool get_args(int argc, char** argv,
        std::string & img1_path, std::string & img2_path, float* variance,
        bool* debug, int* view_index, float* gradient_threshold) {
    /*
     * Reads in input args from commandline.
     */
    extern char* optarg;
    int option;
    bool set_img1_path = false, set_img2_path = false;
    while ((option = getopt(argc, argv, "hda:b:i:v:g:")) != EOF) {
        switch (option) {
            case 'h':  // help
                print_usage();
                break;
            case 'd':
                *debug = true;
                break;
            case 'a':
                img1_path = optarg;
                set_img1_path = true;
                if (*debug) printf("Input Image1: %s\n", img1_path.c_str());
                break;
            case 'b':
                img2_path = optarg;
                set_img2_path = true;
                if (*debug) printf("Input Image2: %s\n", img2_path.c_str());
                break;
            case 'i':
                *view_index = atoi(optarg);
                printf("Index: %d\n", *view_index);
                break;
            case 'v':
                *variance = atof(optarg);
                if (*debug) printf("Variance: %f\n", *variance);
                break;
            case 'g':
                *gradient_threshold = atof(optarg);
                if (*gradient_threshold) printf(
                    "Gradient Threshold: %f\n", *gradient_threshold);
                break;
            default:
                print_usage();
                break;
        }
    }
    return set_img1_path && set_img2_path;
}
