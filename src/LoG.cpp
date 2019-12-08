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

    SIFT_TIME = shrink_half(orig1, half);
    printf("shrink_half:            %.3f ms\n", 1000.f * SIFT_TIME);
    SIFT_TIME = create_blurs(second_octave_LoG, half);
    printf("blur_hafl:              %.3f ms\n", 1000.f *SIFT_TIME);

    SIFT_TIME = shrink_quarter(orig1, quarter);
    printf("shrink_quar:            %.3f ms\n", 1000.f *SIFT_TIME);
    SIFT_TIME = create_blurs(third_octave_LoG, quarter);
    printf("blur_quart:             %.3f ms\n", 1000.f * SIFT_TIME);


    SIFT_TIME = shrink_eighth(orig1, eighth);
    printf("shrink_eigh:            %.3f ms\n", 1000.f * SIFT_TIME);
    SIFT_TIME = create_blurs(fourth_octave_LoG, eighth);
    printf("blur_eight:             %.3f ms\n", 1000.f * SIFT_TIME);

    SIFT_TIME = create_blurs(first_octave_LoG, orig1);
    printf("blur_orig:              %.3f ms\n", 1000.f * SIFT_TIME);


    // #pragma omp parallel sections shared(orig1)
    // {
 

    //     #pragma omp section
    //     {
    //         SIFT_TIME = shrink_half(orig1, half);
    //         printf("shrink_half:                    %.3f ms\n", 1000.f * SIFT_TIME);
    //         SIFT_TIME = create_blurs(second_octave_LoG, half);
    //         printf("blur_hafl:                      %.3f ms\n", 1000.f *SIFT_TIME);

    //     }
        
    //     #pragma omp section
    //     {
    //         SIFT_TIME = shrink_quarter(orig1, quarter);
    //         printf("shrink_quar:                    %.3f ms\n", 1000.f *SIFT_TIME);
    //         SIFT_TIME = create_blurs(third_octave_LoG, quarter);
    //         printf("blur_quart:                     %.3f ms\n", 1000.f * SIFT_TIME);

    //     }
    //     #pragma omp section
    //     {
    //         SIFT_TIME = shrink_eighth(orig1, eighth);
    //         printf("shrink_eigh:                    %.3f ms\n", 1000.f * SIFT_TIME);
    //         SIFT_TIME = create_blurs(fourth_octave_LoG, eighth);
    //         printf("blur_eight:                     %.3f ms\n", 1000.f * SIFT_TIME);
    //     }

    //     #pragma omp section
    //     {
    //         SIFT_TIME = create_blurs(first_octave_LoG, orig1);
    //         printf("blur_orig:                      %.3f ms\n", 1000.f * SIFT_TIME);
    //     }
            

    // }

    // #pragma omp parallel 
    // {
    //     #pragma omp single
    //     {
    //         #pragma omp task
    //         create_blurs(second_octave_LoG, half);
    //         #pragma omp task
    //         create_blurs(third_octave_LoG, quarter);
    //         #pragma omp task
    //         create_blurs(fourth_octave_LoG, eighth);
    //         #pragma omp task
    //         create_blurs(first_octave_LoG, orig);

    //     }
    // }


    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;
}


