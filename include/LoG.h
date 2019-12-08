#ifndef _LOG_
#define _LOG_

#include "Image.h"

extern const std::vector<float> standard_variances;

class LoG {
    Image orig;
public:
    LoG(Image & src);
    double find_LoG_images(std::vector<Image> & first_octave_LoG,
                        std::vector<Image> & second_octave_LoG, 
                        std::vector<Image> & third_octave_LoG,
                        std::vector<Image> & fourth_octave_LoG);
    double create_blurs(std::vector<Image> & result, Image & src);
};

#endif