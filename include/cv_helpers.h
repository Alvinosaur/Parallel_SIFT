#ifndef _CV_HELPERS_
#define _CV_HELPERS_

#include "Image.h"
#include "GaussianBlur.h"

extern const std::vector<float> standard_variances;

void find_DoG_images(std::vector<Image> & blurred_images);

void shrink_half(Image & src, Image & dst);

#endif