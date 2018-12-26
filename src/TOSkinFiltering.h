#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <string>

#define  _DEBUG_TO_SKIN_FILTER false

namespace TO {
    
    /*基于RGB范围的皮肤检测*/
    cv::Mat RGB_detect(cv::Mat& img);

    /*基于椭圆皮肤模型的皮肤检测*/
    cv::Mat ellipse_detect(const cv::Mat& src);

    /*YCrCb颜色空间Cr分量+Otsu法*/
    cv::Mat YCrCb_Otsu_detect(const cv::Mat& src);

    /*YCrCb颜色空间Cr,Cb范围筛选法*/
    cv::Mat YCrCb_detect(const cv::Mat & src);

    /*HSV颜色空间H范围筛选法*/
    cv::Mat HSV_detector(const cv::Mat& src);

    class TOSkinFilter: public cv::BaseCascadeClassifier::MaskGenerator
    {
    public:
        TOSkinFilter();
        virtual cv::Mat generateMask (const cv::Mat& src);
    private:
        int width;
        int height;
    };
}


