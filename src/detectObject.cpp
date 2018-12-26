//////////////////////////////////////////////////////////////////////////////////////
//  用LBP or Haar Cascades检测器检测人脸或眼睛
//////////////////////////////////////////////////////////////////////////////////////

#include "detectObject.h"    

//检测人脸并将结果保存至'objects
void detectObjectsCustom(const Mat &img, CascadeClassifier &cascade, vector<Rect> &objects, int scaledWidth, int flags, Size minFeatureSize, float searchScaleFactor, int minNeighbors)
{
    // 灰度图像的转换
    Mat gray;
    if (img.channels() == 3) {
        cvtColor(img, gray, CV_BGR2GRAY);
    }
    else if (img.channels() == 4) {
        cvtColor(img, gray, CV_BGRA2GRAY);
    }
    else {
        gray = img;
    }

    // 缩放摄像机图像
    Mat inputImg;
    float scale = img.cols / (float)scaledWidth;//缩放比例
    if (img.cols > scaledWidth) {
        int scaledHeight = cvRound(img.rows / scale);
        resize(gray, inputImg, Size(scaledWidth, scaledHeight));
    }
    else {
        inputImg = gray;//图像足够小时，直接处理
    }

    Mat equalizedImg;
    equalizeHist(inputImg, equalizedImg);
    // 在缩小后的图像中检测人脸
    cascade.detectMultiScale(equalizedImg, objects, searchScaleFactor, minNeighbors, flags, minFeatureSize);
    // 检测到人脸后将缩小的人脸恢复至原来的大小
    if (img.cols > scaledWidth) {
        for (int i = 0; i < (int)objects.size(); i++ ) {
            objects[i].x = cvRound(objects[i].x * scale);
            objects[i].y = cvRound(objects[i].y * scale);
            objects[i].width = cvRound(objects[i].width * scale);
            objects[i].height = cvRound(objects[i].height * scale);
        }
    }

    // 对位于边缘的人脸的处理
    for (int i = 0; i < (int)objects.size(); i++ ) {
        if (objects[i].x < 0)
            objects[i].x = 0;
        if (objects[i].y < 0)
            objects[i].y = 0;
        if (objects[i].x + objects[i].width > img.cols)
            objects[i].x = img.cols - objects[i].width;
        if (objects[i].y + objects[i].height > img.rows)
            objects[i].y = img.rows - objects[i].height;
    }
}

// 检测图像中最大的脸，并保存至'largestObject'，用Haar cascades or LBP cascades进行人脸检测
// 为了加快程序运行速度，将图像进行缩放
void detectLargestObject(const Mat &img, CascadeClassifier &cascade, Rect &largestObject, int scaledWidth)
{
	//下面四个参数是detectMultiScale()函数的参数
    // 检测最大的人脸
    int flags = CASCADE_FIND_BIGGEST_OBJECT| CASCADE_DO_ROUGH_SEARCH;
    // 最小的人脸大小为20*20
    Size minFeatureSize = Size(20, 20);
    // 决定有多少不同大小的人脸需要检测，一般大于1
    float searchScaleFactor = 1.1f;
    // 决定人脸检测器如何确定人脸已被检测到
    // minNeighbors=2 表示许多好的和坏的人脸都被检测到；minNeighbors=6 只检测到好的人脸
    int minNeighbors = 4;

    // 检测最大的人脸
    vector<Rect> objects;

    detectObjectsCustom(img, cascade, objects, scaledWidth, flags, minFeatureSize, searchScaleFactor, minNeighbors);

    if (objects.size() > 0) {
        largestObject = (Rect)objects.at(0);//赋值
    }
    else {
        largestObject = Rect(-1,-1,-1,-1);//未检测到
    }
}

//检测图像中所有的人脸，并保存至 'objects'，用Haar cascades or LBP cascades进行人脸检测，并对图像进行缩放
void detectManyObjects(const Mat &img, CascadeClassifier &cascade, vector<Rect> &objects, int scaledWidth)
{
    // 在图像中检测许多人脸
    int flags = CASCADE_SCALE_IMAGE;
    Size minFeatureSize = Size(20, 20);
    float searchScaleFactor = 1.1f;
    int minNeighbors = 4;

    // 检测许多人脸
    detectObjectsCustom(img, cascade, objects, scaledWidth, flags, minFeatureSize, searchScaleFactor, minNeighbors);
}
