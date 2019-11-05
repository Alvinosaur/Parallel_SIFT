#include "LoG.h"
#include "Image.h"
#include <vector>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        cout << "Need to pass in image filepath!" << endl;
        exit(-1);
    }

    vector<int> vars({1});
    Gaussian_Blur gb;
    Mat src = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    cout << src.rows << ", " << src.cols << endl;

    waitKey(0);

    return 0;

}