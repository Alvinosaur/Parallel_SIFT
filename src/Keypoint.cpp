#include "Keypoint.h"
#include <iostream>



int reflect(int M, int x) {
    if(x < 0) return -x - 1;
    if(x >= M) return 2*M - x - 1;
    return x;
}


void getMaxes(Image & img1, Image & img2, Image & img3, ) {

    int row, col, rows = img1.rows, cols = img1.cols;

	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < cols; x++) {

			x1 = reflect
			y1 = reflect

			//if this, then mark point as local max
			if (img2.get(x1, y1) >= 0 && 
				//previous scale
				(img2.get(x1, y1) >= img1.get(x1-1, y1-1)) && (img2.get(x1, y1) >= img1.get(x1, y1-1)) && (img2.get(x1, y1) >= img1.get(x1+1, y1-1)) &&
				(img2.get(x1, y1) >= img1.get(x1-1, y1)) && (img2.get(x1, y1) >= img1.get(x1, y1)) && (img2.get(x1, y1) >= img1.get(x1+1, y1)) &&
				(img2.get(x1, y1) >= img1.get(x1-1, y1+1)) && (img2.get(x1, y1) >= img1.get(x1, y1+1)) && (img2.get(x1, y1) >= img1.get(x1+1, y1+1)) &&
				//current scale
				(img2.get(x1, y1) >= img2.get(x1-1, y1-1)) && (img2.get(x1, y1) >= img2.get(x1, y1-1)) && (img2.get(x1, y1) >= img2.get(x1+1, y1-1)) &&
				(img2.get(x1, y1) >= img2.get(x1-1, y1)) && (img2.get(x1, y1) >= img1.get(x1+1, y1)) &&
				(img2.get(x1, y1) >= img2.get(x1-1, y1+1)) && (img2.get(x1, y1) >= img2.get(x1, y1+1)) && (img2.get(x1, y1) >= img2.get(x1+1, y1+1)) &&
				//next scale
				(img2.get(x1, y1) >= img3.get(x1-1, y1-1)) && (img2.get(x1, y1) >= img3.get(x1, y1-1)) && (img2.get(x1, y1) >= img3.get(x1+1, y1-1)) &&
				(img2.get(x1, y1) >= img3.get(x1-1, y1)) && (img2.get(x1, y1) >= img3.get(x1, y1)) && (img2.get(x1, y1) >=img1.get(x1+1, y1)) &&
				(img2.get(x1, y1) >= img3.get(x1-1, y1+1)) && (img2.get(x1, y1) >= img3.get(x1, y1+1)) && (img2.get(x1, y1) >= img3.get(x1+1, y1+1)) &&
				) {
				//how to mark???
			}

		}

	}
}


void findMax( ) {

}


void Keypoint_find(float var, std::vector<Image> & differences) {

	Image Top_scale = differences[0];
	Image Bottom_scale = differences[var -1];

	for (int i = 1; i < var -1; i++) {

		getMaxes(differences[i -1], differences[i], differences[i +1], );
	}

}