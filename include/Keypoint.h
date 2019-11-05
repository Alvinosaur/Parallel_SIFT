#indef _KEY_
#define _KEY_

#include "Image.h"
#include <vector>
#include <utility>


class KEY {
//     std::vector<std::vector<int>> distribs;
//     std::vector<std::vector<int>> variances;
//     int variance_to_depth(float var);
// public:
//     void generate_binomial_distrib(int n, std::vector<int> & new_distrib);
//     int generate_kernel(std::vector<int> & kernel, float var);
//     void convolve(Image & img, float var);
public:
	void findKeypoint(Image & img1, Image & img2, Image & img3, );
	void findMax(std::vector<int> & points, );

};

#endif 