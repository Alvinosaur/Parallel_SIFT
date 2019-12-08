#include <vector>
#include <iostream>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include "mpi.h"

#include "GaussianBlur.h"
#include "Image.h"
#include "general_helpers.h"
#include "LoG.h"
#include "CycleTimer.h"

using namespace std;
using namespace cv;

bool debug = false;
int view_index = 0;
float grad_threshold = 0;

#define MAX_THREADS 8  // for ghc machines

int main(int argc, char* argv[]) {
    float variance = 1;
    if (argc <= 1) {
        cout << "Need to pass in image filepath!" << endl;
        exit(-1);
    }
    std::string img1_path, img2_path;
    if (!get_args(argc, argv, img1_path, img2_path, &variance, &debug,
        &view_index, &grad_threshold)) {
        std::cout << "Failed to pass in valid image path with -p1 and -p2";
        std::cout << std::endl;
        exit(-1);
    };

    cv::Mat res_output;
    cv::Mat src1_mat = cv::imread(img1_path.c_str(),
        CV_LOAD_IMAGE_GRAYSCALE);
    Image src1(src1_mat);

    int num_tasks, tag, rank;

    // Precompute weight distributions used for blur convolution
    Gaussian_Blur gb;
    std::vector<std::vector<float>> conv_distribs;
    conv_distribs.reserve(standard_variances.size());
    for (float var : standard_variances) {
        std::vector<float> new_distrib;
        int depth = gb.variance_to_depth(var);
        gb.generate_binomial_distrib(depth, new_distrib);
        conv_distribs.push_back(new_distrib);
    }

    // Init MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // two requests per task: one for recv, one for send
    // arranged as [0, num_tasks) for receive, [num_tasks, num_tasks*2) for send
    MPI_Request reqs[num_tasks * 2];
    MPI_Status stats[num_tasks * 2];

    // Each task allocates new work to be done
    std::vector<range> row_assignments, row_pix_assignments;
    allocate_conv_rows_mpi(src1.rows, src1.cols, num_tasks,
        row_assignments, row_pix_assignments);

    // Each task holds local array to hold results received from other tasks
    int conv_row_results[src1.rows * src1.cols];
    int conv_col_results[src1.rows * src1.cols];
    
    // Each task performs its own work in parallel and sends results
    // non-blocking and receives non-blocking
    std::vector<Image> conv_results;
    for (int var_i = 0; var_i < standard_variances.size(); var_i++) {

        // Perform row-convolutions first, send results to other processes
        gb.convolve_rows_mpi(src1, conv_row_results, 
            row_assignments[rank], conv_distribs[var_i]);
        send_to_others(conv_row_results, reqs, rank, row_pix_assignments[rank], 
            var_i, num_tasks);
        receive_from_others(conv_row_results, reqs, row_pix_assignments, 
            rank, var_i);

        // Barrier need to have received all row conv before starting col conv
        for (int task = 0; task < num_tasks; task++) {
            if (task == rank) continue;
            MPI_Wait(&reqs[task], &stats[task]);
        }

        // Now perform column-convolutions using received row-convolutions
        gb.convolve_cols_mpi(conv_row_results, conv_col_results, 
            row_assignments[rank], conv_distribs[var_i], src1.cols);
        send_to_others(conv_col_results, reqs, rank, row_pix_assignments[rank], 
            var_i, num_tasks);
        receive_from_others(conv_col_results, reqs, row_pix_assignments, rank, 
            var_i);

        // Barrier: must receive and store all data before reusing arrays
        for (int task = 0; task < num_tasks; task++) {
            if (task == rank) continue;
            MPI_Wait(&reqs[task], &stats[task]);
        }
        printf("Thread %d finished convolution\n", rank);

        // Store result in image before reusing temporary result arrays
        Image new_conv(src1.rows, src1.cols, conv_col_results);
        conv_results.push_back(new_conv);
    }

    if (rank == view_index) {
        if (debug) cout << "Storing result" << endl;
        conv_results[view_index].store_opencv(res_output);
        imwrite( "after_blur_result.jpg", res_output);
        imshow( "Blurred pikachu!", res_output );
        waitKey(0);
    }

    for (int task = 0; task < num_tasks; task++) {
        if (task == rank) continue;
        MPI_Wait(&reqs[task],&stats[task]);
    }
    MPI_Finalize();
    return 0;
}
