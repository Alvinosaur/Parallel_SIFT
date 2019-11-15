#include "Keypoint.h"
#include <iostream>
#include <math.h>  

#define PI 3.14159265



void putBins(float mag, float ori, std::vector<float> & bins) {

	int bin_index = (ori / 10);

	if (bin_index == 36) {
		bins[35] += mag;
	} else {
		bins[bin_index] += mag;
	}

}

// http://aishack.in/tutorials/sift-scale-invariant-feature-transform-keypoints/
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


// http://aishack.in/tutorials/sift-scale-invariant-feature-transform-keypoints/
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


// http://aishack.in/tutorials/sift-scale-invariant-feature-transform-eliminate-low-contrast/
/*
   keypoint_list_total is the list of keypoints that remain after removing non-corner keypoints
*/
void Keypoint_remove(float threshhold, std::vector<Image> & allScales, std::vector<Point> & keypoint_list_total) {

	for (int i = 0; i < keypoint_list_total.size(); i++) {

		Image curScale = allScales[keypoint_list_total[i].scale_index];
		int curRow keypoint_list[i].row , curCol = keypoint_list[i].col;

		float dy = curScale.get(curRow - 1, curCol) - curScale.get(curRow + 1, curCol);
		float dx = curScale.get(curRow, curCol - 1) - curScale.get(curRow, curCol + 1);


		if (dy < threshhold | dx < threshhold) {
			keypoint_list_total.erase(keypoint_list_total.begin() + i);
		}

	}

}


// http://aishack.in/tutorials/sift-scale-invariant-feature-transform-keypoint-orientation/
/*
   keypoint_list_total is the list of keypoints from intermediary steps
   keypoint_list_final is the list of keypoints thats the outcome of the rotation invariance step
*/
void Keypoint_orientation(std::vector<Image> & allScales, std::vector<Point> & keypoint_list_total,  std::vector<Point> & keypoint_list_final) {


	for (int i = 0; i < keypoint_list_total.size(); i++) {

		float mag, mag;
		float max = 0;
		std::vector<float> bins(36, 0.0);  //36 for every 10 degrees in 360

		Point cur = keypoint_list_total[i];
		int curScaleID = cur.scale_index]
		Image curScale = allScales[curScaleID];
		int curRow keypoint_list[i].row , curCol = keypoint_list[i].col;



		for (int x = 0; x < 3; x++) {
			for (int y = 0; y < 3; y++) {

				if (!(x == 2 && y == 2)) {

					mag = sqrt(  pow(curScale.get(curRow - 1, curCol) - curScale.get(curRow + 1, curCol), 2)
					           + pow(curScale.get(curRow, curCol - 1) - curScale.get(curRow, curCol + 1), 2));
					ori = atan2((curScale.get(curRow, curCol - 1) - curScale.get(curRow, curCol + 1)) /
			                    (curScale.get(curRow - 1, curCol) - curScale.get(curRow + 1, curCol))) * 180 / PI;
					
					putBins(mag, ori, bins);
				}
			}
		}

		//calculate max
		for (int b = 0; b < 36; b++) {
			if (bins[b].mag > max) {
				max = bins[b];
				max_index = b;
			}
		}

		for (int h = 0; h < 36; h++) {
			if (bins[h] >= (max *4/5)) {
				Point newPoint(curScaleID, curRow, curCol, bins[h], h*10);
				keypoint_list_final.pushback(newPoint);
			}
		}



	}
	
}








