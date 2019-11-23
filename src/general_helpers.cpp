#include "general_helpers.h"

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

void print_usage() {
    /*
     * Prints out usage info for passing in commandline args for matrix
     * generation.
     */
    printf("Usage:\n");
    printf("   -h   print help message describing usage\n");
    printf("   -d   print additional diagnostic information\n");
    printf("   -p   path for input image\n");
    printf("   -i   view_index for which gaussian difference to view\n");
    printf("   -v   desired variance level\n");
    printf("   -g   gradient threshold for keypoint filtering\n");
}


bool get_args(int argc, char** argv, std::string & img_path, float* variance, 
        bool* debug, int* view_index, float* gradient_threshold) {
    /*
     * Reads in input args from commandline.
     */
    extern char* optarg;
    int option;
    bool set_img_path = false;
    while ((option = getopt(argc, argv, "hdp:i:v:g:")) != EOF) {
        switch (option) {
            case 'h':  // help
                print_usage();
                break;
            case 'd':
                *debug = true;
                break;
            case 'p':
                img_path = optarg;
                set_img_path = true;
                if (*debug) printf("Input Image Path: %s\n", img_path.c_str());
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
    return set_img_path;
}