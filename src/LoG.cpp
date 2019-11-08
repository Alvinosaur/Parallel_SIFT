#include "GaussianBlur.h"
#include <iostream>
#include <opencv2/highgui/highgui.hpp>





void LoG::create_differences(Image & img1, Image & img2, Image & res) {

	int row1, col1, rows1 = img1.rows, cols1 = img1.cols;
	int row2, col2, rows2 = img2.rows, cols2 = img2.cols;

    int temp[rows * cols];  

    for(y = 0; y < rows1; y++){
        for(x = 0; x < cols1; x++){
            res.set(y, x, img1.get(y, x) - img2.get(y, x));
        }
    }

}


void LoG::get_DoG(float var, std::vector<Image> & differences) {

	differences.reserve(var -1);

	std::vector<Image> images;
	images.reserve(var);

	for (int i = 0; i < var; i++) {
		Gaussian_Blur gb;

		Image cur;
		gb.convolve(img, variance, cur);
		images.pushback(cur);
	}


	for (int j = 0; j < var - 1; j++) {

		Image cur_diff;
		create_differences(images[j], images[j +1], cur_diff);
		differences.pushback(cur_diff);
	}
}

