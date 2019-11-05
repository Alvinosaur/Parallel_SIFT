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
    void generate_binomial_distrib(int n, std::vector<int> & new_distrib);
    int generate_kernel(std::vector<int> & kernel, float var);
    void convolve(Image & img, float var);
};

#endif