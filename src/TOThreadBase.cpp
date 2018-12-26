//
//  ThreadBase.cpp
//  ThreadBase
//
//  Created by Tiny Oh 2018.1.14
//  Copyright (c) 2018 White Sky. All rights reserved.
//

#include "TOThreadBase.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>

using namespace TO;
using namespace std;

std::vector<TO_THREAD_MANAGER_UNIT>TOThreadBase::m_objs;
bool TOThreadBase::m_threadExitTag = true;
bool TOThreadBase::m_threadStopTag = false;
bool TOThreadBase::m_beginErrorTag = false;

TOThreadBase::TOThreadBase():
m_dormantTag(false),
m_threadLiveStatus(false),
m_threadResponseInterval(DEFAULT_RESPONSE_INTERVAL),
m_SLfps(256),
m_IntegerMultipleTimeIntervalEnableTag(true)
{
    m_threadStopTag = false;
    m_threadExitTag =  true;
    m_beginErrorTag = false;
    m_calssName = typeid(*this).name();
    TO_THREAD_MANAGER_UNIT unit;
    unit.obj  =this;
    unit.isThreadExist=false;
    m_objs.push_back(unit);
}

TOThreadBase::~TOThreadBase()
{
    for (int i = 0; i < m_objs.size(); ++i)
    {
        if(m_objs[i].obj  ==this)
        {
            m_objs.erase(m_objs.begin()+i);
            break;
        }
    }

    if(m_threadLiveStatus)
    {
        stopThread();
    }
}

bool TOThreadBase::startThread()
{
    if(m_threadLiveStatus)
    {
        cerr << "already have one thread runing" << endl;
        return false;
    }
    m_threadLiveStatus = true;
    int retT = pthread_create(&m_threadID, NULL, TOThreadBase::threadMain, this);
    if (retT != 0)
    {
        cerr << "[TOThreadBase::startThread] start pthread_create error: error_code=" << retT << endl;
        m_threadLiveStatus = false;
        return false;
    }
    return true;
}

bool TOThreadBase::dormantThread()
{
    m_dormantTag = !m_dormantTag;
    return m_dormantTag;
}

bool TOThreadBase::stopThread()
{
    if(m_threadLiveStatus)
    {
        m_threadLiveStatus = false;
        void* returnValue;
        pthread_join(m_threadID,&returnValue);
        return true;
    }else
    {
#if _DEBUG_TO_THREAD_BASE
        cerr << "none thread runing" << endl;
#endif
        return false;
    }
}

void TOThreadBase::run()
{
    cout << "no implement to run , run the base string output" << endl;
}

bool TOThreadBase::stopAllThread()
{
    if(m_beginErrorTag)
    {
        return true;
    }
    m_threadStopTag = false;
    m_threadExitTag = true;

    while(true)
    {
        bool res = false;
        for (int i = 0; i < m_objs.size(); ++i)
        {
            res = res || m_objs[i].isThreadExist;
        }

        if(!res)
        {
            break;
        }
    }

    for (int j = m_objs.size()-1; j >= 0 ; --j)
    {
        m_objs[j].obj->beforeStopThread();
    }

    m_threadStopTag  =  true;

    for (int k = 0; k < m_objs.size(); ++k)
    {
        m_objs[k].obj->stopThread();
    }

    cout << "[TOThreadBase::stopAllThread] success" << endl;
    return true;
}

void TOThreadBase::beforeStopThread()
{
#if _DEBUG_TO_THREAD_BASE
    cout <<  "[TOThreadBase::beforeStopThread]no implement" << endl;
#endif
}

void TOThreadBase::afterStartThread()
{
#if _DEBUG_TO_THREAD_BASE
    cout <<  "[TOThreadBase::afterStartThread]no implement" << endl;
#endif
}

bool TOThreadBase::startAllThread()
{
    m_threadStopTag = false;
    m_threadExitTag =  true;
    m_beginErrorTag = false;

    int breakNum = 0; 
    for (int i = 0; i < m_objs.size(); ++i)
    {
        m_objs[i].isThreadExist = true;
        if(m_objs[i].obj->startThread())
        {}else{
            m_objs[i].isThreadExist = false;
            cout << "error in begin ,close everythings" << endl;
            m_beginErrorTag = true;
            breakNum = i;
            break;
        }
    }
    if(m_beginErrorTag)
    {
        for (int j = 0; j < breakNum; ++j)
        {
            m_objs[j].obj->stopThread();
        }
        return false;
    }
    for (int j = m_objs.size()-1; j >= 0 ; --j)
    {
        m_objs[j].obj->afterStartThread();
    }

    cout << "[TOThreadBase::startAllThread] objs sum:" << m_objs.size() << endl;

    m_threadExitTag = false;
    return true;
}

