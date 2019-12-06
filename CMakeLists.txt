cmake_minimum_required(VERSION 2.8.12.2)
project(Parallel_SIFT)

set(CFLAGS
    -std=c++11
    -m64
    #     -fopenmp
)

add_compile_options(${CFLAGS})
# NOTE: The below path is for CMU machines
set(OpenCV_DIR /usr/local/depot/opencv/share/OpenCV)
find_package(OpenCV REQUIRED)
find_package(MPI REQUIRED)

include_directories(
    include
	${OpenCV_INCLUDE_DIRS}
  ${MPI_INCLUDE_PATH}
)

set(ALL_LIBS
	${OpenCV_LIBS}
  ${MPI_LIBRARIES}
)



# find_package(OpenMP)
# if (OPENMP_FOUND)
#    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
#    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
#endif()

set(${PROJECT_NAME}_SOURCES
    src/main.cpp
    src/Image.cpp
    src/GaussianBlur.cpp
    src/Keypoint.cpp
    src/LoG.cpp
    src/general_helpers.cpp
)

set(${PROJECT_NAME}_test_gaussian_blur_SOURCES
    src/test_gaussian_blur.cpp
    src/Image.cpp
    src/GaussianBlur.cpp
    src/Keypoint.cpp
    src/LoG.cpp
    src/general_helpers.cpp
)

# Main
add_executable(${PROJECT_NAME}_main ${${PROJECT_NAME}_SOURCES})
target_link_libraries(${PROJECT_NAME}_main ${ALL_LIBS})

# Test
add_executable(${PROJECT_NAME}_test_gaussian_blur ${${PROJECT_NAME}_test_gaussian_blur_SOURCES})
target_link_libraries(${PROJECT_NAME}_test_gaussian_blur ${ALL_LIBS})

# Test OpenMP Compilation
# add_executable(test_openmp src/test_openmp.cpp)

if(MPI_COMPILE_FLAGS)
  set_target_properties(${PROJECT_NAME}_main PROPERTIES
    COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
  set_target_properties(${PROJECT_NAME}_test_gaussian_blur PROPERTIES
    COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(${PROJECT_NAME}_main PROPERTIES
    LINK_FLAGS "${MPI_LINK_FLAGS}")
  set_target_properties(${PROJECT_NAME}_test_gaussian_blur PROPERTIES
    LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()
