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

    // Init MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // two requests per task: one for recv, one for send
    // arranged as [0, num_tasks) for receive, [num_tasks, num_tasks*2) for send
    MPI_Request reqs[num_tasks * 2];
    MPI_Status stats[num_tasks * 2];

    // Each task allocates new work to be done
    std::vector<range> half_assignments;
    std::vector<range> quarter_assignments;
    std::vector<range> eighth_assignments;
    allocate_shrink_work_mpi(src1.rows, src1.cols, num_tasks,
        half_assignments, quarter_assignments,
        eighth_assignments);

    // Each task holds local array to hold results received from other tasks
    int half_temp[src1.rows/2 * src1.cols/2];
    int quarter_temp[src1.rows/4 * src1.cols/4];
    int eighth_temp[src1.rows/8 * src1.cols/8];

    // Each task performs its own work in parallel and sends results
    // non-blocking and receives non-blocking
    shrink_mpi(src1, half_temp, half_assignments[rank], HALF);
    send_shrink_to_others(half_temp, reqs, rank, half_assignments[rank], HALF, num_tasks);
    receive_shrink_from_others(half_temp, reqs, half_assignments, rank, HALF);

    shrink_mpi(src1, quarter_temp, quarter_assignments[rank], QUARTER);
    send_shrink_to_others(quarter_temp, reqs, rank, quarter_assignments[rank], QUARTER,
        num_tasks);
    receive_shrink_from_others(quarter_temp, reqs, quarter_assignments, rank, QUARTER);

    shrink_mpi(src1, eighth_temp, eighth_assignments[rank], EIGHTH);
    send_shrink_to_others(eighth_temp, reqs, rank, eighth_assignments[rank], EIGHTH,
        num_tasks);
    receive_shrink_from_others(eighth_temp, reqs, eighth_assignments, rank, EIGHTH);

    printf("Thread %d finished shrink half\n", rank);
    for (int task = 0; task < num_tasks; task++) {
        if (task == rank) continue;
        MPI_Wait(&reqs[task],&stats[task]);
    }

    Image half(src1.rows/2, src1.cols/2, half_temp);
    Image quarter(src1.rows/4, src1.cols/4, quarter_temp);
    Image eighth(src1.rows/8, src1.cols/8, eighth_temp);

    if (rank == view_index) {
        if (debug) cout << "Storing result" << endl;
        quarter.store_opencv(res_output);
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
