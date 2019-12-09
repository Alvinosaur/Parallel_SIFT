#include "GaussianBlur.h"
#include <iostream>
#include <assert.h>
#include "general_helpers.h"
#include <cmath>
#include "CycleTimer.h"


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


double Gaussian_Blur::convolve(Image & img, Image &new_img, float var) {

    double startTime = CycleTimer::currentSeconds();

    if (var == 0) {
        std::cout << "NOTE: Input variance = 0, no change to image";
        std::cout << std::endl;
        return 0.0;
    }
    else if (var >= MAX_VARIANCE) {
        std::cout << "Desired variance too high! Max is " << MAX_VARIANCE;
        std::cout << std::endl;
        return 0.0;
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
    int x = 0,y = 0;
    int x1, y1, shift, new_val; 

    // #pragma omp parallel for shared(new_img, temp, distrib) reduction(+: sumy) reduction(+: sumx)
    // #pragma omp parallel for schedule(dynamic)
    // #pragma omp parallel for reduction(+: sum)
    // #pragma omp parallel 
    // {
    //     #pragma omp for schedule(static) nowait reduction(+: sum)
    //     for (int j = 0; j < rows * cols; j++) {
    //         int x1, y1, shift, new_val; 
    //         int x = j % cols;
    //         if (x == cols -1 && y != rows-1) y++;

    //         sum = 0.0;
    //         for(int i = 0; i < K; i++){
    //             shift = i - mean_K;
    //             y1 = reflect(rows, y + shift);
    //             sum += distrib[i] * (float)img.get(y1, x);
    //         }
    //         temp[y*cols + x] = sum;
    //     }
    // }

    // printf("%d %d\n", rows, cols);

    for (int j = 0; j < rows * cols; j++) {
        x = j % cols;
        if (x == cols -1 && y != rows-1) y++;

        sum = 0.0;
        for(int i = 0; i < K; i++){
            shift = i - mean_K;
            y1 = reflect(rows, y + shift);
            sum += distrib[i] * (float)img.get(y1, x);
        }
        temp[y*cols + x] = sum;
    }

    y = 0;
    for (int j = 0; j < rows * cols; j++) {
        x = j % cols;
        if (x == cols -1 && y != rows-1) y++;

        sum = 0.0;
        for(int i = 0; i < K; i++){
            shift = i - mean_K;
            x1 = reflect(cols, x + shift);
            sum += distrib[i] * temp[y*cols + x1];
        }
        new_val = (int)sum;
        new_img.set(y, x, new_val);
    }
    

    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;

}

double Gaussian_Blur::convolve_half_y(Image & img, std::vector<float> & temp_temp, Image &new_img, 
                                         float var, int n) {
                                          // std::vector<float> distrib, float var, int n) {

    double startTime = CycleTimer::currentSeconds();

    if (var == 0) {
        std::cout << "NOTE: Input variance = 0, no change to image";
        std::cout << std::endl;
        return 0.0;
    }
    else if (var >= MAX_VARIANCE) {
        std::cout << "Desired variance too high! Max is " << MAX_VARIANCE;
        std::cout << std::endl;
        return 0.0;
    }
    int rows = img.rows, cols = img.cols;
    std::vector<float> distrib;  // one-dimensional binomial distribution
    generate_binomial_distrib(variance_to_depth(var), distrib);
    float temp[rows * cols];  // result from vertical convolution

    int K = distrib.size();
    int mean_K = K / 2;

    int middle = (rows * cols) / 2;

    int x = 0, y = 0, y1, shift, new_val; 
    int sum;


    if (n == 0) {
        y = 0;
        // printf("here y 0\n");
        for (int j = 0; j < middle; j++) {
            x = j % cols;
            if (x == cols -1 && y != rows/2) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                // printf("inside %d\n", i);
                sum += distrib[i] * (float)img.get(y1, x);
            }
            // printf("here outside %d %d\n", x, y);
            temp_temp[y*cols + x] = sum;
        }
        
    } else if (n == 1) {
        y = rows/2 ;
        // printf("here y 1\n");
        for (int j = middle; j < (rows * cols); j++) {
            x = j % cols;
            if (x == cols -1 && y != rows -1) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                sum += distrib[i] * (float)img.get(y1, x);
            }
            temp_temp[y*cols + x] = sum;
        }

    } 
    
    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;

}


