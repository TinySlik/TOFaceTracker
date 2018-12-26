//////////////////////////////////////////////////////////////////////////////////////
// 用LBP or Haar Cascades检测器检测人脸或眼睛
//////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <stdio.h>
#include <iostream>
#include <vector>

#include "opencv2/opencv.hpp"


using namespace cv;
using namespace std;

// 检测图像中最大的脸，并保存至'largestObject'，用Haar cascades or LBP cascades进行人脸检测
// 为了加快程序运行速度，将图像进行缩放
void detectLargestObject(const Mat &img, CascadeClassifier &cascade, Rect &largestObject, int scaledWidth = 320);

//检测图像中所有的人脸，并保存至 'objects'，用Haar cascades or LBP cascades进行人脸检测，并对图像进行缩放
void detectManyObjects(const Mat &img, CascadeClassifier &cascade, vector<Rect> &objects, int scaledWidth = 320);
