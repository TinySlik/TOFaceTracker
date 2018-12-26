//
//  FaceDetector.h
//  FaceDetector
//
//  Created by Tiny Oh 2018.1.14
//  Copyright (c) 2018 White Sky. All rights reserved.
//

#ifndef TO_FACE_DETECTOR_H
#define TO_FACE_DETECTOR_H
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

#include "kcftracker.hpp"
#include "preprocessFace.h"
#include "TOFrameManager.h"
#include "TOSkinFiltering.h"
#include "TOTrackerTwistedPool.h"


//#define FACE_CASCADE_FILENAME_DEFAULT "../cv_resource/haarcascade_frontalface_alt.xml" //path*
#define FACE_CASCADE_FILENAME "../cv_resource/lbpcascade_frontalface.xml" //path*
// #define EYE1_CASCADE_FILENAME_DEFAULT "../cv_resource/haarcascade_eye.xml"  //path*
// #define EYE2_CASCADE_FILENAME_DEFAULT "../cv_resource/haarcascade_eye_tree_eyeglasses.xml" //path*
#define DETECT_SAVE_MATTER_THR 3
#define DETECT_CHAINS_CONFIDENCE_THR 0.5
#define DETECT_FIX_CONFIDENCE_SPLIT_THR 0.1
#define DETECT_FIX_CONFIDENCE_ENABLE_THR 0.4
#define DETECT_FIX_CONFIDENCE_SKIP_THR 0.9
#define DETECT_SIMILAR_ENBALE_THR 5

namespace TO {
// chain holder
    typedef struct{
        size_t frameIndex;
        size_t rectIndex;
        std::vector<int> leftHand;
        std::vector<int> rightHand;
        cv::Rect rect;
    }STRATEGY_CHAIN_HOLDER_UNIT;

// in analisys
    typedef struct{
        long long timeStamp;
        std::vector<STRATEGY_CHAIN_HOLDER_UNIT> STfaces;
        std::vector<cv::Rect> sameTimeTrackerReses;
    }FRAME_STRATEGY_FACE_UNIT;


    class TOFrameManager;
    class TOFaceDetector:public  TOStatusThreadBase
    {
    public:
        TOFaceDetector(TOFrameManager & frameManager,string faceDefaultCascadeLocation,const size_t width,const size_t height);
        virtual ~TOFaceDetector();

        inline std::vector<FRAME_STRATEGY_FILTRATION_UNIT> getBridge(){return m_bridge;}

        inline void addTrackerPool(TOTrackerTwistedPool& pool){m_trackerTwistedPool = &pool; }
        void getBridgeFromTracker(std::vector<FRAME_STRATEGY_FILTRATION_UNIT>& src);

        void setFaceDetectCountEnableThreshold(int num);

        inline void setFilterEnable(bool is){m_isNeedSkinFilter = is;}
        inline void setSaveMatterThr(unsigned int thr){m_saveMatterThr = thr;}
        inline void setChainsConfidenceThr(double thr){m_chainsConfidenceThr = thr;}
        inline void setFixConfidenceSplitThr(double thr){m_fixConfidenceSplitThr = thr;}
        inline void setFixConfidenceEnableThr(double thr){m_fixConfidenceEnableThr = thr;}
        inline void setFixConfidenceSkipThr(double thr){m_fixConfidenceSkipThr = thr;}
        inline void setDetectActiveBetweenSimilarThr(unsigned int thr){m_DetectActiveBetweenSimilarThr = thr;}
    protected:
        virtual void afterStartThread();

    private:
        const size_t m_width;
        const size_t m_height;

        unsigned int m_saveMatterThr;
        double m_chainsConfidenceThr;
        double m_fixConfidenceSplitThr;
        double m_fixConfidenceEnableThr;
        double m_fixConfidenceSkipThr;
        unsigned int  m_DetectActiveBetweenSimilarThr;

        virtual std::string rule(const std::string& last);
        void predetect(void);
        void detect(void);
        void dormant(void);
        void error(void);
        void analysis(void);
        void prepare(void);

        bool FaceDetectResesToTracker(const FRAME_STRATEGY_FILTRATION_UNIT & to);

        int initDetectors(const std::string& name);
        bool ProcessFaceDetect(TO_QUEUE_FRAME& frame,std::vector<cv::Rect>& res );

        long long getTimeStampInterval(const long long & t1,const long long & t2);

        size_t getMinistIntervalIndex(std::vector<FRAME_STRATEGY_FACE_UNIT>& array);
        void makeChains(std::vector<FRAME_STRATEGY_FACE_UNIT>& array,float trustThreshold );
        // int  chainMerge(std::vector < std::vector <FRAME_STRATEGY_CHAIN_UNIT> >& chains,const vector<FRAME_STRATEGY_FACE_UNIT>& array); // todo
        void  prognosisChainHeadMergeReliability(vector<FRAME_STRATEGY_FACE_UNIT>& array); //todo
        std::vector<FRAME_STRATEGY_FILTRATION_UNIT> get2NodeTails(const std::vector<FRAME_STRATEGY_FACE_UNIT>& array);
        bool isBorderEqul(const cv::Rect& c1,const cv::Rect& c2); //&
        bool isBorderOn(const cv::Rect& c1,const cv::Rect& c2); //|
        float isBorderOnPer(const cv::Rect& c1,const cv::Rect& c2);//%

        void analysisTAndD(const FRAME_STRATEGY_FACE_UNIT & c);

        //围绕矩形中心缩放  
        cv::Rect rectCenterScale(cv::Rect rect, cv::Size size);


        cv::CascadeClassifier* m_faceCascade;

        TOFrameManager* m_frameManager;

        bool m_isNeedSkinFilter;

        TOSkinFilter* m_skinFilter;

        int m_faceDetectCountEnableThreshold;

        int m_tmpFaceDetectContinuityCount;

        size_t m_BridgeRealSize;

        std::vector<FRAME_STRATEGY_FACE_UNIT> m_resOnceST1;
        //std::vector<FRAME_STRATEGY_FILTRATION_UNIT> m_resOnceST2;
        // FRAME_STRATEGY_TRACKER_UNIT m_resOnceST3;

        TOTrackerTwistedPool* m_trackerTwistedPool;
        std::vector<FRAME_STRATEGY_FILTRATION_UNIT> m_bridge;

        cv::Rect m_ORIrect;
        bool m_firstTime;
    };
}


#endif