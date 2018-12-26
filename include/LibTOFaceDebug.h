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
#ifdef __cplusplus
}
#endif

#endif //TO_LIB_DEBUG_H
