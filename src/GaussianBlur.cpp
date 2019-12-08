#include "GaussianBlur.h"
#include <iostream>
#include <assert.h>
#include "general_helpers.h"
#include <cmath>
#include "mpi.h"

#define DEBUG 1
#define MAX_VARIANCE 4

const float MAX_ERROR = 1e-05;

void allocate_conv_rows_mpi(int rows, int cols, int num_tasks,
        std::vector<range> & row_assignments,
        std::vector<range> & col_assignments,
        std::vector<range> & row_pix_assignments,
        std::vector<range> & col_pix_assignments) {
    int rows_per_task = rows / num_tasks;
    int cols_per_task = cols / num_tasks;

    for (int task = 0; task < num_tasks; task++) {
        // Work for rows
        range row_range, row_pix_range;
        row_range.first = rows_per_task * task;
        row_range.second = rows_per_task * (task + 1);
        row_pix_range.first = row_range.first * cols;
        row_pix_range.second = row_range.second * cols;

        // Work for cols
        range col_range, col_pix_range;
        col_range.first = cols_per_task * task;
        col_range.second = cols_per_task * (task + 1);
        // do flipped row-major order
        col_pix_range.first = col_range.first * rows;
        col_pix_range.second = col_range.second * rows;

        // account for leftovers if uneven division of pixels btwn tasks
        // have last task take care of extra rows
        if (task == num_tasks-1) {
            row_range.second += rows % num_tasks;
            col_range.second += cols % num_tasks;
        }

        // store assignments
        row_assignments.push_back(row_range);
        col_assignments.push_back(col_range);
        row_pix_assignments.push_back(row_pix_range);
        col_pix_assignments.push_back(col_pix_range);
    }
}

// sigma^2 = np(1-p) = n/4 [p = 1/2 binomial distrib]
int Gaussian_Blur::variance_to_depth(float var) {
    return (int)ceilf(var * var * 4.0);
}

// Generate pascal triangle
// Source: https://www.geeksforgeeks.org/pascal-triangle/
void Gaussian_Blur::generate_binomial_distrib(int n, 
        std::vector<float> & new_distrib) {
    // depth of n has n terms
    new_distrib.reserve(n);
    if (n == 1) {
        new_distrib.push_back(0.5);
        new_distrib.push_back(0.5);
        return;
    }
    // The first value in a line is always 1 
    long double val = 1;
    // 2^(n-1) is sum of weights for nth level binomial distrib
    uint64_t norm = ((uint64_t)1 << (n-1));
    for (int i = 1; i <= n; i++) { 
        new_distrib.push_back((float)(val / norm));
        val = val * (n - i) / i;
        
    }
    if (DEBUG) assert(almost_equal(sum<float>(new_distrib), 1.0, MAX_ERROR));
}

void Gaussian_Blur::convolve_rows_mpi(Image & img, int* row_conv, 
        range col_range, std::vector<float> & distrib) {
    int row, col, rows = img.rows, cols = img.cols;
    int K = distrib.size();
    int mean_K = K / 2;
    float sum;
    int c, r, r1, i, shift;
    int start = col_range.first, end = col_range.second;

    // might want to switch the order of the two loops so less branching
    for(r = 0; r < rows; r++){
        for(c = start; c < end; c++){
            sum = 0.0;
            for(i = 0; i < K; i++){
                shift = i - mean_K;
                r1 = reflect(rows, r + shift);
                sum = sum + distrib[i] * (float)img.get(r1, c);
            }
            // Must flip to store in row-major order so can be sent contiguously
            row_conv[c*rows + cols] = sum;
        }
    }
}

void Gaussian_Blur::convolve_cols_mpi(int* row_conv, int* col_conv, 
        range row_range, std::vector<float> & distrib, int cols, int rows) {
    int row, col;
    int K = distrib.size();
    int mean_K = K / 2;
    float sum;
    int r, c, c1, i, shift;
    int start = row_range.first, end = row_range.second;

    // along x - direction
    for(r = start; r < end; r++){
        for(c = 0; c < cols; c++){
            sum = 0.0;
            for(i = 0; i < K; i++){
                shift = i - mean_K;
                c1 = reflect(cols, c + shift);
                // flipped image version
                sum = sum + distrib[i] * (float)row_conv[c1*rows + r];
            }
            col_conv[r*cols + c] = sum;
        }
    }
}

void Gaussian_Blur::convolve(Image & img, Image &new_img, float var) {
    if (var == 0) {
        std::cout << "NOTE: Input variance = 0, no change to image";
        std::cout << std::endl;
        return;
    }
    else if (var >= MAX_VARIANCE) {
        std::cout << "Desired variance too high! Max is " << MAX_VARIANCE;
        std::cout << std::endl;
        return;
    }
    int row, col, rows = img.rows, cols = img.cols;
    float temp[rows * cols];  // result from vertical convolution
    std::vector<float> distrib;  // one-dimensional binomial distribution
    int depth = variance_to_depth(var);
    generate_binomial_distrib(depth, distrib);
    int K = distrib.size();
    int mean_K = K / 2;

    // along y - direction
    float sum;
    int x, y, x1, y1, i, shift, new_val;
    for(y = 0; y < rows; y++){
        for(x = 0; x < cols; x++){
            sum = 0.0;
            for(i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                sum = sum + distrib[i]* (float)img.get(y1, x);
            }
            temp[y*cols + x] = sum;
        }
    }

    // along x - direction
    for(y = 0; y < img.rows; y++){
        for(x = 0; x < img.cols; x++){
            sum = 0.0;
            for(i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum = sum + distrib[i]*temp[y*cols + x1];
            }
            new_val = (int)sum;
            new_img.set(y, x, new_val);
        }
    }
}