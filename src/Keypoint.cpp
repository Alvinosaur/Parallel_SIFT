#include "Keypoint.h"
#include <iostream>



int reflect(int M, int x) {
    if(x < 0) return -x - 1;
    if(x >= M) return 2*M - x - 1;
    return x;
}


void getMaxes(int scaleID, Image & img1, Image & img2, Image & img3, std::vector<Point> & keypoint_list) {

    int row, col, rows = img1.rows, cols = img1.cols;

	for (int y = 1; y < rows - 1; y++) {
		for (int x = 1; x < cols - 1; x++) {

			// x1 = reflect
			// y1 = reflect

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

				Point cur;
				cur.scaleID = scaleID;
				cur.row = x1, cur.col = y1;

				keypoint_list.pushback(cur);
			}

		}

	}
}



void Keypoint_find(float var, std::vector<Image> & differences, std::vector<Point> & keypoint_list_total) {

	Image Top_scale = differences[0];
	Image Bottom_scale = differences[var -1];



	for (int i = 1; i < var -1; i++) {

		std::vector<Point> keypoint_list_scale;
		getMaxes(differences[i -1], differences[i], differences[i +1], keypoint_list_scale);

		for (int j = 0; j < keypoint_list_scale.size(); j++) {
			keypoint_list_total.pushback(keypoint_list_scale[j]);
		}
	}



}


