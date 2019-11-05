#indef _LOG_
#define _LOG_

#include "Image.h"
#include <vector>
#include <utility>


class LoG {
//     std::vector<std::vector<int>> distribs;
//     std::vector<std::vector<int>> variances;
//     int variance_to_depth(float var);
// public:
//     void generate_binomial_distrib(int n, std::vector<int> & new_distrib);
//     int generate_kernel(std::vector<int> & kernel, float var);
//     void convolve(Image & img, float var);
public:
	void create_differences(Image & img1, Image & img2, Image & res);
	void get_DoG(Image & img, float var, std::vector<Image> & differences);

};

#endif