double LoG::create_blurs(std::vector<Image> & result, Image & src) {

    double startTime = CycleTimer::currentSeconds();

    Gaussian_Blur gb;
    Image temp(src.rows, src.cols);
    float temp_temp[src.rows * src.cols]; 
    Image prev_scale = src;
    std::vector<float> distrib;  // one-dimensional binomial distribution
    double SIFT_TIME = 50000.;

    // printf("%d\n", standard_variances.size());
    for (int i = 0; i < standard_variances.size(); i++) {

        float var = standard_variances[i];
        generate_binomial_distrib(variance_to_depth(var), distrib);

        std::cout << "Distrib for "  << var << std::endl;
        print_arr<float>(distrib);
        std::cout << std::endl;

        // convolve source image with some variance
        // #pragma omp parallel sections shared(src, temp)
        // {
        
        //     #pragma omp section
        //     {
        //         SIFT_TIME = gb.convolve_quarters(src, temp, var, 0);
        //         printf("convolve for %d on %f: %.3f ms\n", i, var, 1000.f * SIFT_TIME);
        //     }
        //     #pragma omp section
        //     {
        //         SIFT_TIME = gb.convolve_quarters(src, temp, var, 1);
        //         printf("convolve for %d on %f: %.3f ms\n", i, var, 1000.f * SIFT_TIME);
        //     }
        //     #pragma omp section
        //     {
        //         SIFT_TIME = gb.convolve_quarters(src, temp, var, 2);
        //         printf("convolve for %d on %f: %.3f ms\n", i, var, 1000.f * SIFT_TIME);
        //     }
        //     #pragma omp section
        //     {
        //         SIFT_TIME = gb.convolve_quarters(src, temp, var, 3);
        //         printf("convolve for %d on %f: %.3f ms\n", i, var, 1000.f * SIFT_TIME);
        //     }
            
        // }


        // #pragma omp parallel shared(src, temp, temp_temp) 
        // {
        //     #pragma omp single
        //     {
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, var, 0);
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, var, 1);
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, var, 2);
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, var, 3);
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, var, 4);
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, var, 5);
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, var, 6);
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, var, 7);

        //     }
        // }

        // #pragma omp parallel shared(src, temp, temp_temp) 
        // {
        //     #pragma omp single
        //     {
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, var, 0);
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, var, 1);
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, var, 2);
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, var, 3);
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, var, 4);
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, var, 5);
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, var, 6);
        //         #pragma omp task
        //         SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, var, 7);

        //     }
        // }

        
        // SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, distrib, var, 0);
        // SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, distrib, var, 1);
        // SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, distrib, var, 2);
        // SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, distrib, var, 3);
        // SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, distrib, var, 4);
        // SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, distrib, var, 5);
        // SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, distrib, var, 6);
        // SIFT_TIME = gb.convolve_eighths_y(src, temp_temp, temp, distrib, var, 7);

        // SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, distrib, var, 0);
        // SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, distrib, var, 1);
        // SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, distrib, var, 2);
        // SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, distrib, var, 3);
        // SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, distrib, var, 4);
        // SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, distrib, var, 5);
        // SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, distrib, var, 6);
        // SIFT_TIME = gb.convolve_eighths_x(src, temp_temp, temp, distrib, var, 7);



        SIFT_TIME = gb.convolve_quarters_y(src, temp_temp, temp, distrib, var, 0);
        SIFT_TIME = gb.convolve_quarters_y(src, temp_temp, temp, distrib, var, 1);
        SIFT_TIME = gb.convolve_quarters_y(src, temp_temp, temp, distrib, var, 2);
        SIFT_TIME = gb.convolve_quarters_y(src, temp_temp, temp, distrib, var, 3);

        SIFT_TIME = gb.convolve_quarters_x(src, temp_temp, temp, distrib, var, 0);
        SIFT_TIME = gb.convolve_quarters_x(src, temp_temp, temp, distrib, var, 1);
        SIFT_TIME = gb.convolve_quarters_x(src, temp_temp, temp, distrib, var, 2);
        SIFT_TIME = gb.convolve_quarters_x(src, temp_temp, temp, distrib, var, 3);


        // SIFT_TIME = gb.convolve_half(src, temp, var, 0);
        // printf("convolve for %d on %f: %.3f ms\n", i, var, 1000.f * SIFT_TIME);
        // SIFT_TIME = gb.convolve_half(src, temp, var, 1);
        // printf("convolve for %d on %f: %.3f ms\n", i, var, 1000.f * SIFT_TIME);


        // SIFT_TIME = gb.convolve(src, temp, var);
        // printf("convolve for %d on %f: %.3f ms\n", i, var, 1000.f * SIFT_TIME);


        // subtract from previous scale to obtain gaussian difference
        // result.push_back(prev_scale - temp);
        result.push_back(temp);
        prev_scale = temp;
    }


    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;

}

LoG::LoG(Image & src) : orig(src) {} 