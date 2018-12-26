//
//  TOTrackerTwistedPool.h
//  TOTrackerTwistedPool
//
//  Created by Tiny Oh 2018.1.14
//  Copyright (c) 2018 White Sky. All rights reserved.
//

#ifndef TO_TRACKER_TWISTED__POOL_H
#define TO_TRACKER_TWISTED__POOL_H

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <queue>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>

#include "dlib/image_processing/frontal_face_detector.h"
#include "dlib/image_processing/render_face_detections.h"
#include "dlib/image_processing.h"
#include "dlib/gui_widgets.h"
#include "dlib/image_io.h"
#include "dlib/opencv.h"

#include "kcftracker.hpp"
#include "TOThreadBase.h"
#include "TOFaceType.h"

#define _DEBUG_TO_TRACKER_TWISTED_POOL false

#define SHAPE_PREDICTOR_DEFAULT "../res/shape_predictor_68_face_landmarks.dat"
#define TRACKER_OUT_OF_VIEW_THR 0.8
#define THEORETICAL_UPPER_LIMIT 10
#define ERROR_COUNT_RISK_TOLERANCE_THR 3
#define TRACKER_KEEP_COUNT_THR 50
#define TRACKER_DORMANT_COUNT_ENABLE_THR 3
#define HOG false
#define FIXEDWINDOW false
#define MULTISCALE true
#define SILENT true
#define LAB false

namespace TO {

    class TOTracker;
    class TOFrameManager;
    class TOFaceDetector;
    class TOTrackerTwistedPool:public TOStatusThreadBase
    {
    public:
        TOTrackerTwistedPool(TOFrameManager & frameManager,TOFaceDetector & detector,const size_t width,const size_t height);
        virtual ~TOTrackerTwistedPool();

        void checkConcurrence(std::vector<TO_RECT> & vv,std::vector<TO_LANDMARK_68>& landMark,uchar* pBGRA = NULL);

        inline std::vector<FRAME_STRATEGY_FILTRATION_UNIT> getBridge(){return m_bridge;}

        void getBridgeFromDetector(std::vector<FRAME_STRATEGY_FILTRATION_UNIT>& src);

        void setMaxFaceCount(size_t num);

        inline void setSyncEnable(bool enable){m_isSyncEnable = enable;}
        inline void noticeLoopEnd(){m_OneDetectLoopTag = true;}
        //inline bool getLoopTag(){return m_OneDetectLoopTag;}
        void setLandMarkEnable(bool enable);

        void tryToActiveObject(const cv::Rect& src,size_t index);
        void tryToDormantObject(size_t index);
        void tryToFixObject(const cv::Rect& src,size_t index);
        void keepObjectWell(size_t index);
        
        inline void setShapeLandMarkFile(std:: string fileName){m_ShapeLandMarkFileName = fileName;}
        inline void setOutOfViewThr(double thr){m_outOfViewThr = thr;}
        inline void setErroCountRiskToleranceThr(unsigned int thr){m_ErroCountRiskToleranceThr = thr;}
        inline void setTrackerKeepCountThr(unsigned int thr){m_TrackerKeepCountThr = thr;}
        inline void setTrackerDormantCountEnableThr(unsigned int thr){m_TrackerDormantCountEnableThr = thr;}
        inline void setKCFTRackerConfigHOGEnable(bool enable){ m_kcfHOG = enable; }
        inline void setKCFTRackerConfigFIXEDWINDOWEnable(bool enable){ m_kcfFIXEDWINDOW = enable; }
        inline void setKCFTRackerConfigMULTISCALEEnable(bool enable){ m_kcfMULTISCALE = enable; }
        inline void setKCFTRackerConfigSILENTEnable(bool enable){ m_kcfSILENT = enable; }
        inline void setKCFTRackerConfigLABEnable(bool enable){ m_kcfLAB = enable; }

    protected:
        virtual void beforeStopThread();
        virtual void afterStartThread();

    private:
        virtual std::string rule(const std::string& last);
        void pretrack(void);
        void track(void);
        void prepare(void);
        void dormant(void);
        void error(void);

        bool ProcessKCFTracker(KCFTracker& kcftracker, TO_QUEUE_FRAME& frame,cv::Rect& rect);
        bool InitKCFTracker(KCFTracker& kcftracker,TO_QUEUE_FRAME& frame,const cv::Rect& rect,int& resTag);

        dlib::array2d<dlib::rgb_pixel> cvMatToDLibRgb(const cv::Mat & cvMatRgb);
        dlib::array2d<dlib::bgr_pixel> cvMatToDLibBgr(const cv::Mat & cvMatBgr);

        void checkAndResetSkipTag();
        void checkAndJudgeStopOrActive();

        void checkIsOutOfView();

        std::string m_ShapeLandMarkFileName;
        double m_outOfViewThr;
        unsigned int m_ErroCountRiskToleranceThr;
        unsigned int  m_TrackerKeepCountThr;
        unsigned int  m_TrackerDormantCountEnableThr;
        bool m_kcfHOG;
        bool m_kcfFIXEDWINDOW;
        bool m_kcfMULTISCALE;
        bool m_kcfSILENT;
        bool m_kcfLAB;
        bool m_preLogTag;

        bool m_OneDetectLoopTag;

        const size_t m_width;
        const size_t m_height;

        TOFrameManager* m_frameManager;

        TOFaceDetector* m_detector;
        
        vector<KCFTracker> m_workerQueue;
        std::vector<FRAME_STRATEGY_FILTRATION_UNIT> m_bridge;
        std::vector<int> m_bridgeDormantCount;

        long long m_tmpTimeStampLast;
        vector<cv::Rect> m_tmpRectRess;
        vector<int> m_tmpRectPreRess;
        long long m_tmpLoopFrameCountSum;

        bool m_isSyncEnable;
        bool m_isLandMarkEnable;

        dlib::array2d<dlib::bgr_pixel> m_dlibImg;
        dlib::shape_predictor m_dlibSp;

        std::vector<TO_RECT>  m_resFaceLast;
        std::vector<TO_LANDMARK_68> m_resLandMarkLast;

        long long m_checkTimeStampLast;
    };
}
#endif