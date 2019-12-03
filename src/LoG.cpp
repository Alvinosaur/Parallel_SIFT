#include "LoG.h"
#include "Image.h"
#include "GaussianBlur.h"
#include "general_helpers.h"
#include "CycleTimer.h"
#include <omp.h>



#include <vector>

const std::vector<float> standard_variances({
    (float)(pow(2, 0.5) / 2.0),
    (float)(pow(2, 1) / 2.0),
    (float)(pow(2, 1.5) / 2.0),
    (float)(pow(2, 2) / 2.0),
    (float)(pow(2, 2.5) / 2.0)
});

double LoG::find_LoG_images(std::vector<Image> & first_octave_LoG,
        std::vector<Image> & second_octave_LoG,
        std::vector<Image> & third_octave_LoG,
        std::vector<Image> & fourth_octave_LoG) {

    double startTime = CycleTimer::currentSeconds();

    Image half(orig.rows/2, orig.cols/2);
    Image quarter(orig.rows/4, orig.cols/4);
    Image eighth(orig.rows/8, orig.cols/8);

    shrink_half(orig, half);
    shrink_half(half, quarter);
    shrink_half(quarter, eighth);

    create_blurs(first_octave_LoG, orig);
    create_blurs(second_octave_LoG, half);
    create_blurs(third_octave_LoG, quarter);
    create_blurs(fourth_octave_LoG, eighth);

    double endTime = CycleTimer::currentSeconds();
    double overallTime = endTime - startTime;
    return overallTime;
}

void LoG::create_blurs(std::vector<Image> & result, Image & src) {
    Gaussian_Blur gb;
    Image temp(src.rows, src.cols);
    Image prev_scale = src;

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < standard_variances.size(); i++) {
        float var = standard_variances[i];
        // convolve source image with some variance
        gb.convolve(src, temp, var);
        // subtract from previous scale to obtain gaussian difference
        result.push_back(prev_scale - temp);
        prev_scale = temp;
    }
}

LoG::LoG(Image & src) : orig(src) {}
