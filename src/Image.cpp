#include <iostream>
#include <vector>
#include <assert.h>
#include "Image.h"

Image::Image(int rows_x, int cols_x, int* data_x) : rows(rows_x), cols(cols_x)
{
    int idx;
    assert(data_x != NULL);
    data.reserve(rows_x * cols_x);
    for (int r = 0; r < rows_x; r++) {
        for (int c = 0; c < cols_x; c++) {
            idx = r * cols_x + c;
            data.push_back(data_x[idx]);
        }
    }
}


Image & Image::operator=(const Image & rhs) {
    assert(this->rows == rhs.rows && this->cols == rhs.cols);
    assert(this != &rhs);
    int idx;
    for (int r = 0; r < rhs.rows; r++) {
        for (int c = 0; c < rhs.cols; c++) {
            idx = r * rhs.cols + c;
            this->data.push_back(rhs.data[idx]);
        }
    }
    return *this;
}

Image operator-(const Image & lhs, const Image & rhs) {
    assert(lhs.rows == rhs.rows && lhs.cols == rhs.cols);
    Image subt_res(lhs.rows, lhs.cols);
    int idx;
    for (int r = 0; r < lhs.rows; r++) {
        for (int c = 0; c < lhs.cols; c++) {
            idx = r * lhs.cols + c;
            subt_res.data.push_back(
                lhs.data[idx] - rhs.data[idx]);
        }
    }
    return subt_res;
}

Image::Image(int rows_x, int cols_x) : rows(rows_x), cols(cols_x)
{
    data.reserve(rows_x * cols_x);
}

Image::Image(cv::Mat & src) {
    rows = src.rows;
    cols = src.cols;
    for (int r = 0; r < src.rows; r++) {
        for (int c = 0; c < src.cols; c++) {
            data.push_back(src.at<uchar>(r, c));
        }
    }
}

void Image::store_opencv(cv::Mat & dst) {
    dst.create(rows, cols, CV_8UC(1));
    int idx;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            idx = r * cols + c;
            dst.at<uchar>(r, c) = data[idx];
        }
    }
}

int Image::get(int row, int col) {
    assert(0 <= row && row < rows &&
           0 <= col && col < cols);
    return data[row * cols + col];
}

void Image::set(int row, int col, int val) {
    assert(0 <= row && row < rows &&
           0 <= col && col < cols);
    data[row*cols + col] = val;
}