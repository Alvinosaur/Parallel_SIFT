#include "LoG.h"
#include "Image.h"
#include "GaussianBlur.h"
#include "general_helpers.h"
#include "CycleTimer.h"
#include "mpi.h"
#include <vector>

const std::vector<float> standard_variances({
    (float)(pow(2, 0.5) / 2.0),
    (float)(pow(2, 1) / 2.0),
    (float)(pow(2, 1.5) / 2.0),
    (float)(pow(2, 2) / 2.0),
    (float)(pow(2, 2.5) / 2.0)
});

double LoG::find_LoG_images(std::vector<Image> & first_octave_LoG,
        std::vector<Image> & second_octave_LoG,
        std::vector<Image> & third_octave_LoG,
        std::vector<Image> & fourth_octave_LoG) {

    double startTime = CycleTimer::currentSeconds();

    // Each task holds local array to hold results received from other tasks
    int half_temp[orig.rows/2 * orig.cols/2];
    int quarter_temp[orig.rows/4 * orig.cols/4];
    int eighth_temp[orig.rows/8 * orig.cols/8];

    perform_shrinks(half_temp, quarter_temp, eighth_temp);

    Image half(orig.rows/2, orig.cols/2, half_temp);
    Image quarter(orig.rows/4, orig.cols/4, quarter_temp);
    Image eighth(orig.rows/8, orig.cols/8, eighth_temp);

    // Precompute weight distributions used for blur convolution
    Gaussian_Blur gb;
    std::vector<std::vector<float>> conv_distribs;
    conv_distribs.reserve(standard_variances.size());
    gb.precompute_distributions(conv_distribs);

    create_blurs(first_octave_LoG, orig, conv_distribs);
    create_blurs(second_octave_LoG, half, conv_distribs);
    create_blurs(third_octave_LoG, quarter, conv_distribs);
    create_blurs(fourth_octave_LoG, eighth, conv_distribs);

    double endTime = CycleTimer::currentSeconds();

    double overallTime = endTime - startTime;
    return overallTime;
}

void LoG::create_blurs(std::vector<Image> & result, Image & src,
        std::vector<std::vector<float>> & conv_distribs) {
    Gaussian_Blur gb;
    Image prev_scale = src;

    // Each task allocates new work to be done
    std::vector<range> row_assignments, row_pix_assignments;
    allocate_conv_rows_mpi(src.rows, src.cols, num_tasks,
        row_assignments, row_pix_assignments);

    // Each task holds local array to hold results received from other tasks
    int conv_row_results[src.rows * src.cols];
    int conv_col_results[src.rows * src.cols];

    // Each task performs its own work in parallel and sends results
    // non-blocking and receives non-blocking
    std::vector<Image> conv_results;
    for (int var_i = 0; var_i < standard_variances.size(); var_i++) {

        // Perform row-convolutions first, send results to other processes
        gb.convolve_rows_mpi(src, conv_row_results,
            row_assignments[rank], conv_distribs[var_i]);
        send_to_others(conv_row_results, reqs, rank, row_pix_assignments[rank],
            var_i, num_tasks);
        receive_from_others(conv_row_results, reqs, row_pix_assignments,
            rank, var_i);

        // Barrier need to have received all row conv before starting col conv
        mpi_barrier(rank, num_tasks, reqs, stats);

        // Now perform column-convolutions using received row-convolutions
        gb.convolve_cols_mpi(conv_row_results, conv_col_results,
            row_assignments[rank], conv_distribs[var_i], src.cols);
        send_to_others(conv_col_results, reqs, rank, row_pix_assignments[rank],
            var_i, num_tasks);
        receive_from_others(conv_col_results, reqs, row_pix_assignments, rank,
            var_i);

        // Barrier: must receive and store all data before reusing arrays
        mpi_barrier(rank, num_tasks, reqs, stats);

        // Store result in image before reusing temporary result arrays
        Image new_conv(src.rows, src.cols, conv_col_results);
        result.push_back(prev_scale - new_conv);
        prev_scale = new_conv;
    }

    // Barrier: Make sure all threads finished
    mpi_barrier(rank, num_tasks, reqs, stats);
}

void LoG::perform_shrinks(int* half_temp, int* quarter_temp, int* eighth_temp) {
    std::vector<range> half_assignments;
    std::vector<range> quarter_assignments;
    std::vector<range> eighth_assignments;
    allocate_shrink_work_mpi(orig.rows, orig.cols, num_tasks,
        half_assignments, quarter_assignments,
        eighth_assignments);

    // Each task performs its own work in parallel and sends results
    // non-blocking and receives non-blocking
    shrink_mpi(orig, half_temp, half_assignments[rank], HALF);
    send_to_others(half_temp, reqs, rank, half_assignments[rank], HALF, num_tasks);
    receive_from_others(half_temp, reqs, half_assignments, rank, HALF);

    shrink_mpi(orig, quarter_temp, quarter_assignments[rank], QUARTER);
    send_to_others(quarter_temp, reqs, rank, quarter_assignments[rank], QUARTER,
        num_tasks);
    receive_from_others(quarter_temp, reqs, quarter_assignments, rank, QUARTER);

    shrink_mpi(orig, eighth_temp, eighth_assignments[rank], EIGHTH);
    send_to_others(eighth_temp, reqs, rank, eighth_assignments[rank], EIGHTH,
        num_tasks);
    receive_from_others(eighth_temp, reqs, eighth_assignments, rank, EIGHTH);

    // Barrier: Make sure all threads finished
}

LoG::LoG(Image & src, int num_tasks_x, int rank_x, MPI_Request* reqs_x, 
    MPI_Status* stats_x) : orig(src), num_tasks(num_tasks_x), rank(rank_x), 
    reqs(reqs_x), stats(stats_x) {}
