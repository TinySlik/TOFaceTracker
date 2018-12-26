//*******************************************************************/
// opencv 3.1 and c99 is requied.
//
//
//
//********************************************************************/

#ifndef TO_LIB_DEBUG_H
#define TO_LIB_DEBUG_H

#include "LibTOFace.h"

typedef enum {
    TO_SKIN_FILTER_RGB = 0,
    TO_SKIN_FILTER_ELLIPSE,
    TO_SKIN_FILTER_YCRCB_OTSU,
    TO_SKIN_FILTER_YCRCB,
    TO_SKIN_FILTER_HSV,
    TO_SKIN_FILTER_NONE
} TO_SKIN_FILTER_KIND;

#ifdef __cplusplus
extern "C" {
#endif

    /*
    //帧取样
     */
    TO_LIB_STATUS TO_Face_Tracker_Sampling(
        //取样数据指针
        const unsigned char* pBGRA);

    /*
    //实时处理接口
     */
    TO_LIB_STATUS TO_Face_Tracker_Check_With_Debug(
        //脸部位置框结果输出
        std::vector<TO_RECT>& face,
        //特征点 长度固定为68
        std::vector<TO_LANDMARK_68>& landMark,
        //测试结果显示输出，NULL时不做结果显示工作，在主检测口会增加耗时～5ms
        unsigned char* pBGRA);


    //=================================Plans are no longer available==================================

    /*
    //实时处理接口 (Plans are no longer available)
     */
    TO_LIB_STATUS TO_Face_Tracker_Check(
        //脸部位置框结果输出
        std::vector<TO_RECT>& face,
        //阻塞超时阀值
        const unsigned int blockWaitTime, // ms
        //测试结果显示输出，NULL时不做结果显示工作，在主检测口会增加耗时～5ms
        unsigned char* pBGRA);

    /*
    //肤色滤波 (Plans are no longer available)
     */
    TO_LIB_STATUS TO_Face_Skin_Filter(
        //输出结果
        cv::Mat& out,
        //输入
        cv::Mat& in,
        //滤波器种类
        TO_SKIN_FILTER_KIND filtet
        );

    /*
    //停止线程，保留类内变量资源 (Plans are no longer available)
     */
    TO_LIB_STATUS TO_Face_Tracker_Pause();

    /*
    //使用类内变量资源启动线程 (Plans are no longer available)
     */
    TO_LIB_STATUS TO_Face_Tracker_Resume();

#ifdef __cplusplus
}
#endif

#endif //TO_LIB_DEBUG_H
