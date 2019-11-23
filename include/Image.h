#ifndef _IMAGE_
#define _IMAGE_

#include <vector>
#include <opencv2/highgui/highgui.hpp>

class Image {
public:
    int rows, cols;
    // not sure whether to use RGB or grayscale
    std::vector<int> data;
    Image();
    Image(int rows_x, int cols_x, int* data);
    Image(int rows_x, int cols_x);
    Image(cv::Mat & src);
    /* Copy constructor */
    Image(const Image &rhs);
    // NOTE: Handles negative values by rounding to 0
    friend Image operator-(const Image & lhs, const Image & rhs);
    /* Copy/assignment operator */
    Image & operator=(const Image & rhs);
    void store_opencv(cv::Mat & dst);
    int get(int row, int col) const;
    void set(int row, int col, int val);
};

#endif