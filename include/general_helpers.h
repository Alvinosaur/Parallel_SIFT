#ifndef _GEN_HELPERS_
#define _GEN_HELPERS_

#include <vector>
#include <iostream>
#include <string>
#include <getopt.h>

// print vector of any type, as long as that type can be printed
template <class data_type>
void print_arr(std::vector<data_type> vec) {
    for (data_type val : vec) std::cout << val << ", ";
    std::cout << std::endl;
}

template <class data_type>
data_type sum(std::vector<data_type> & vec) {
    data_type total = 0;
    for (data_type val : vec) total += val;
    return total;
}

int almost_equal(float v1, float v2, float abs_error);

// handle edge cases where kernel exceeds image bounds
// by having out-of-bounds pixel be pixel on opposite side of kernel
int reflect(int M, int x);

void print_usage();
bool get_args(int argc, char** argv, std::string & img_path, float* variance, 
        bool* debug, int* view_index);

#endif