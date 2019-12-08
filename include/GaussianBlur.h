#ifndef _GBLUR_
#define _GBLUR_

#include "Image.h"
#include "general_helpers.h"
#include <vector>
#include <utility>

// First step: write function to generate any gaussian blur given
// some variance sigma value
// Figure out how to do horizontal and vertical gaussian blurs separately
// 

void allocate_conv_rows_mpi(int rows, int cols, int num_tasks,
        std::vector<range> & row_assignments,
        std::vector<range> & row_pix_assignments);

class Gaussian_Blur {
    std::vector<std::vector<int>> distribs;
    std::vector<std::vector<int>> variances;
public:
    int variance_to_depth(float var);
    void generate_binomial_distrib(int n, std::vector<float> & new_distrib);
    int generate_kernel(std::vector<int> & kernel, float var);
    void convolve(Image & img, Image & new_img, float var);
    // NOTE: convolve_cols uses the result from convolve_rows
    void convolve_rows_mpi(Image & img, int* row_conv, 
        range row_range, std::vector<float> & distrib);
    void convolve_cols_mpi(int* row_conv, int* col_conv, 
        range row_range, std::vector<float> & distrib, int cols);
};

#endif