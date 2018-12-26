//
//  TOInterFace.cpp
//  TOInterFace
//
//  Created by Tiny Oh 2018.1.14
//  Copyright (c) 2018 White Sky. All rights reserved.
//

#include "TOInterFace.h"

TO::TOFrameManager* g_frameManager;
TO::TOFaceDetector* g_facedetector;
TO::TOTrackerTwistedPool* g_trackerTeistedPool;

unsigned int g_frameWidth;
unsigned int g_frameHeight;

bool isInstantiation= false;

#define CONFIGURU_IMPLEMENTATION 1
#include "configuru.hpp"

#include <fstream>
#define DEFAULT_CONFIG_FILENAME "TOLibConfig.cfg"


#define VERSION 0.10

/*
//开始
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
    )
{
    if(isInstantiation)
    {
        std::cerr <<  "already isInstantiation" << endl;
        return STATUS_TO_LIB_ERROR;
    }
    std::string cfgFileName = DEFAULT_CONFIG_FILENAME;

    return TO_Face_Tracker_Start_With_Config(cfgFileName);
}


/*
//开始
exp: TO_LIB_STATUS res = TO_Face_Tracker_Start_With_Config("TOLibConfig.cfg");
 */
TO_LIB_STATUS TO_Face_Tracker_Start_With_Config(
    // 配置文件路径
    std::string cfgFileName
    )
{
    if(isInstantiation)
    {
        std::cerr <<  "already isInstantiation" << endl;
        return STATUS_TO_LIB_ERROR;
    }

    fstream _file;
    configuru::Config cfg;
    _file.open(cfgFileName,ios::in);
    if(!_file)
    {
        cout<<"no config file named " << cfgFileName <<" found!" << endl;
        return STATUS_TO_LIB_ERROR;
    }
    else
    {
        cfg = configuru::parse_file(cfgFileName, configuru::CFG);
    }

    if (cfg.has_key("detector") && 
        cfg.has_key("tracker") && 
        cfg.has_key("frame") &&
        cfg.has_key("frame_width") &&
        cfg.has_key("frame_height")
        )
    {
        cout << "TOFace lib config:" << endl << cfg << endl;
        cout << "Program Version: " << VERSION << endl;
    }else
    {
        cout << "ERROR: Necessary config deficiency!" << endl;
        return STATUS_TO_LIB_ERROR;
    }

    if (cfg.has_key("debug")) {
        for (auto& p : cfg["debug"].as_object()) {
            // std::cout << "Key: " << p.key() << std::endl;
            // std::cout << "Value: " << p.value() << std::endl;
        }
    }

    g_frameWidth = (unsigned int)cfg["frame_width"];
    g_frameHeight = (unsigned int)cfg["frame_height"];
    // step 1
    g_frameManager = new TO::TOFrameManager();
    g_frameManager->setFPSSuperiorLimit((unsigned int)cfg["frame"]["fps_superior_limit"]);
    g_frameManager->setIntegerMultipleTimeInterval((bool)cfg["frame"]["integer_fps_multiple"]);
    g_frameManager->setPoolDepth((unsigned int)cfg["frame"]["pool_depth"]);



    //step 2
    //nessary
    g_facedetector = new TO::TOFaceDetector(*g_frameManager,(std::string)cfg["detector"]["face_cascade_filename"],g_frameWidth,g_frameHeight);
    g_facedetector->setFPSSuperiorLimit((unsigned int)cfg["detector"]["fps_superior_limit"]);
    g_facedetector->setIntegerMultipleTimeInterval((bool)cfg["detector"]["integer_fps_multiple"]);
    g_facedetector->setFilterEnable((bool)cfg["detector"]["skin_filter_enable"]);
    g_facedetector->setFaceDetectCountEnableThreshold((unsigned int)cfg["detector"]["face_detect_init_threshold"]);
    //unnessary
    if(cfg["detector"].has_key("save_matter_threshold"))
    {
        g_facedetector->setSaveMatterThr((unsigned int)cfg["detector"]["save_matter_threshold"]);
    }
    if(cfg["detector"].has_key("chains_configdence_threshold"))
    {
        g_facedetector->setChainsConfidenceThr((double)cfg["detector"]["chains_configdence_threshold"]);
    }
    if(cfg["detector"].has_key("fix_configdence_split_threshold"))
    {
        g_facedetector->setFixConfidenceSplitThr((double)cfg["detector"]["fix_configdence_split_threshold"]);
    }
    if(cfg["detector"].has_key("fix_configdence_enable_threshold"))
    {
        g_facedetector->setFixConfidenceEnableThr((double)cfg["detector"]["fix_configdence_enable_threshold"]);
    }
    if(cfg["detector"].has_key("fix_configdence_skip_threshold"))
    {
        g_facedetector->setFixConfidenceSkipThr((double)cfg["detector"]["fix_configdence_skip_threshold"]);
    }
    if(cfg["detector"].has_key("time_frames_similar_threshold"))
    {
        g_facedetector->setDetectActiveBetweenSimilarThr((unsigned int)cfg["detector"]["time_frames_similar_threshold"]);
    }
    




    //step 3
    //nessary
    g_trackerTeistedPool = new TO::TOTrackerTwistedPool(*g_frameManager,*g_facedetector,g_frameWidth,g_frameHeight);
    g_trackerTeistedPool->setFPSSuperiorLimit((unsigned int)cfg["tracker"]["fps_superior_limit"]);
    g_trackerTeistedPool->setIntegerMultipleTimeInterval((bool)cfg["tracker"]["integer_fps_multiple"]);
    g_trackerTeistedPool->setMaxFaceCount((unsigned int)cfg["tracker"]["max_face_superior_limit"]);
    g_trackerTeistedPool->setSyncEnable((bool)cfg["tracker"]["sync_enable"]);
    bool landMarkEnable = (bool)cfg["tracker"]["dlib_landmark_68_enable"];
    g_trackerTeistedPool->setLandMarkEnable(landMarkEnable);
    if(landMarkEnable)
    {
        g_trackerTeistedPool->setShapeLandMarkFile((std::string)cfg["tracker"]["shape_landmark_filename"]);
    }

    //unnessary
    if(cfg["tracker"].has_key("save_matter_threshold"))
    {
        g_trackerTeistedPool->setErroCountRiskToleranceThr((unsigned int)cfg["tracker"]["save_matter_threshold"]);
    }
    if(cfg["tracker"].has_key("keep_count_threshold"))
    {
        g_trackerTeistedPool->setTrackerKeepCountThr((unsigned int)cfg["tracker"]["keep_count_threshold"]);
    }
    if(cfg["tracker"].has_key("dormant_count_threshold"))
    {
        g_trackerTeistedPool->setTrackerDormantCountEnableThr((unsigned int)cfg["tracker"]["dormant_count_threshold"]);
    }
    if(cfg["tracker"].has_key("out_of_view_threshold"))
    {
        g_trackerTeistedPool->setOutOfViewThr((double)cfg["tracker"]["out_of_view_threshold"]);
    }
    if(cfg["tracker"].has_key("kcf_hog"))
    {
        g_trackerTeistedPool->setKCFTRackerConfigHOGEnable((bool)cfg["tracker"]["kcf_hog"]);
    }
    if(cfg["tracker"].has_key("kcf_fixedwindow"))
    {
        g_trackerTeistedPool->setKCFTRackerConfigFIXEDWINDOWEnable((bool)cfg["tracker"]["kcf_fixedwindow"]);
    }
    if(cfg["tracker"].has_key("kcf_multiscale"))
    {
        g_trackerTeistedPool->setKCFTRackerConfigMULTISCALEEnable((bool)cfg["tracker"]["kcf_multiscale"]);
    }
    if(cfg["tracker"].has_key("kcf_silent"))
    {
        g_trackerTeistedPool->setKCFTRackerConfigSILENTEnable((bool)cfg["tracker"]["kcf_silent"]);
    }
    if(cfg["tracker"].has_key("kcf_lab"))
    {
        g_trackerTeistedPool->setKCFTRackerConfigLABEnable((bool)cfg["tracker"]["kcf_lab"]);
    }


    
    TO::TOThreadBase::startAllThread();

    std::cout  <<  "[TO_Face_Tracker_Start]start success" <<std::endl;

    isInstantiation  = true;
    return STATUS_TO_LIB_NORMAL;
}

