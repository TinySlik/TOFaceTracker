//
//  FrameManager.h
//  FrameManager
//
//  Created by Tiny Oh 2018.7.14
//  Copyright (c) 2018 White Sky. All rights reserved.
//

#ifndef TO_THREAD_FRAME_MANAGER_H
#define TO_THREAD_FRAME_MANAGER_H

#include "TOThreadBase.h"
#include "TOFaceType.h"
#include <assert.h>

// ** sampling()  (fps)  remove() (tr) || 1 status loop 
// ** clear() status when exit thread 
// ** prepare() status before thread begin

namespace TO {

    class TOTracker;
    class TOFrameManager : public TOStatusThreadBase
    {
    public:
        TOFrameManager();
        virtual ~TOFrameManager();

        void sampling(const uchar* pBGRA,const size_t width,const size_t height,const bool isDetectAlpha);

        // extract 
        TO_QUEUE_FRAME extractCurrentFrameInUse();
        TO_QUEUE_FRAME extractLastFrameInUse();
        TO_QUEUE_FRAME extractIndexFrameInUse(size_t index);

        // relieve
        bool relieveFrameInUse(TO_QUEUE_FRAME& t);

        // fps and pool size (fps * (depth - 1)) is the longest time we saved at every moment.
        // some time when the fps is not match the effecient of machine ,
        // the fps can auto become slow ,at the same time the depth should be smaler to keep same thing
        // big than 2
        void setPoolDepth(size_t depth);

        inline bool isWorking(){return (this->getCurrentStatus() == "N2TO14TOFrameManagerE&STATUS_NORMAL");}

        inline cv::Mat getSimlarSample(){return m_simlarSample;}
        inline void setSimlarSample(const cv::Mat & src){m_simlarSample = src;}
        
    protected:
        virtual void afterStartThread();
        virtual void beforeStopThread();

    private:
        virtual std::string rule(const std::string& last);
        void normal(void);
        void sampling(void);
        void remove(void);
        void clear(void);
        void prepare(void);
        void error(void); 


        pthread_mutex_t mutex_release_queue_tofacetracke;
        pthread_mutex_t mutex_sampling_queue_tofacetracke;
        size_t m_PoolDepth;

        std::vector<TO_QUEUE_FRAME> m_FrameQueueSampling;
        std::vector<TO_QUEUE_FRAME> m_FrameQueueWorking;
        cv::Mat m_simlarSample;
        bool m_simlarSampledInitTag;
    };
}

#endif
