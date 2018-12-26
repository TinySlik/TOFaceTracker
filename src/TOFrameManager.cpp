//
//  FrameManager.cpp
//  FrameManager
//
//  Created by Tiny Oh 2018.1.14
//  Copyright (c) 2018 White Sky. All rights reserved.
//

#include "TOFrameManager.h"
using  namespace TO;

TOFrameManager::TOFrameManager():m_PoolDepth(2),m_simlarSampledInitTag(false)
{
    m_calssName = typeid(*this).name();

    TO_THREAD_STATUS_UNIT units[] =
    {
        {"N2TO14TOFrameManagerE&STATUS_PREPARE",(void (TOStatusThreadBase::*)())(&TOFrameManager::prepare)      },
        {"N2TO14TOFrameManagerE&STATUS_NORMAL", (void (TOStatusThreadBase::*)())(&TOFrameManager::normal)       },
        {"N2TO14TOFrameManagerE&STATUS_CLEAR",  (void (TOStatusThreadBase::*)())(&TOFrameManager::clear)        },
        {"N2TO14TOFrameManagerE&STATUS_ERROR",  (void (TOStatusThreadBase::*)())(&TOFrameManager::error)        }
    };

    for (int i = 0; i < sizeof(units)/sizeof(TO_THREAD_STATUS_UNIT); ++i)
    {
        addStatusUnit(units[i]);
    }
    defineStatusRule(&TOStatusThreadBase::rule);

}

std::string TOFrameManager::rule(const std::string& last)
{
    if(last == "N2TO14TOFrameManagerE&STATUS_NORMAL")
    {
        return "N2TO14TOFrameManagerE&STATUS_NORMAL";
    }else
    {
        return "N2TO14TOFrameManagerE&STATUS_ERROR";
    }
}

void TOFrameManager::afterStartThread()
{
    pthread_mutex_init(&mutex_release_queue_tofacetracke,NULL);
    pthread_mutex_init(&mutex_sampling_queue_tofacetracke,NULL);
}

void TOFrameManager::beforeStopThread()
{
    changeStatus("N2TO14TOFrameManagerE&STATUS_CLEAR");
}

TOFrameManager::~TOFrameManager()
{
    while(m_FrameQueueWorking.size() > 0)
    {
        TO_QUEUE_FRAME::remove(&(m_FrameQueueWorking[0]),true);
        m_FrameQueueWorking.erase(m_FrameQueueWorking.begin());
    }

    while(m_FrameQueueSampling.size() > 0)
    {
        TO_QUEUE_FRAME::remove(&(m_FrameQueueSampling[0]),true);
        m_FrameQueueSampling.erase(m_FrameQueueSampling.begin());
    }
}

void  TOFrameManager::sampling(
    const uchar* pBGRA,
    const size_t width,
    const size_t height,
    const bool isDetectAlpha)
{
    assert(pBGRA);

    TO_QUEUE_FRAME frameData = TO_QUEUE_FRAME::create(pBGRA,width,height,isDetectAlpha);

//pthread_mutex_lock (&mutex_sampling_queue_tofacetracke);
    m_FrameQueueSampling.push_back(frameData);
//pthread_mutex_unlock (&mutex_sampling_queue_tofacetracke);
    if(!m_simlarSampledInitTag)
    {
        m_simlarSampledInitTag = true;
        m_simlarSample = *(frameData.getData());
    }
}

void TOFrameManager::setPoolDepth(size_t depth)
{
    if(depth > 1)
    { 
        m_PoolDepth = depth;
        m_FrameQueueWorking.reserve(m_PoolDepth);
    }
}

void TOFrameManager::error(void)
{
//todo
}

void TOFrameManager::normal(void)
{
    sampling();
    remove();
}