/*
//结束
 */
TO_LIB_STATUS TO_Face_Tracker_Stop()
{
    if(!isInstantiation)
    {
        std::cerr <<  "not instantiation" << endl;
        return STATUS_TO_LIB_ERROR;
    }
    isInstantiation =false;

    TO::TOThreadBase::stopAllThread();

    delete g_trackerTeistedPool;
    g_trackerTeistedPool = NULL;

    delete  g_facedetector;
    g_facedetector = NULL;

    delete  g_frameManager;
    g_frameManager = NULL;
    
    g_frameWidth=0;
    g_frameHeight=0;
    
    // TO::EventManager::DeleteInstance();
    std::cout  <<  "[TO_Face_Tracker_Stop]stop success" <<std::endl;
    return STATUS_TO_LIB_NORMAL;
}

/*
//帧取样
 */
TO_LIB_STATUS TO_Face_Tracker_Sampling(
    //取样数据指针
    const unsigned char* pBGRA)
{
    if(isInstantiation)
    {
        g_frameManager->sampling(pBGRA,g_frameWidth,g_frameHeight,false);
        return STATUS_TO_LIB_NORMAL;
    }
    else{
        std::cerr <<  "not instantiation"  << endl;
        return STATUS_TO_LIB_ERROR;
    }
}







/*
//实时处理简化接口
 */

