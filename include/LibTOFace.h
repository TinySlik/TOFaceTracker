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



    //=================================Plans are no longer available==================================



    /*
    //开始 (Plans are no longer available)
     */
    TO_LIB_STATUS TO_Face_Tracker_Start(
        // these 3 Cascade  path is the main detect tool of face detect. 
        const char *faceCascadeFilename,
        // 线程的休眠间隔最小单位，就是说隔多久对整体的状态做一次反应，不可设置太大 单位：us
        const unsigned long threadsWaitTimeInterval,
        // 脸部检测休眠检测判定需要的检测失败次数 
        const unsigned short faceDetectCountEnableThreshold,
        // 自适应的主检测更新策略阀值 
        const int updateStrategy,
        //更新限制时间
        const int updateStrategyTime,
        //宽
        const size_t width,
        //高
        const size_t height,
        //最高脸部检测数量上限
        const size_t count
        );

    /*
    //当前是否存在脸(Plans are no longer available)
     */
    TO_LIB_STATUS TO_Face_is_Face_Exist_Now();


    /*
    //实时处理简化接口(包含TO_Face_Tracker_Sampling 和 TO_Face_Tracker_Check 功能 ，为单帧非异步调用提供接口)(Plans are no longer available)
    */
    TO_LIB_STATUS TO_Face_Tracker_Update(
        //脸部位置框结果输出
        std::vector<TO_RECT>& face,
        //阻塞超时阀值
        const unsigned int blockWaitTime, // ms
        //测试结果显示输出，NULL时不做结果显示工作，在主检测口会增加耗时～5ms
        unsigned char* pBGRA);

#ifdef __cplusplus
}
#endif

#endif //TO_LIB_H
