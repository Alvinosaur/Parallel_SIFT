#include "Keypoint.h"
#include "general_helpers.h"
#include <iostream>
#include <assert.h>

#define SWINDOW_X 5
#define SWINDOW_Y 5
#define SWINDOW_X_NUM 4
#define SWINDOW_Y_NUM 4


int checkBoundary(int M, int x) {
    if(x < 0) return 0;
    if(x >= M) return M - 1;
    return x;
}


void create_window(Point cur, Image window, std::vector<Image> & allScales) {

	int curScale = cur.scale_index;
	Image curImage = allScales[curScale];

	int curRow, curCol = cur.row, cur.col;
	
	std::vector<float> mag(16);
	std::vector<float> ori(16);


	//for 16 x 16 window
	for (int r = -2; r < 2; r++) {
		for (int c = -2; c < 2; c++) {


			int xOffset = curCol + c * SWINDOW_X;
			int yOffset = curRow + r * SWINDOW_Y;


			std::vector<float> smag(16);  
			std::vector<float> sori(16);  


			// for 4 x 4 window
			for (int ir = 0; ir < SWINDOW_Y; ir++) {
				for (int ic = 0; ic < SWINDOW_X; ic++){

					int xOffset_I = xOffset + ic;
					int yOffset_I = yOffset + ir;


					for (int i = -1; i < 1; i++) {
						for (int j = -1; j < 1; j++) {
							
							int x1 = xOffset_I + j;
							int y1 = yOffset_I + i;

							int curMag, curOri;

							xN = checkBoundary(curImage.cols, xOffset_I);
							yN = checkBoundary(curImage.rows, yOffset_I -1);
							xS = checkBoundary(curImage.cols, xOffset_I);
							yS = checkBoundary(curImage.rows, yOffset_I +1);
							xW = checkBoundary(curImage.cols, xOffset_I -1);
							yW = checkBoundary(curImage.rows, yOffset_I);
							xE = checkBoundary(curImage.cols, xOffset_I +1);
							yE = checkBoundary(curImage.rows, yOffset_I);
							

							if (!(x1 == 0 && y == 0)) {

								curMag = sqrt(  pow(curScale.get(yN, xN) - curScale.get(yS, xS), 2)
								              + pow(curScale.get(yW, xW) - curScale.get(yE, xE), 2));
								curOri = atan2((curScale.get(yN, xN) - curScale.get(yN, xN)) /
						                       (curScale.get(yW, xW) - curScale.get(yE, xE))) * 180 / PI;
								
								smag[ir*4 + ic] = curMag;
								sori[ir*4 + ic] = curOri;
							}

						}
					}


					//get gradient of small 4x4 

					for (int b = 0; b < 16; b++) {

					}



				}
			}




		}
	}

}



void Feature_generation(std::vector<Point> & keypoint_list_final, )