TO_LIB_STATUS TO_Face_Tracker_Update(
    //脸部位置框结果输出
    std::vector<TO_RECT>& face,
    //阻塞超时阀值
    const unsigned int blockWaitTime, // ms
    //测试结果显示输出，NULL时不做结果显示工作，在主检测口会增加耗时～5ms
    unsigned char* pBGRA)
{
    std::vector<TO_LANDMARK_68> landMark;
    return TO_Face_Tracker_Update_Simplify(face,landMark,pBGRA);
}
TO_LIB_STATUS TO_Face_Tracker_Update_Simplify(
    //脸部位置框结果输出
    std::vector<TO_RECT>& face,
    //特征点 长度固定为68
    std::vector<TO_LANDMARK_68>& landMark,
    //测试输入
    unsigned char* pBGRA)
{
    if (isInstantiation )
    {
        g_frameManager->sampling(pBGRA,g_frameWidth,g_frameHeight,false);
        if(g_trackerTeistedPool)
        {
            g_trackerTeistedPool -> checkConcurrence(face,landMark,NULL);
        }
        return STATUS_TO_LIB_NORMAL;
    }
    return STATUS_TO_LIB_ERROR;
}

/*
//停止线程，保留实例对象内变量资源
 */
TO_LIB_STATUS TO_Face_Tracker_Pause()
{
    if (isInstantiation )
    {
        //todo
        return STATUS_TO_LIB_NORMAL;
    }
    return STATUS_TO_LIB_ERROR;
}

/*
//使用类内变量资源启动线程
 */
TO_LIB_STATUS TO_Face_Tracker_Resume()
{
    if (isInstantiation)
    {
        //todo
        return STATUS_TO_LIB_NORMAL;
    }
    return STATUS_TO_LIB_ERROR;
}

/*
//当前是否存在脸
 */
TO_LIB_STATUS TO_Face_is_Face_Exist_Now()
{
    // if (isInstantiation && g_facedetector)
    // {
    //  if(g_facedetector->isFaceExit())
    //  {
    //      return STATUS_TO_LIB_YES;
    //  }else
    //  {
    //      return STATUS_TO_LIB_NO;
    //  }
    // }
    return STATUS_TO_LIB_ERROR;
}

/*
//实时处理接口
 */
TO_LIB_STATUS TO_Face_Tracker_Check(
    //脸部位置框结果输出
    std::vector<TO_RECT>& face,
    //阻塞超时阀值
    const unsigned int blockWaitTime, // ms
    //测试结果显示输出，NULL时不做结果显示工作，在主检测口会增加耗时～5ms
    unsigned char* pBGRA)
{
    if (isInstantiation)
    {
        if(g_trackerTeistedPool)
        {
            std::vector<TO_LANDMARK_68> landMark;
            g_trackerTeistedPool -> checkConcurrence(face,landMark,pBGRA);
        }
        
        return STATUS_TO_LIB_NORMAL;
    }
    return STATUS_TO_LIB_ERROR;
}
TO_LIB_STATUS TO_Face_Tracker_Check_With_Debug(
    //脸部位置框结果输出
    std::vector<TO_RECT>& face,
    //特征点 长度固定为68
    std::vector<TO_LANDMARK_68>& landMark,
    //测试结果显示输出，NULL时不做结果显示工作，在主检测口会增加耗时～5ms
    unsigned char* pBGRA)
{
    if (isInstantiation)
    {
        if(g_trackerTeistedPool)
        {
            g_trackerTeistedPool -> checkConcurrence(face,landMark,pBGRA);
        }
        
        return STATUS_TO_LIB_NORMAL;
    }
    return STATUS_TO_LIB_ERROR;
}



/*
//肤色滤波
 */
TO_LIB_STATUS TO_Face_Skin_Filter(
    //输出结果
    cv::Mat& out,
    //输入
    cv::Mat& in,
    //滤波器种类
    TO_SKIN_FILTER_KIND filtet)
{
    switch(filtet)
    {
    case TO_SKIN_FILTER_RGB:
        out = TO::RGB_detect(in);
        break;
    case TO_SKIN_FILTER_ELLIPSE:
        out = TO::ellipse_detect(in);
        break;
    case TO_SKIN_FILTER_YCRCB_OTSU:
        out = TO::YCrCb_Otsu_detect(in);
        break;
    case TO_SKIN_FILTER_YCRCB:
        out = TO::YCrCb_detect(in);

        // morphologyEx(src,dst,MORPH_OPEN,Mat(3,3,CV_8U),Point(-1,-1),1);
     //    imwrite("open.jpg",dst);

        //morphologyEx(out,out,MORPH_CLOSE,Mat(3,3,CV_8U),Point(0,0),1);
        //morphologyEx(out, out, MORPH_CLOSE, Mat(3,3,CV_8U),Point(-1,-1),1);

        break;
    case TO_SKIN_FILTER_HSV:
        out = TO::HSV_detector(in);
        break;
    default:
        return STATUS_TO_LIB_ERROR;
        break;
    }
    return STATUS_TO_LIB_NORMAL;
}
