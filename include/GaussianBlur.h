#ifndef _GBLUR_
#define _GBLUR_

#include "Image.h"
#include <vector>
#include <utility>

// First step: write function to generate any gaussian blur given
// some variance sigma value
// Figure out how to do horizontal and vertical gaussian blurs separately
// 

class Gaussian_Blur {
    std::vector<std::vector<int>> distribs;
    std::vector<std::vector<int>> variances;
    int variance_to_depth(float var);
public:
    void generate_binomial_distrib(int n, std::vector<float> & new_distrib);
    int generate_kernel(std::vector<int> & kernel, float var);
    double convolve(Image & img, Image & new_img, float var);
 //    double convolve_quarters_x(Image & img, float temp_temp[], Image & new_img, std::vector<float> distrib, float var, int n);
 //    double convolve_quarters_y(Image & img, float temp_temp[], Image & new_img, std::vector<float> distrib, float var, int n);
	// double convolve_eighths_x(Image & img, float temp_temp[], Image & new_img, std::vector<float> distrib, float var, int n);
	// double convolve_eighths_y(Image & img, float temp_temp[], Image & new_img, std::vector<float> distrib, float var, int n);

    double convolve_half_x(Image & img, std::vector<float> & temp_temp, Image & new_img, float var, int n);
    double convolve_half_y(Image & img, std::vector<float> & temp_temp, Image & new_img, float var, int n);
    double convolve_quarters_x(Image & img, std::vector<float> & temp_temp, Image & new_img, float var, int n);
    double convolve_quarters_y(Image & img, std::vector<float> & temp_temp, Image & new_img, float var, int n);
    double convolve_eighths_x(Image & img, std::vector<float> & temp_temp, Image & new_img, float var, int n);
    double convolve_eighths_y(Image & img, std::vector<float> & temp_temp, Image & new_img, float var, int n);

};

#endif