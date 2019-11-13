#include "GaussianBlur.h"
#include <iostream>
#include <assert.h>
#include "general_helpers.h"

#define DEBUG 1
#define MAX_VARIANCE 4

const float MAX_ERROR = 1e-05;

// sigma^2 = np(1-p) = n/4 [p = 1/2 binomial distrib]
int Gaussian_Blur::variance_to_depth(float var) {
    return (int)(var * var * 4.0);
}

// Generate pascal triangle
// Source: https://www.geeksforgeeks.org/pascal-triangle/
void Gaussian_Blur::generate_binomial_distrib(int n, 
        std::vector<float> & new_distrib) {
    // depth of n has n terms
    new_distrib.reserve(n);
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


// handle edge cases where kernel exceeds image bounds
// by having out-of-bounds pixel be pixel on opposite side of kernel
int reflect(int M, int x) {
    if(x < 0) return -x - 1;
    if(x >= M) return 2*M - x - 1;
    return x;
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