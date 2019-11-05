cmake_minimum_required(VERSION 3.1)
project(Parallel_SIFT)

set(CFLAGS
    -std=c++11
    -m64
)

add_compile_options(${CFLAGS})

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
)

set(${PROJECT_NAME}_test_gaussian_blur_SOURCES
    src/test_gaussian_blur.cpp
    src/Image.cpp
    src/GaussianBlur.cpp
)

# Main
add_executable(${PROJECT_NAME}_main ${${PROJECT_NAME}_SOURCES})
target_link_libraries(${PROJECT_NAME}_main ${ALL_LIBS})

# Test
add_executable(${PROJECT_NAME}_test_gaussian_blur ${${PROJECT_NAME}_test_gaussian_blur_SOURCES})
target_link_libraries(${PROJECT_NAME}_test_gaussian_blur ${ALL_LIBS})