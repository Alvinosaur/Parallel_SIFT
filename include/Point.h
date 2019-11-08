#ifndef _POINT_
#define _POINT_

#include <vector>
#include <opencv2/highgui/highgui.hpp>

class Image {
public:
	int scale_index;
    int row, col;
    // not sure whether to use RGB or grayscale
    int value;
    Point(int row, int col, int value);
    
};

#endif
