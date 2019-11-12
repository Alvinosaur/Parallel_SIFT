#include "Image.h"

void shrink_half(Image & src, Image & dst) {
    int total_val;
    for (int i = 0; i < src.rows-1; i+=2) {
        for (int j = 0; j < src.cols-1; j+=2) {
            total_val = (src.get(i, j) + src.get(i+1, j) + 
                src.get(i, j+1) + src.get(i+1, j+1));
            dst.set(i/2, j/2, (int)((float)total_val / 4.0));
        }
    }
}

// void laplacian_of_gaussian(Img & src) {

// }