#include "general_helpers.h"
#include "mpi.h"

#define MAX_TASKS 8

int almost_equal(float v1, float v2, float abs_error) {
    return (v1 - v2) <= abs_error;
}

// handle edge cases where kernel exceeds image bounds
// by having out-of-bounds pixel be pixel on opposite side of kernel
int reflect(int M, int x) {
    if(x < 0) return -x - 1;
    if(x >= M) return 2*M - x - 1;
    return x;
}

void allocate_work_mpi(int rows, int cols, int num_tasks,
        std::vector<range> & half_assignments,
        std::vector<range> & quarter_assignments,
        std::vector<range> & eighth_assignments) {
    int total_pix = rows * cols;
    int scale;
    int num_pix[3] = {
        total_pix / (HALF * HALF),
        total_pix / (QUARTER * QUARTER),
        total_pix / (EIGHTH * EIGHTH)
    };

    int pix_per_task[3] = {
        num_pix[0] / num_tasks,
        num_pix[1] / num_tasks,
        num_pix[2] / num_tasks
    };

    // arrays of start and end indexes, one for each image scale
    int start[3] = {0};
    int end[3] = {0};

    for (int task = 0; task < num_tasks; task++) {
        for (int scale_i = 0; scale_i < 3; scale_i++) {
            start[scale_i] = pix_per_task[scale_i] * task;
            end[scale_i] = pix_per_task[scale_i] * (task + 1);
            range new_range;
            new_range.first = start[0];
            new_range.second = end[0];
            if (scale_i == 0) half_assignments.push_back(new_range);
            else if (scale_i == 1) quarter_assignments.push_back(new_range);
            else eighth_assignments.push_back(new_range);
        }
    }
}

void receive_from_others(int* result, MPI_Request* reqs,
    std::vector<range> & assignments, int self_rank, int scale) {
    // Loop through all other tasks to receive results from them
    range new_range;
    int size, num_tasks = assignments.size();
    for (int task = 0; task < num_tasks; task++) {
        if (task == self_rank) continue;  // don't need to request from self
        new_range = assignments[task];
        size = new_range.second - new_range.first;
        // MPI_Irecv(buffer, count, type, source, tag, comm, request);
        MPI_Irecv(&result[new_range.first], size, MPI_INT, task, scale, 
            MPI_COMM_WORLD, &reqs[task]);
    }
}

void send_to_others(const Image & src, MPI_Request* reqs, int self_rank, 
        range self_range, int scale, int num_tasks) {
    int start = self_range.first;
    int size = self_range.second - start;
    for (int task = 0; task < assignments.size(); task++) {
        if (task == self_rank) continue;  // don't need to send to self
        // MPI_Isend(buffer, count, type, dest, tag, comm, request);
        MPI_Isend(&src.data.data()[start], size, MPI_INT, task, scale,
            MPI_COMM_WORLD, &reqs[task + num_tasks]);
    }
}

void shrink_mpi(const Image & src, Image & dst, const range & start_end,
        int scale) {
    int total_val;
    int r_offset, c_offset;
    int i, j, k;
    int max_rows = src.rows-(scale-1);
    int max_cols = src.cols-(scale-1);
    int start = start_end.first, end = start_end.second;
    int num_pix = (max_rows * max_cols) / scale;
    int src_cols = src.cols / scale;
    int dst_i, dst_row, dst_col;

    for (dst_i = start; dst_i < end; dst_i++) {
        k = dst_i * scale;
        j = k % max_cols;
        i = k / max_cols;
        total_val = 0;
        for (r_offset = 0; r_offset < scale; r_offset++) {
            for (c_offset = 0; c_offset < scale; c_offset++) {
                total_val += src.get(i+r_offset, j+c_offset);
            }
        }

        dst_row = dst_i / src_cols;
        dst_col = dst_i % src_cols; 
        dst.set(dst_row, dst_col, (int)(
            (float)total_val / (float)(scale * scale)));
    }
}

void print_usage() {
    /*
     * Prints out usage info for passing in commandline args for matrix
     * generation.
     */
    printf("Usage:\n");
    printf("   -h   print help message describing usage\n");
    printf("   -d   print additional diagnostic information\n");
    printf("   -a   path for input image 1\n");
    printf("   -b   path for input image 2\n");
    printf("   -i   view_index for which gaussian difference to view\n");
    printf("   -v   desired variance level\n");
    printf("   -g   gradient threshold for keypoint filtering\n");
}


bool get_args(int argc, char** argv, 
        std::string & img1_path, std::string & img2_path, float* variance, 
        bool* debug, int* view_index, float* gradient_threshold) {
    /*
     * Reads in input args from commandline.
     */
    extern char* optarg;
    int option;
    bool set_img1_path = false, set_img2_path = false;
    while ((option = getopt(argc, argv, "hda:b:i:v:g:")) != EOF) {
        switch (option) {
            case 'h':  // help
                print_usage();
                break;
            case 'd':
                *debug = true;
                break;
            case 'a':
                img1_path = optarg;
                set_img1_path = true;
                if (*debug) printf("Input Image1: %s\n", img1_path.c_str());
                break;
            case 'b':
                img2_path = optarg;
                set_img2_path = true;
                if (*debug) printf("Input Image2: %s\n", img2_path.c_str());
                break;
            case 'i':
                *view_index = atoi(optarg);
                printf("Index: %d\n", *view_index);
                break;
            case 'v':
                *variance = atof(optarg);
                if (*debug) printf("Variance: %f\n", *variance);
                break;
            case 'g':
                *gradient_threshold = atof(optarg);
                if (*gradient_threshold) printf(
                    "Gradient Threshold: %f\n", *gradient_threshold);
                break;
            default:
                print_usage();
                break;
        }
    }
    return set_img1_path && set_img2_path;
}