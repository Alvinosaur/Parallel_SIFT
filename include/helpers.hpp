#include <vector>
#include <iostream>

// print vector of any type, as long as that type can be printed
template <class data_type>
void print_arr(std::vector<data_type> vec) {
    for (data_type val : vec) std::cout << val << ", ";
    std::cout << std::endl;
}

int sum(std::vector<int> & vec) {
    int total = 0;
    for (int val : vec) total += val;
    return total;
}