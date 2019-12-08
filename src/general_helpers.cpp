#include "general_helpers.h"
#include "CycleTimer.h"


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

double shrink_half(Image & src, Image & dst) {
    int total_val;
    double startTime = CycleTimer::currentSeconds();

    // int i, j;
    // for (int k = 0; k < (src.rows-1) * (src.cols-1); k += 2) {
    //     j = k % src.cols;
    //     if (j == src.cols -2 && i != src.rows -2) i+= 2;

    //     total_val = (src.get(i, j) + src.get(i+1, j) + 
    //         src.get(i, j+1) + src.get(i+1, j+1));
    //     dst.set(i/2, j/2, (int)((float)total_val / 4.0));
    // }
    for (int i = 0; i < src.rows-1; i+=2) {
        for (int j = 0; j < src.cols-1; j+=2) {
            total_val = (src.get(i, j) + src.get(i+1, j) + 
                src.get(i, j+1) + src.get(i+1, j+1));
            dst.set(i/2, j/2, (int)((float)total_val / 4.0));
        }
    }

    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;
}

double shrink_quarter(Image & src, Image & dst) {
    double startTime = CycleTimer::currentSeconds();

    int total_val = 0;
    for (int i = 0; i < src.rows-3; i+=4) {
        for (int j = 0; j < src.cols-3; j+=4) {

            for (int y = 0; y < 3; y++) {
                for (int x = 0; x < 3; x++) {
                    total_val += src.get(i+y, j+x);
                }
            }
            dst.set(i/4, j/4, (int)((float)total_val / 4.0));
        }
    }

    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;
}

double shrink_eighth(Image & src, Image & dst) {
    double startTime = CycleTimer::currentSeconds();

    int total_val = 0;
    for (int i = 0; i < src.rows-7; i+=8) {
        for (int j = 0; j < src.cols-7; j+=8) {

            for (int y = 0; y < 7; y++) {
                for (int x = 0; x < 7; x++) {
                    total_val += src.get(i+y, j+x);
                }
            }
            dst.set(i/8, j/8, (int)((float)total_val / 4.0));
        }
    }

    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;
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