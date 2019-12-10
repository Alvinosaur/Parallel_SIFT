## [Analysis of Data Parallelism and Task Parallelism on the SIFT Algorithm](https://alvinosaur.github.io/Parallel_SIFT/)


### Summary:
We are implementing a parallel version of the SIFT algorithm to match similar localized features between two images. We will also analyze the different impacts of task-based and data-based parallelism on the SIFt algorithm.

### [Final Report PDF: Analysis of Results](Final_Report.pdf)

### How to run Sequential:
1. Clone the repo
2. Install [OpenCV](https://docs.opencv.org/master/df/d65/tutorial_table_of_content_introduction.html)
3. ```cd build```
4. ```cmake ..```
5. ```make```
6. Sequential: ```./Parallel_SIFT_main -a photo2.jpg -b photo1.jpg -d -i 0 -g 7```

### How to run Parallel OpenMPI:
1. Repeat steps 1 and 2 for sequential above
2. ```git checkout openMPI```
3. Same build steps 3 through 5 as above
4. ```mpiexec -n 8 ./Parallel_SIFT_main -a pikachu.jpg -b pikachu.jpg -g 12```

### How to run Parallel OpenMP:
1. Same steps as Sequential, but in ```openMP``` branch

### Background:
Matching features across two images is a common task in many computer vision applications. For instance, everything from robots to the newest iPhone uses disparity estimation to generate depth images from 2D images, similar to how humans estimate depth through two eyes. This disparity estimation attempts to align two slight shifted images through feature matching. Motion capture and optical flow similarly attempt to match two images and identify the overall displacement between these matching parts of image.

Our project will involve parallelizing an algorithm for feature-matching known as Scale Invariant Feature Transform (SIFT), which can account for rotation, illumination, and perspective change between two images. At a high level, SIFT can be broken down into a few steps:

Find key points in both images by finding regions with high change from surrounding pixels.
Find orientation of key points and apply inverse rotation. This explains why SIFT is invariant to rotations.
Distinguish key points in the same image and match key points from the two images.

  Many of the intermediary calculations for the above steps are identical across an image’s pixels. For instance, the first step involves calculating the Difference of Gaussian on scale space images, which are blurred and downsized versions of an original image. Blurring involves calculating the average of a region of pixels and setting the center pixel to this value. This computation is the exact same for every pixel in an image, so different pixels and sections of the image can be handled by different processes. With this, much of the SIFT algorithm is data-parallelizable, meaning the same function can be applied on different, independent data values. We will also explore task-based parallelism by allowing different processes work on different tasks simultaneously if they are order-invariant.


### Challenges:
One of the major challenges that we have to face is creating all the algorithms must be implemented from scratch in both sequential and parallel. The sift algorithm itself has requires many calculations and modules in itself such as:
### Scale Space:
- Used for detecting keypoints
- Gaussian Blur(similar to Ocean Simulator, playing with cache efficiency)
- shrinking image to half prev size
- Laplacian of Gaussian

### Keypoint Localization
- Finding min/max in a 2D array of 2nd order derivatives
- Orientation Assignment
- Assignment of orientation to keypoints in case of invariance in image rotation
- Keypoint Descriptor
- Some specified dimension of points around the keypoint is taken
- Keypoint Matching
- Matching of keypoints between images

While the SIFT algorithm is not innate sequential in nature, there needs to be a lot of synchronization and communication between workers in order to create an accurate mapping of similarities. If we were to do data parallelism, then we must account for the fact that if we split the data up into discrete chunks of pixels, we could potentially lose a similarity of localized points that lie on the border of those discrete chunks of pixels, hence requiring some thought into the synchronization and communication protocols.


### Resources
- aishack.in, aishack.in/tutorials/sift-scale-invariant-feature-transform-introduction/.
- Mordvintsev, Alexander, and Abid K. Revision. Introduction to SIFT (Scale-Invariant Feature Transform),
opencv-python-tutroals.readthedocs.io/en/latest/py_tutorials/py_feature2d/py_sift_intro/py_sift_intro.html.
- Kumar, R. , Muknahallipatna, S. and McInroy, J. (2016) An Approach to Parallelization of SIFT Algorithm on GPUs for Real-Time Applications. Journal of Computer and Communications, 4, 18-50. doi: 10.4236/jcc.2016.417002.
- "Computer Vision: Feature detection and m." UW CSE vision faculty, courses.cs.washington.edu/courses/cse455/09wi/Lects/lect6.pdf?fbclid=IwAR3F3Vs3sK0a2BtInWO8M5_NFSknCf7q857_rewX8qFlzxqN-uzb4BU0gyo.



### Goals and Deliverables:

### Plan to achieve:
Complete sequential and parallel implementations of the SIFT algorithm
Implement task parallelism and data parallelism of the SIFt algorithm
Achieve 3x speedup for data parallel implementation on the GHC machines
Interactive demo that has a participant take two pictures in different poses that  will allow the sequential and parallel implementations of the SIFT algorithm to match similar features as seen in this picture:

Analysis of different impacts that task parallelism and data parallelism has on the SIFT algorithm. This will include graphs comparing the speedup of the two different implementations.

### Hope to achieve:
Achieve 5x speedup for both data parallel and task parallel implementations
Analysis the impact that different image resolution can have on SIFT algorithm



### Platform of Choice:
- Language: C++
- APIs: OpenMP, OpenMPI, Cuda
- Machines: Latedays Cluster (Xeon Phi Processors, NVIDIA K40 GPU)


### Original Schedule (Revised/New 11/18):
- November 22nd: Complete sequential implementation of task parallelism
  - Hojun and Alvin will work together exclusively on finishing sequential after the Wednesday midterm
  - We have most of the sequential done, but just need to implement the last step
  - We will also begin data parallelization of our algorithms, which should be straightforward with double for-loops
- November 24rd: Implement both data parallelism and task parallelism (basic versions)
  - Hojun will focus on task parallelism. Implement at least 3x Speedup
  - Alvin will focus on data parallelism. Implement at least 3x Speedup
- November 28th:
  - If we finish extra, we will play with the MPI version, which will be straightforward since image data can be represented as a 2D array
- December 3rd: Achieve most speedup possible (advanced parallelism) and begin final report
  - Further develop our intial approaches and attempt hybrid of MP and MPI as well as data and task parallelism
  - Draft Poster and necessary graphs, images for presentation
- December 5th: Order all the necessary presentation materials
- December 7th: Complete script for demo
  - Clean up scripts and source code
- December 9th: Project deadline
  - Practice presenting material


### Original Schedule:
- November 6th: Complete sequential version of the SIFT algorithm
- November 12th: Complete implementation of data parallelism
- November 18th: Complete implementation of task parallelism
- November 24rd: Implement both data parallelism and task parallelism
- December 3rd: Achieve most speedup possible and begin final report
- December 7th: Complete script for demo
- December 9th: Project deadline
