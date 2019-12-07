#ifndef _GEN_HELPERS_
#define _GEN_HELPERS_

#include <Image.h>

#include <vector>
#include <iostream>
#include <string>
#include <getopt.h>
#include "mpi.h"

#define HALF 2
#define QUARTER 4
#define EIGHTH 8

extern const std::vector<float> standard_variances;
typedef std::pair<int, int> coord;
typedef std::pair<int, int> range;

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
bool get_args(int argc, char** argv,
        std::string & img1_path, std::string & img2_path, float* variance,
        bool* debug, int* view_index, float* gradient_threshold);

void shrink(Image & src, Image & dst, int scale);
void shrink_mpi(const Image & src, int* result, const range & start_end,
    int scale);

void allocate_shrink_work_mpi(int rows, int cols, int num_tasks,
        std::vector<range> & half_assignments,
        std::vector<range> & quarter_assignments,
        std::vector<range> & eighth_assignments);

void send_shrink_to_others(int* data, MPI_Request* reqs, int self_rank,
        range self_range, int scale, int num_tasks);
void receive_shrink_from_others(int* result, MPI_Request* reqs,
    std::vector<range> & assignments, int self_rank, int scale);

#endif