void* TOThreadBase::threadMain(void* args)
{
    TOThreadBase * pThread = static_cast<TOThreadBase *>(args);

    while (m_threadExitTag)
    {
#if  _DEBUG_TO_THREAD_BASE
        cout << pThread->m_threadID << "in wait begin mode" << endl;
#endif
        if(m_beginErrorTag)
        {
            return NULL;
        }
        usleep(pThread->m_threadResponseInterval/10);
    }

    while(pThread->m_threadLiveStatus)
    {
        pThread->onUpdate();
        bool breakTag = false;
        bool waitBreakTag  =false;

        double designedTimeInterval = 1000.0*1000.0 / pThread->m_SLfps;
        //cout << designedTimeInterval << endl;

        while(m_threadExitTag)
        {
#if  _DEBUG_TO_THREAD_BASE
            cout << pThread->m_threadID << "in wait command mode" << endl;
#endif
            if(!waitBreakTag)
            {
                for (int i = 0; i < m_objs.size(); ++i)
                {
                    if(m_objs[i].obj == pThread)
                    {
                        m_objs[i].isThreadExist = false;
                        cout << "["  << pThread->m_threadID << "]-[" << pThread->m_calssName <<  "]" << "release ready" << endl;
                        waitBreakTag = true;
                    }
                }
            }
            
            if(m_threadStopTag)
            {
                breakTag = true;
                break;
            }
            usleep(pThread->m_threadResponseInterval/10);
        }
        if(breakTag)
        {
            break;
        }
#if  _DEBUG_TO_THREAD_BASE
        cout << pThread->m_threadID << "empty run" << endl;
#endif
        if(pThread->m_dormantTag)
        {
            usleep(designedTimeInterval);
        }else
        {
            int start = cv::getCPUTickCount();
            try
            {
                pThread->run();
            }catch(cv::Exception &e)
            {
                cerr << "ERROR:[TOStatusThreadBase]=============================================================" << pThread->m_threadID << endl;
            }
            int end = cv::getCPUTickCount();

            double tealInterval = (end - start) / cv::getTickFrequency();
            
#if 0
            double fps = (15 * fps + (1 / tealInterval)) / 16;
            cout << "["  << pThread->m_threadID << "]-[" << pThread->m_calssName <<  "]" ;
            printf("Time working per frame: %f\t theory max FPS: %3.3f\n", tealInterval, fps);
#endif
            double s = tealInterval* 1000 * 1000;
            if(s < designedTimeInterval)
            {
                usleep(designedTimeInterval - s);
            }else
            {
#if 0
                cout << "WARING:[TOStatusThreadBase]-["  << pThread->m_threadID << "]-[" << pThread->m_calssName <<  "]" <<  "  slow then designed time value!." << endl;
#endif
                if(pThread->m_IntegerMultipleTimeIntervalEnableTag)
                {
                    int count = 0;
                    double p = designedTimeInterval;
                    while(true)
                    {
                        s -= p;
                        count++;
                        if(s < 0)
                        {
                            break;
                        }
                    }
                    usleep(0-s);
#if 0
                    cout<< "inter simplify count :" << count << "|||" << tealInterval* 1000 * 1000 - s << endl;
#endif
                }
            }
        }
    }

    return NULL;
}

void TOThreadBase::onUpdate()
{
#if _DEBUG_TO_THREAD_BASE
    cout <<  "[TOThreadBase::onUpdate]no implement" << endl;
#endif
}

TOStatusThreadBase::TOStatusThreadBase():m_rule(NULL),isInRun(false)
{
    m_calssName = typeid(*this).name();
}

TOStatusThreadBase::~TOStatusThreadBase()
{}
void TOStatusThreadBase::addStatusUnit(TO_THREAD_STATUS_UNIT& unit)
{
    m_baseStatusFuncList.push_back(unit);
    if(1 == m_baseStatusFuncList.size())
    {
        m_current = m_baseStatusFuncList[0];
    }
}

void TOStatusThreadBase::defineStatusRule(TO_STATUS_RULE func)
{
    m_rule = func;
}

void TOStatusThreadBase::run()
{
    isInRun  = true;
    if(m_baseStatusFuncList.size() > 0)
    {
        (this->*m_current.StatusExecute)();
    }
#if _DEBUG_TO_THREAD_BASE
    else
    {
        cout << "no unit in status list" << endl;
    }
#endif
    isInRun  = false;
}  

void TOStatusThreadBase::nextStatus(bool isNeedToWait)
{
    if(NULL == m_rule)
    {
        cout << "no rule define with normal status change." << endl;
        return;
    }
    
    if(isNeedToWait)
    {
        while(isInRun)
        {
            usleep(m_threadResponseInterval/10);
        }
    }

    string next =  (this->*m_rule)(m_current.StatusName);
    for (int i = 0; i < m_baseStatusFuncList.size(); ++i)
    {
        if(m_baseStatusFuncList[i].StatusName == next)
        {
            string temp = m_current.StatusName;
#if _DEBUG_TO_THREAD_BASE
            cout  << "[TOStatusThreadBase::changeStatus] success" <<  temp + "2" + next <<  endl;
#endif
            m_current = m_baseStatusFuncList[i];
            return;
        }
    }
}

void TOStatusThreadBase::changeStatus(string target,bool isNeedToWait)
{
    if(m_current.StatusName   ==  target)
    {
        return;
    }
    for (int i = 0; i < m_baseStatusFuncList.size(); ++i)
    {
        string temp = m_current.StatusName;
        if(m_baseStatusFuncList[i].StatusName == target)
        {
            m_current = m_baseStatusFuncList[i];
            
#if _DEBUG_TO_THREAD_BASE
            cout  << "[TOStatusThreadBase::changeStatus]   success" <<  temp + "2" + target <<  endl;
#endif
            if(isNeedToWait)
            {
                while(isInRun)
                {
                    usleep(m_threadResponseInterval/10);
                }
            }
            return;
        }
    }
}

std::string TOStatusThreadBase::rule(const std::string& last)
{
    for (int i = 0; i < m_baseStatusFuncList.size(); ++i)
    {
        if(last == m_baseStatusFuncList[i].StatusName)
        {
            if(i == (m_baseStatusFuncList.size()-1))
            {
                return m_baseStatusFuncList[0].StatusName;
            }else
            {
                return m_baseStatusFuncList[i+1].StatusName;
            }
        }
    }
    return "empty";
}