double Gaussian_Blur::convolve_half_x(Image & img, std::vector<float> & temp_temp, Image &new_img, 
                                         float var, int n) {
                                          // std::vector<float> distrib, float var, int n) {
// float temp_temp[],
    double startTime = CycleTimer::currentSeconds();

    if (var == 0) {
        std::cout << "NOTE: Input variance = 0, no change to image";
        std::cout << std::endl;
        return 0.0;
    }
    else if (var >= MAX_VARIANCE) {
        std::cout << "Desired variance too high! Max is " << MAX_VARIANCE;
        std::cout << std::endl;
        return 0.0;
    }
    int rows = img.rows, cols = img.cols;
    std::vector<float> distrib;  // one-dimensional binomial distribution
    generate_binomial_distrib(variance_to_depth(var), distrib);
    int K = distrib.size();
    int mean_K = K / 2;

    float temp[rows * cols];  // result from vertical convolution

    int middle = (rows * cols) / 2;

    int x = 0, y = 0, x1,  shift, new_val; 
    int sum;

    if (n == 0) {
        y = 0;
        // printf("here x\n");
        for (int j = 0; j < middle; j++) {
            x = j % cols;
            if (x == cols -1 && y != rows/2) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum += distrib[i] * temp_temp[y*cols + x1];
            }
            new_val = (int)sum;
            new_img.set(y, x, new_val);
        }
 
    } else if (n == 1) {
        y = rows/2;
        for (int j = middle; j < (rows * cols); j++) {
            x = j % cols;
            if (x == cols -1 && y != rows -1) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum += distrib[i] * temp_temp[y*cols + x1];
            }
            new_val = (int)sum;
            new_img.set(y, x, new_val);
        }

    } 


    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;

}



double Gaussian_Blur::convolve_quarters_y(Image & img, std::vector<float> & temp_temp, Image &new_img, 
                                         float var, int n) {
                                          // std::vector<float> distrib, float var, int n) {

    double startTime = CycleTimer::currentSeconds();

    if (var == 0) {
        std::cout << "NOTE: Input variance = 0, no change to image";
        std::cout << std::endl;
        return 0.0;
    }
    else if (var >= MAX_VARIANCE) {
        std::cout << "Desired variance too high! Max is " << MAX_VARIANCE;
        std::cout << std::endl;
        return 0.0;
    }
    int rows = img.rows, cols = img.cols;
    std::vector<float> distrib;  // one-dimensional binomial distribution
    generate_binomial_distrib(variance_to_depth(var), distrib);
    
    int K = distrib.size();
    int mean_K = K / 2;

    int middle = (rows * cols) / 2;
    int quarter = (rows * cols) / 4;

    int x = 0, y = 0, y1, shift, new_val; 
    int sum;
    

    if (n == 0) {
        y = 0;
        for (int j = 0; j < quarter; j++) {
            x = j % cols;
            if (x == cols -1 && y != rows/4) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                sum += distrib[i] * (float)img.get(y1, x);
            }
            temp_temp[y*cols + x] = sum;
        }
        
    } else if (n == 1) {
        y = rows/4 ;
        for (int j = quarter; j < middle; j++) {
            x = j % cols;
            if (x == cols -1 && y != rows/2) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                sum += distrib[i] * (float)img.get(y1, x);
            }
            temp_temp[y*cols + x] = sum;
        }

    } else if (n == 2) {
        y = rows/2;
        for (int j = middle; j < middle + quarter; j++) {
            x = j % cols;
            if (x == cols -1 && y != (rows * 3)/4) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                sum += distrib[i] * (float)img.get(y1, x);
            }
            temp_temp[y*cols + x] = sum;
        }
    } else if (n == 3) {
        y = (rows * 3) / 4;
        for (int j = middle + quarter; j < (rows * cols); j++) {
            x = j % cols;
            if (x == cols -1 && y != rows-1) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                sum += distrib[i] * (float)img.get(y1, x);
            }
            temp_temp[y*cols + x] = sum;
        }

    }
    
    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;

}


