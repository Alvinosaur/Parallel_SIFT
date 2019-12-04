#include "GaussianBlur.h"
#include <iostream>
#include <assert.h>
#include "general_helpers.h"
#include <cmath>

#define DEBUG 1
#define MAX_VARIANCE 4

const float MAX_ERROR = 1e-05;

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
    int y = 0;
    #pragma omp parallel for reduction(+: sum) 
    for (int j = 0; j < rows * cols; j++) {
        int x0 = 0, x1, y1, shift, new_val; 
        int x = j % cols;
        if (x == cols -1 && y != rows-1) y++;

        float sum = 0.0;
        for(int i = 0; i < K; i++){
            shift = i - mean_K;
            y1 = reflect(rows, y + shift);
            sum += distrib[i]* (float)img.get(y1, x);
        }
        temp[y*cols + x] = sum;
    }


    // along x - direction
    y = 0;
    // #pragma omp parallel for reduction(+: sum)
    for (int j = 0; j < rows * cols; j++) {
        int x0 = 0, x1, y1, shift, new_val; 
        int x = j % cols;
        if (x == cols -1 && y != rows-1) y++;

        sum = 0.0;
        for(int i = 0; i < K; i++){
            shift = i - mean_K;
            x1 = reflect(cols, x + shift);
            sum = sum + distrib[i]*temp[y*cols + x1];
        }
        new_val = (int)sum;
        new_img.set(y, x, new_val);
    }


}

