import cv2
import numpy as np
from matplotlib import pyplot as plt

"""
Set of tools to evaluate accuracy and correctness of various substeps of SIFT,
which are other algorithms.
"""
def compare_images(path1, path2, viz=False):
    # Load an color image, pass in 0 as 2nd arg for grayscale
    img1 = cv2.imread(path1)
    img2 = cv2.imread(path2)

    # 1) Check if 2 images are equals
    if img1.shape != img2.shape:
        print("The images have different size and channels!")
        print("Image 1: " + str(img1.shape))
        print("Image 2: " + str(img2.shape))
        return False

    diff = cv2.subtract(img1, img2)
    b, g, r = cv2.split(diff)
    is_same = (
        cv2.countNonZero(b) == 0 and 
        cv2.countNonZero(g) == 0 and 
        cv2.countNonZero(r) == 0)
    if is_same: print("Images are Same!")
    else: print("Images Different!")
    
    if viz:
        cv2.namedWindow(path1, cv2.WINDOW_NORMAL)
        cv2.imshow('Image 1', img1)
        cv2.namedWindow(path2, cv2.WINDOW_NORMAL)
        cv2.imshow('Image 2', img2)
        cv2.waitKey(0)
        cv2.destroyAllWindows()
    
    return is_same

def gaussian_blur():
    img = cv2.imread('build/pikachu.jpg')
    blur = cv2.GaussianBlur(img,(9,9),4)
    plt.subplot(121),plt.imshow(img),plt.title('Original')
    plt.xticks([]), plt.yticks([])
    plt.subplot(122),plt.imshow(blur),plt.title('Averaging')
    plt.xticks([]), plt.yticks([])
    plt.show()

gaussian_blur()