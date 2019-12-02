cmake_minimum_required(VERSION 2.8.12.2)
project(Parallel_SIFT)

set(CFLAGS
    -std=c++11
    -m64
    // -fopenmp
)

add_compile_options(${CFLAGS})
# NOTE: The below path is for CMU machines
set(OpenCV_DIR /usr/local/depot/opencv/share/OpenCV)
find_package(OpenCV REQUIRED)


include_directories(
    include
	${OpenCV_INCLUDE_DIRS}
)

set(ALL_LIBS
	${OpenCV_LIBS}
)

set(${PROJECT_NAME}_SOURCES
    src/main.cpp
    src/Image.cpp
    src/GaussianBlur.cpp
    src/LoG.cpp
    src/Keypoint.cpp
    src/general_helpers.cpp

)

set(${PROJECT_NAME}_test_gaussian_blur_SOURCES
    src/test_gaussian_blur.cpp
    src/Image.cpp
    src/GaussianBlur.cpp
    src/LoG.cpp
    src/Keypoint.cpp
    src/general_helpers.cpp
)

find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
    message("OPENMP FOUND")

    message([main] " OpenMP_C_FLAGS=${OpenMP_C_FLAGS}") # -fopenmp
    message([main] " OpenMP_CXX_FLAGS}=${OpenMP_CXX_FLAGS}") # -fopenmp
    message([main] " OpenMP_EXE_LINKER_FLAGS=${OpenMP_EXE_LINKER_FLAGS}") # ***

    # no use for xxx_INCLUDE_DIRS and xxx_libraries for OpenMP
    message([main] " OpenMP_INCLUDE_DIRS=${OpenMP_INCLUDE_DIRS}") # ***
    message([main] " OpenMP_LIBRARIES=${OpenMP_LIBRARIES}") # ***

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()


# Main
add_executable(${PROJECT_NAME}_main ${${PROJECT_NAME}_SOURCES})
target_link_libraries(${PROJECT_NAME}_main PRIVATE "${OpenMP_CXX_FLAGS}" ${ALL_LIBS})

# Test
add_executable(${PROJECT_NAME}_test_gaussian_blur ${${PROJECT_NAME}_test_gaussian_blur_SOURCES})
target_link_libraries(${PROJECT_NAME}_test_gaussian_blur ${ALL_LIBS})