void TOFrameManager::sampling(void)
{
#if 0
cout <<"m_FrameQueueSampling" << m_FrameQueueSampling.size() << endl;
#endif
    double designedTimeInterval = 1000.0*1000.0 / getFPSSuperiorLimit();
    while(0 == m_FrameQueueSampling.size())
    {   
        double wait = 100.0;
        usleep(wait);
        designedTimeInterval -= wait;
        if(designedTimeInterval < 0)
        {
#if 1
            cout << "WARNING: no frame sampled ,the camera frame get thread is too slow than the frame manager!" << endl;
#endif      
            return;
        }
    }

//pthread_mutex_lock (&mutex_sampling_queue_tofacetracke);
    int count = 0;

    while(m_FrameQueueSampling.size() > 1)
    {
        count ++;
#if 0
cout << "stop ERROR! while(m_FrameQueueSampling.size() > 1)" << endl;
#endif
        TO_QUEUE_FRAME::remove(&(m_FrameQueueSampling[0]),true);
        m_FrameQueueSampling.erase(m_FrameQueueSampling.begin());
    }
#if 1
    if(count)
    {
        cout << "skip "  << count << " frames/loop" << endl;
    }
#endif
    pthread_mutex_lock (&mutex_sampling_queue_tofacetracke);
    m_FrameQueueWorking.push_back(m_FrameQueueSampling[0]);
    pthread_mutex_unlock (&mutex_sampling_queue_tofacetracke);

    m_FrameQueueSampling.clear();
//pthread_mutex_unlock (&mutex_sampling_queue_tofacetracke);
}
void TOFrameManager::remove(void)
{
#if 0
cout <<"m_FrameQueueWorking.size():  " << m_FrameQueueWorking.size() << endl;
#endif  
    pthread_mutex_lock (&mutex_sampling_queue_tofacetracke);
    if(m_FrameQueueWorking.size() > m_PoolDepth)
    {
        TO_QUEUE_FRAME::remove(&(m_FrameQueueWorking[0]),true);
// while(true)
// {
//  //
// }
        m_FrameQueueWorking.erase(m_FrameQueueWorking.begin());
    }else
    {
        pthread_mutex_unlock (&mutex_sampling_queue_tofacetracke);
        return;
    }

    if(m_FrameQueueWorking.size() > m_PoolDepth)
    {
        TO_QUEUE_FRAME::remove(&(m_FrameQueueWorking[0]),true);
// while(true)
// {
//  //
// }
        m_FrameQueueWorking.erase(m_FrameQueueWorking.begin());
    }
    pthread_mutex_unlock (&mutex_sampling_queue_tofacetracke);
}

void TOFrameManager::clear(void)
{
    printf("[TOFrameManager::clear]\n");
}

void TOFrameManager::prepare(void)
{
    static bool preTag = true;
    if(preTag){
        cout << "[TOFrameManager::prepare]" << endl;
        preTag = false;
    }

    if(m_FrameQueueSampling.size() > 0)
    {
        changeStatus("N2TO14TOFrameManagerE&STATUS_NORMAL");
    }
}

// extract 
TO_QUEUE_FRAME TOFrameManager::extractCurrentFrameInUse()
{
    return extractIndexFrameInUse(m_FrameQueueWorking.size()-1);
}
TO_QUEUE_FRAME TOFrameManager::extractLastFrameInUse()
{
    return extractIndexFrameInUse(0);
}
TO_QUEUE_FRAME TOFrameManager::extractIndexFrameInUse(size_t index)
{
    pthread_mutex_lock (&mutex_sampling_queue_tofacetracke);
    if(index >= m_FrameQueueWorking.size())
    {
        if(m_FrameQueueWorking.size() == 0)
        {
            cout << "error : m_FrameQueueWorking size is 0" << endl;
            assert(m_FrameQueueWorking.size());
            pthread_mutex_unlock (&mutex_sampling_queue_tofacetracke);
            return TO_QUEUE_FRAME::createEmptyFrameInError();
        }else
        {
            cout << "[WARNING] out of m_FrameQueueWorking range use new last instead" << endl;
            m_FrameQueueWorking[m_FrameQueueWorking.size()-1].setUsingExtract();
            TO_QUEUE_FRAME res = m_FrameQueueWorking[m_FrameQueueWorking.size()-1];
            pthread_mutex_unlock (&mutex_sampling_queue_tofacetracke);
            return res;
        }
    }
    m_FrameQueueWorking[index].setUsingExtract();
    TO_QUEUE_FRAME res = m_FrameQueueWorking[index];
    pthread_mutex_unlock (&mutex_sampling_queue_tofacetracke);
    return res;
}

// relieve
bool TOFrameManager::relieveFrameInUse(TO_QUEUE_FRAME& t)
{
    pthread_mutex_lock (&mutex_sampling_queue_tofacetracke);
    for (int i = 0; i < m_FrameQueueWorking.size(); ++i)
    {
        if(m_FrameQueueWorking[i].getData() == t.getData())
        {
            m_FrameQueueWorking[i].setRelieveAble();
            pthread_mutex_unlock (&mutex_sampling_queue_tofacetracke);
            return true;
        }
    }
    pthread_mutex_unlock (&mutex_sampling_queue_tofacetracke);
    cout << "error" << endl;
    assert(false);
    return false;
}
