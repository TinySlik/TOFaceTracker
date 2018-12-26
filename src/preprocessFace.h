//////////////////////////////////////////////////////////////////////////////////////
// 对检测到的人脸进行预处理
//////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <stdio.h>
#include <iostream>
#include <vector>

#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

// 检测人眼
void detectBothEyes(const Mat &face, CascadeClassifier &eyeCascade1, CascadeClassifier &eyeCascade2, Point &leftEye, Point &rightEye, Rect *searchedLeftEye = NULL, Rect *searchedRightEye = NULL);

// 人脸预处理：左侧和右侧人脸直方图均衡化
void equalizeLeftAndRightHalves(Mat &faceImg);

// 获得预处理的人脸
Mat getPreprocessedFace(Mat &srcImg, int desiredFaceWidth, CascadeClassifier &faceCascade, CascadeClassifier &eyeCascade1, CascadeClassifier &eyeCascade2, bool doLeftAndRightSeparately, Rect *storeFaceRect = NULL, Point *storeLeftEye = NULL, Point *storeRightEye = NULL, Rect *searchedLeftEye = NULL, Rect *searchedRightEye = NULL);

bool getPreprocessedFaceOnly(Mat &srcImg, CascadeClassifier &faceCascade, Rect& storeFaceRect);

bool getPreprocessedFaceMutil(Mat &srcImg, CascadeClassifier &faceCascade, std::vector<Rect>& rectS);