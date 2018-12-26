//*******************************************************************/
// opencv 3.1 and c++ 11 is requied.
//
//
//
//********************************************************************/

#ifndef TO_LIB_H
#define TO_LIB_H

typedef enum {
    STATUS_TO_LIB_NORMAL = 0,
    STATUS_TO_LIB_ERROR,
    STATUS_TO_LIB_YES,
    STATUS_TO_LIB_NO
} TO_LIB_STATUS;

typedef struct {
	int x;
	int y;
	int width;
	int height;
} TO_RECT;

typedef struct {
    TO_RECT rect;
    
} TO_FACE_INFO;

typedef struct {
	int x;
	int y;
} TO_POINT;

typedef struct {
    TO_POINT shape[68];
} TO_LANDMARK_68;

#ifdef __cplusplus
extern "C" {
#endif
    

    /*
    //实时处理简化接口(包含TO_Face_Tracker_Sampling 和 TO_Face_Tracker_Check 功能 ，为单帧非异步调用提供接口)
    */
    TO_LIB_STATUS TO_Face_Tracker_Update_Simplify(
        //脸部位置框结果输出
        std::vector<TO_RECT>& face,
        //特征点 长度固定为68
        std::vector<TO_LANDMARK_68>& landMark,
        //取样
        unsigned char* pBGRA);

    /*
    //开始
     */
    TO_LIB_STATUS TO_Face_Tracker_Start_With_Config(std::string cfgFileName);

    /*
    //结束
     */
    TO_LIB_STATUS TO_Face_Tracker_Stop();
#ifdef __cplusplus
}
#endif

#endif //TO_LIB_H
