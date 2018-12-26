
#include "TOSkinFiltering.h"

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <queue>

#include "kcftracker.hpp"
#include "preprocessFace.h"
#include "TOFrameManager.h"

using namespace cv;
using namespace std;
using namespace TO;

/*基于RGB范围的皮肤检测*/
Mat TO::RGB_detect(Mat& img)
{
    /*
        R>95 AND G>40 B>20 AND MAX(R,G,B)-MIN(R,G,B)>15 AND ABS(R-G)>15 AND R>G AND R>B
            OR
        R>220 AND G>210 AND B>170 AND ABS(R-G)<=15 AND R>B AND G>B
    */
    Mat detect = img.clone();
    detect.setTo(0);
    if (img.empty() || img.channels() != 3)
    {
        return detect;
    }

    for (int i = 0; i < img.rows; i++)
    {
        for (int j = 0; j < img.cols; j++)
        {
            uchar *p_detect = detect.ptr<uchar>(i, j);
            uchar *p_img = img.ptr<uchar>(i, j);
            if ((p_img[2] > 95 && p_img[1]>40 && p_img[0] > 20 &&
                (MAX(p_img[0], MAX(p_img[1], p_img[2])) - MIN(p_img[0], MIN(p_img[1], p_img[2])) > 15) &&
                abs(p_img[2] - p_img[1]) > 15 && p_img[2] > p_img[1] && p_img[1] > p_img[0]) ||
                (p_img[2] > 200 && p_img[1] > 210 && p_img[0] > 170 && abs(p_img[2] - p_img[1]) <= 15 &&
                p_img[2] > p_img[0] &&  p_img[1] > p_img[0]))
            {
                p_detect[0] = p_img[0];
                p_detect[1] = p_img[1];
                p_detect[2] = p_img[2];
            }
        }

    }
    return detect;
}

/*基于椭圆皮肤模型的皮肤检测*/
Mat TO::ellipse_detect(const Mat& src)
{
    Mat img = src.clone();
    Mat skinCrCbHist = Mat::zeros(Size(256, 256), CV_8UC1);
    //利用opencv自带的椭圆生成函数先生成一个肤色椭圆模型
    ellipse(skinCrCbHist, Point(113, 155.6), Size(23.4, 15.2), 43.0, 0.0, 360.0, Scalar(255, 255, 255), -1);
    Mat ycrcb_image;
    Mat output_mask = Mat::zeros(img.size(), CV_8UC1);
    cvtColor(img, ycrcb_image, CV_BGR2YCrCb); //首先转换成到YCrCb空间
    for (int i = 0; i < img.cols; i++)   //利用椭圆皮肤模型进行皮肤检测
        for (int j = 0; j < img.rows; j++) 
        {
            Vec3b ycrcb = ycrcb_image.at<Vec3b>(j, i);
            if (skinCrCbHist.at<uchar>(ycrcb[1], ycrcb[2]) > 0)   //如果该落在皮肤模型椭圆区域内，该点就是皮肤像素点
                output_mask.at<uchar>(j, i) = 255;
        }

    Mat detect;
    img.copyTo(detect,output_mask);  //返回肤色图
    return detect;
}

/*YCrCb颜色空间Cr分量+Otsu法*/
Mat TO::YCrCb_Otsu_detect(const Mat& src)
{
    Mat ycrcb_image;
    cvtColor(src, ycrcb_image, CV_BGR2YCrCb); //首先转换成到YCrCb空间
    Mat detect;
    vector<Mat> channels;
    split(ycrcb_image, channels);
    Mat output_mask = channels[1];
    threshold(output_mask, output_mask, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
    src.copyTo(detect, output_mask);
    return detect;

}

/*YCrCb颜色空间Cr,Cb范围筛选法*/
Mat TO::YCrCb_detect(const Mat & src)
{
    Mat ycrcb_image;
    int Cr = 1;
    int Cb = 2;
    cvtColor(src, ycrcb_image, CV_BGR2YCrCb); //首先转换成到YCrCb空间
    Mat output_mask = Mat::zeros(src.size(), CV_8UC1);
    for (int i = 0; i < src.rows; i++)
    {
        for (int j = 0; j < src.cols; j++)
        {
            uchar *p_mask = output_mask.ptr<uchar>(i, j);
            uchar *p_src = ycrcb_image.ptr<uchar>(i, j);
            if (p_src[Cr] >= 133 && p_src[Cr] <= 173 && p_src[Cb] >= 77 && p_src[Cb] <= 127)
            {
                p_mask[0] = 255;
            }
        }
    }
    Mat detect;
    src.copyTo(detect, output_mask);;
    return detect;

}

/*HSV颜色空间H范围筛选法*/
Mat TO::HSV_detector(const Mat& src)
{
    Mat hsv_image;
    int h = 0;
    int s = 1;
    int v = 2;
    cvtColor(src, hsv_image, CV_BGR2HSV); //首先转换成到YCrCb空间
    Mat output_mask = Mat::zeros(src.size(), CV_8UC1);
    for (int i = 0; i < src.rows; i++)
    {
        for (int j = 0; j < src.cols; j++)
        {
            uchar *p_mask = output_mask.ptr<uchar>(i, j);
            uchar *p_src = hsv_image.ptr<uchar>(i, j);
            if (p_src[h] >= 0 && p_src[h] <= 20 && p_src[s] >=48 && p_src[v] >=50)
            {
                p_mask[0] = 255;
            }
        }
    }
    Mat detect;
    src.copyTo(detect, output_mask);;
    return detect;
}

TOSkinFilter::TOSkinFilter()
{
    width = 0;
    height = 0;
}

cv::Mat TOSkinFilter::generateMask (const cv::Mat& src)
{
    width = src.cols;
    height = src.rows;
#if _DEBUG_TO_SKIN_FILTER
    imshow("original image",src); 
#endif
    return src;
}