double Gaussian_Blur::convolve_quarters_x(Image & img, std::vector<float> & temp_temp,  Image &new_img, 
                                         float var, int n) {
                                          // std::vector<float> distrib, float var, int n) {

    double startTime = CycleTimer::currentSeconds();

    if (var == 0) {
        std::cout << "NOTE: Input variance = 0, no change to image";
        std::cout << std::endl;
        return 0.0;
    }
    else if (var >= MAX_VARIANCE) {
        std::cout << "Desired variance too high! Max is " << MAX_VARIANCE;
        std::cout << std::endl;
        return 0.0;
    }
    int rows = img.rows, cols = img.cols;
    std::vector<float> distrib;  // one-dimensional binomial distribution
    generate_binomial_distrib(variance_to_depth(var), distrib);
    int K = distrib.size();
    int mean_K = K / 2;


    int middle = (rows * cols) / 2;
    int quarter = (rows * cols) / 4;

    int x = 0, y = 0, x1,  shift, new_val; 
    int sum;

    if (n == 0) {
        y = 0;
        for (int j = 0; j < quarter; j++) {
            x = j % cols;
            if (x == cols -1 && y != rows/4) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum += distrib[i] * temp_temp[y*cols + x1];
            }
            new_val = (int)sum;
            new_img.set(y, x, new_val);
        }
 
    } else if (n == 1) {
        y = rows/4;
        for (int j = quarter; j < middle; j++) {
            x = j % cols;
            if (x == cols -1 && y != rows/2) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum += distrib[i] * temp_temp[y*cols + x1];
            }
            new_val = (int)sum;
            new_img.set(y, x, new_val);
        }

    } else if (n == 2) {
        y = rows/2;
        for (int j = middle; j < middle + quarter; j++) {
            x = j % cols;
            if (x == cols -1 && y != (rows * 3)/4) y++;
            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum += distrib[i] * temp_temp[y*cols + x1];
            }
            new_val = (int)sum;
            new_img.set(y, x, new_val);
        }
    } else if (n == 3) {
        y = (rows * 3) / 4;
        for (int j = middle + quarter; j < (rows * cols); j++) {
            x = j % cols;
            if (x == cols -1 && y != rows-1) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum += distrib[i] * temp_temp[y*cols + x1];
            }
            new_val = (int)sum;
            new_img.set(y, x, new_val);
        }
    }


    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;

}

double Gaussian_Blur::convolve_eighths_y(Image & img, std::vector<float> & temp_temp,  Image &new_img, 
                                         float var, int n) {
                                         // std::vector<float> distrib, float var, int n) {

    double startTime = CycleTimer::currentSeconds();

    if (var == 0) {
        std::cout << "NOTE: Input variance = 0, no change to image";
        std::cout << std::endl;
        return 0.0;
    }
    else if (var >= MAX_VARIANCE) {
        std::cout << "Desired variance too high! Max is " << MAX_VARIANCE;
        std::cout << std::endl;
        return 0.0;
    }
    int rows = img.rows, cols = img.cols;
    std::vector<float> distrib;  // one-dimensional binomial distribution
    generate_binomial_distrib(variance_to_depth(var), distrib);
    int K = distrib.size();
    int mean_K = K / 2;


    int middle = (rows * cols) / 2;
    int quarter = (rows * cols) / 4;
    int eighths = (rows * cols) / 8;

    int x = 0, y = 0, y1, shift, new_val; 
    float sum;

    if (n == 0) {
        y = 0;
        for (int j = 0; j < eighths; j++) {
            x = j % cols;
            if (x == cols -1 && y != (rows/8)) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                sum += distrib[i] * (float)img.get(y1, x);
            }
            temp_temp[y*cols + x] = sum;
        }
        
    } else if (n == 1) {
        y = rows/8;
        for (int j = eighths; j < quarter; j++) {
            x = j % cols;
            if (x == cols -1 && y != rows /4) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                sum += distrib[i] * (float)img.get(y1, x);
            }
            temp_temp[y*cols + x] = sum;
        }

    } else if (n == 2) {
        y = rows / 4;
        for (int j = quarter; j < quarter + eighths; j++) {
            x = j % cols;
            if (x == cols -1 && y != (rows * 3)/8) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                sum += distrib[i] * (float)img.get(y1, x);
            }
            temp_temp[y*cols + x] = sum;
        }
    } else if (n == 3) {
        y = (rows * 3) / 8;
        for (int j =  quarter + eighths; j < middle; j++) {
            x = j % cols;
            if (x == cols -1 && y != rows/2) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                sum += distrib[i] * (float)img.get(y1, x);
            }
            temp_temp[y*cols + x] = sum;
        }

    } else if (n == 4) {
        y = rows / 2;
        for (int j = middle; j < middle + eighths; j++) {
            x = j % cols;
            if (x == cols -1 && y != (rows * 5)/8) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                sum += distrib[i] * (float)img.get(y1, x);
            }
            temp_temp[y*cols + x] = sum;
        }
        
    } else if (n == 5) {
        y = (rows * 5) / 8 ;
        for (int j = middle + eighths; j < middle + quarter; j++) {
            x = j % cols;
            if (x == cols -1 && y != (rows * 3)/4) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                sum += distrib[i] * (float)img.get(y1, x);
            }
            temp_temp[y*cols + x] = sum;
        }

    } else if (n == 6) {
        y = (rows * 3) / 4;
        for (int j = middle + quarter; j < middle + quarter + eighths; j++) {
            x = j % cols;
            if (x == cols -1 && y != (rows * 7)/8) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                sum += distrib[i] * (float)img.get(y1, x);
            }
            temp_temp[y*cols + x] = sum;
        }
    } else if (n == 7) {
        y = (rows * 7) / 8;
        for (int j = middle + quarter + eighths; j < (rows * cols); j++) {
            x = j % cols;
            if (x == cols -1 && y != rows-1) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                y1 = reflect(rows, y + shift);
                sum += distrib[i] * (float)img.get(y1, x);
            }
            temp_temp[y*cols + x] = sum;
        }

    }    

    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;

}



