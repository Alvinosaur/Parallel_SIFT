#include "GaussianBlur.h"
#include <iostream>

// print vector of any type, as long as that type can be printed
template <class data_type>
void print_arr(std::vector<data_type> vec) {
    for (data_type val : vec) std::cout << val << ", ";
    std::cout << std::endl;
}

// sigma^2 = np(1-p) = n/4 [p = 1/2 binomial distrib]
int Gaussian_Blur::variance_to_depth(float var) {
    return (int)(var * var * 4.0);
}

// Generate pascal triangle
// Source: https://www.geeksforgeeks.org/pascal-triangle/
void Gaussian_Blur::generate_binomial_distrib(int n, 
        std::vector<int> & new_distrib) {
    // depth of n has n terms
    new_distrib.reserve(n);
    // The first value in a line is always 1 
    int val = 1;
    for (int i = 1; i <= n; i++) { 
        new_distrib.push_back(val);
        val = val * (n - i) / i;
    }
}


// handle edge cases where kernel exceeds image bounds
// by having out-of-bounds pixel be pixel on opposite side of kernel
int reflect(int M, int x) {
    if(x < 0) return -x - 1;
    if(x >= M) return 2*M - x - 1;
    return x;
}

void Gaussian_Blur::convolve(Image & img, float var) {
    int row, col, rows = img.rows, cols = img.cols;
    int temp[rows * cols];  // result from vertical convolution
    std::vector<int> distrib;  // one-dimensional binomial distribution
    int depth = variance_to_depth(var);
    int kernel_norm = 2 << depth;
    generate_binomial_distrib(depth, distrib);
    print_arr<int>(distrib);
    int K = distrib.size();
    int mean_K = K / 2;

    // along y - direction
    int sum, x, y, x1, y1, i, shift, new_val;
    for(y = 0; y < rows; y++){
        for(x = 0; x < cols; x++){
            sum = 0;
            for(i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                sum = sum + distrib[i]*img.get(y1, x);
            }
            temp[y*cols + x] = sum;
        }
    }

    // along x - direction
    for(y = 0; y < img.rows; y++){
        for(x = 0; x < img.cols; x++){
            sum = 0;
            for(i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum = sum + distrib[i]*temp[y*cols + x1];
            }
            new_val = (int)((float)sum / (float)kernel_norm);
            img.set(y, x, new_val);
        }
    }
}