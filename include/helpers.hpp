#include <vector>
#include <iostream>

// print vector of any type, as long as that type can be printed
template <class data_type>
void print_arr(std::vector<data_type> vec) {
    for (data_type val : vec) std::cout << val << ", ";
    std::cout << std::endl;
}

template <class data_type>
data_type sum(std::vector<data_type> & vec) {
    data_type total = 0;
    for (data_type val : vec) total += val;
    return total;
}