double Gaussian_Blur::convolve_eighths_x(Image & img, std::vector<float> & temp_temp,  Image &new_img, 
                                         float var, int n) {
                                         // std::vector<float> distrib, float var, int n) {

    double startTime = CycleTimer::currentSeconds();

    if (var == 0) {
        std::cout << "NOTE: Input variance = 0, no change to image";
        std::cout << std::endl;
        return 0.0;
    }
    else if (var >= MAX_VARIANCE) {
        std::cout << "Desired variance too high! Max is " << MAX_VARIANCE;
        std::cout << std::endl;
        return 0.0;
    }
    int rows = img.rows, cols = img.cols;
    std::vector<float> distrib;  // one-dimensional binomial distribution
    generate_binomial_distrib(variance_to_depth(var), distrib);
    int K = distrib.size();
    int mean_K = K / 2;


    int middle = (rows * cols) / 2;
    int quarter = (rows * cols) / 4;
    int eighths = (rows * cols) / 8;

    int x = 0, y = 0, x1, shift, new_val; 
    float sum;

    if (n == 0) {
        y = 0;
        for (int j = 0; j < quarter; j++) {
            x = j % cols;
            if (x == cols -1 && y != (rows/4)) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum += distrib[i] * temp_temp[y*cols + x1];
            }
            new_val = (int)sum;
            new_img.set(y, x, new_val);
        }
 
    } else if (n == 1) {
        y = rows/8;
        for (int j = quarter; j < middle; j++) {
            x = j % cols;
            if (x == cols -1 && y != rows/2) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum += distrib[i] * temp_temp[y*cols + x1];
            }
            new_val = (int)sum;
            new_img.set(y, x, new_val);
        }

    } else if (n == 2) {
        y = rows/4;
        for (int j = middle +1; j < middle + quarter; j++) {
            x = j % cols;
            if (x == cols -1 && y != (rows *(3/4))) y++;
            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum += distrib[i] * temp_temp[y*cols + x1];
            }
            new_val = (int)sum;
            new_img.set(y, x, new_val);
        }
    } else if (n == 3) {
        y = (rows * 3) / 8;
        for (int j = middle + quarter; j < (rows * cols); j++) {
            x = j % cols;
            if (x == cols -1 && y != rows-1) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum += distrib[i] * temp_temp[y*cols + x1];
            }
            new_val = (int)sum;
            new_img.set(y, x, new_val);
        }
    } else if (n == 4) {
        y = rows / 2;
        for (int j = middle; j < middle + eighths; j++) {
            x = j % cols;
            if (x == cols -1 && y != (rows * 5)/8) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum += distrib[i] * temp_temp[y*cols + x1];
            }
            new_val = (int)sum;
            new_img.set(y, x, new_val);
        }

    } else if (n == 5) {
        y = (rows * 5) / 8 ;
        for (int j = middle + eighths; j < middle + quarter; j++) {
            x = j % cols;
            if (x == cols -1 && y != (rows * 3)/4) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum += distrib[i] * temp_temp[y*cols + x1];
            }
            new_val = (int)sum;
            new_img.set(y, x, new_val);
        }

    } else if (n == 6) {
        y = (rows * 3) / 4;
        for (int j = middle + quarter; j < middle + quarter + eighths; j++) {
            x = j % cols;
            if (x == cols -1 && y != (rows * 7)/8) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum += distrib[i] * temp_temp[y*cols + x1];
            }
            new_val = (int)sum;
            new_img.set(y, x, new_val);
        }

    } else if (n == 7) {
        y = (rows * 7) / 8;
        for (int j = middle + quarter + eighths; j < (rows * cols); j++) {
            x = j % cols;
            if (x == cols -1 && y != rows-1) y++;

            sum = 0.0;
            for(int i = 0; i < K; i++){
                shift = i - mean_K;
                x1 = reflect(cols, x + shift);
                sum += distrib[i] * temp_temp[y*cols + x1];
            }
            new_val = (int)sum;
            new_img.set(y, x, new_val);
        }
    }


    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;

}