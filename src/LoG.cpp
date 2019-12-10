#include "LoG.h"
#include "Image.h"
#include "GaussianBlur.h"
#include "general_helpers.h"
#include "CycleTimer.h"
#include "omp.h"

#include <vector>


#define DEBUG 1
#define MAX_VARIANCE 4

const float MAX_ERROR = 1e-05;


const std::vector<float> standard_variances({
    (float)(pow(2, 0.5) / 2.0),
    (float)(pow(2, 1) / 2.0),
    (float)(pow(2, 1.5) / 2.0),
    (float)(pow(2, 2) / 2.0),
    (float)(pow(2, 2.5) / 2.0)
});


// sigma^2 = np(1-p) = n/4 [p = 1/2 binomial distrib]
int variance_to_depth(float var) {
    return (int)ceilf(var * var * 4.0);
}


// Source: https://www.geeksforgeeks.org/pascal-triangle/
void generate_binomial_distrib(int n, 
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
    // if (DEBUG) assert(almost_equal(sum<float>(new_distrib), 1.0, MAX_ERROR));
}



double LoG::find_LoG_images(std::vector<Image> & first_octave_LoG,
        std::vector<Image> & second_octave_LoG,
        std::vector<Image> & third_octave_LoG,
        std::vector<Image> & fourth_octave_LoG) {

    double startTime = CycleTimer::currentSeconds();

    Image orig1 = orig;

    Image half(orig1.rows/2, orig.cols/2);
    Image quarter(orig1.rows/4, orig.cols/4);
    Image eighth(orig1.rows/8, orig.cols/8);

    double SIFT_TIME = 50000.;


    #pragma omp parallel sections shared(orig1)
    {
        #pragma omp section
        {
            SIFT_TIME = shrink_half(orig1, half);
            SIFT_TIME = create_blurs(second_octave_LoG, half);

        }
        
        #pragma omp section
        {
            SIFT_TIME = shrink_quarter(orig1, quarter);
            SIFT_TIME = create_blurs(third_octave_LoG, quarter);

        }
        #pragma omp section
        {
            SIFT_TIME = shrink_eighth(orig1, eighth);
            SIFT_TIME = create_blurs(fourth_octave_LoG, eighth);
        }

        #pragma omp section
        {
            SIFT_TIME = create_blurs(first_octave_LoG, orig1);
        }
            
    }


    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;
}


double LoG::create_blurs(std::vector<Image> & result, Image & src) {

    double startTime = CycleTimer::currentSeconds();

    Gaussian_Blur gb;
    Image temp(src.rows, src.cols);
    // float temp_temp[src.rows * src.cols]; 
    std::vector<float> temp_temp;
    int idx;
    Image prev_scale = src;
    double SIFT_TIME = 50000.;

    temp_temp.reserve(src.rows * src.cols);
    for (int r = 0; r < src.rows; r++) {
        for (int c = 0; c < src.cols; c++) {
            idx = r * src.cols + c;
            temp_temp.push_back(0.0);
        }
    }

    // printf("%d\n", standard_variances.size());
    for (int i = 0; i < standard_variances.size(); i++) {

        float var = standard_variances[i];
        // std::vector<float> distrib;  // one-dimensional binomial distribution
        // generate_binomial_distrib(variance_to_depth(var), distrib);

        // std::cout << "Distrib for "  << var << std::endl;
        // print_arr<float>(distrib);
        // std::cout << std::endl;

        // convolve source image with some variance

        #pragma omp parallel shared(src, temp_temp, temp)
        {
            #pragma omp single
            {
                #pragma omp task untied 
                SIFT_TIME = gb.convolve_half_y(src, temp_temp, temp, var, 0);
                #pragma omp task untied 
                SIFT_TIME = gb.convolve_half_y(src, temp_temp, temp, var, 1);
            }
        }

        #pragma omp parallel shared(src, temp)
        {
            #pragma omp single
            {
                #pragma omp task untied 
                SIFT_TIME = gb.convolve_half_x(src, temp_temp, temp, var, 0);
                #pragma omp task untied  
                SIFT_TIME = gb.convolve_half_x(src, temp_temp, temp, var, 1);
            }
        }


        // subtract from previous scale to obtain gaussian difference
        result.push_back(prev_scale - temp);
        // result.push_back(temp);
        prev_scale = temp;
    }


    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;

}

LoG::LoG(Image & src) : orig(src) {} 