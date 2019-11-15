#ifndef _POINT_
#define _POINT_

#include <vector>
#include <opencv2/highgui/highgui.hpp>

class Image {
public:
	int scale_index;
    int row, col;
    // not sure whether to use RGB or grayscale
    int mag;
    int ori;
    Point(int scale_index, int row, int col, int mag, ori);
    
};

#endif
