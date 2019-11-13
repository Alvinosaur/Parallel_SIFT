#include "general_helpers.h"

int almost_equal(float v1, float v2, float abs_error) {
    return (v1 - v2) <= abs_error;
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
}


bool get_args(int argc, char** argv, std::string & img_path, float* variance, 
        bool* debug, int* view_index) {
    /*
     * Reads in input args from commandline.
     */
    extern char* optarg;
    int option;
    bool set_img_path = false;
    while ((option = getopt(argc, argv, "hdp:i:v:")) != EOF) {
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
                break;
            case 'v':
                *variance = atof(optarg);
                if (*debug) printf("Variance: %f\n", *variance);
                break;
            default:
                print_usage();
                break;
        }
    }
    return set_img_path;
}