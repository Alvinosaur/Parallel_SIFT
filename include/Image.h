#ifndef _IMAGE_
#define _IMAGE_

#include <vector>
#include <opencv2/highgui/highgui.hpp>

class Image {
public:
    int rows, cols;
    // not sure whether to use RGB or grayscale
    std::vector<int> data;
    Image(int rows_x, int cols_x, int* data);
    Image(cv::Mat & src);
    void store_opencv(cv::Mat & dst);
    int get(int row, int col);
    void set(int row, int col, int val);
};

#endif