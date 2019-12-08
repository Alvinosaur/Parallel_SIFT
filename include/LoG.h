#ifndef _LOG_
#define _LOG_

#include "Image.h"
#include "mpi.h"

extern const std::vector<float> standard_variances;

class LoG {
    Image orig;
    int num_tasks;
    int rank;
    MPI_Request* reqs;
    MPI_Status* stats;
public:
    LoG(Image & src, int num_tasks, int rank, MPI_Request* reqs, 
        MPI_Status* stats);
    double find_LoG_images(std::vector<Image> & first_octave_LoG,
                        std::vector<Image> & second_octave_LoG, 
                        std::vector<Image> & third_octave_LoG,
                        std::vector<Image> & fourth_octave_LoG);
    void create_blurs(std::vector<Image> & result, Image & src,
        std::vector<std::vector<float>> & conv_distribs);
    void create_blurs_mpi(std::vector<Image> & result, Image & src, 
        int num_tasks, int rank, MPI_Request* reqs, MPI_Status* stats);
    void perform_shrinks(int* half_temp, int* quarter_temp, int* eighth_temp);
};

#endif