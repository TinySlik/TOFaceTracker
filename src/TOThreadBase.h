//
//  ThreadBase.h
//  ThreadBase
//
//  Created by Tiny Oh 2018.1.14
//  Copyright (c) 2018 White Sky. All rights reserved.
//

#ifndef TO_THREAD_BASE_H
#define TO_THREAD_BASE_H

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include <typeinfo>
#include <vector>
#include <map>
#include <string>

#define _DEBUG_TO_THREAD_BASE false
#define DEFAULT_RESPONSE_INTERVAL 2000

namespace TO {
    class TOThreadBase;
    class TOStatusThreadBase;
    typedef struct 
    {
        std::string StatusName;
        void (TOStatusThreadBase::*StatusExecute)(void);
    }TO_THREAD_STATUS_UNIT;

    typedef struct 
    {
        TOThreadBase* obj;
        bool isThreadExist;
    }TO_THREAD_MANAGER_UNIT;

    typedef std::string (TOStatusThreadBase::*TO_STATUS_RULE)(const std::string&);

    class TOThreadBase
    {
    public:
        TOThreadBase();
        virtual ~TOThreadBase();
        virtual void run();
        bool dormantThread();

        static std::vector<TO_THREAD_MANAGER_UNIT>m_objs;

        static bool stopAllThread();
        static bool startAllThread();

        inline void setFPSSuperiorLimit(size_t SLfps){m_SLfps = SLfps;}
        inline size_t getFPSSuperiorLimit(void){return m_SLfps;}
        inline void setIntegerMultipleTimeInterval(bool isEnable){m_IntegerMultipleTimeIntervalEnableTag = isEnable;}
        
    protected:
        virtual void beforeStopThread();
        virtual void afterStartThread();
        virtual void onUpdate();
        
        pthread_t m_threadID;
        bool m_dormantTag;
        bool m_threadLiveStatus;
        static bool m_threadExitTag;
        static bool m_threadStopTag;
        static bool m_beginErrorTag;
        std::string m_calssName;
        unsigned long m_threadResponseInterval;
        size_t m_SLfps; //default 256 for max speed fast enough

    private:
        
        bool m_IntegerMultipleTimeIntervalEnableTag; // default is true (make the time pass interger multiple db)
        bool startThread();
        bool stopThread();
        static void* threadMain(void* args);
    };

    class TOStatusThreadBase:public TOThreadBase
    {
    public:
        TOStatusThreadBase();
        virtual ~TOStatusThreadBase();
        
        virtual void run();

        virtual std::string rule(const std::string& last);
        void addStatusUnit(TO_THREAD_STATUS_UNIT& unit);
        void defineStatusRule(TO_STATUS_RULE func);
        void changeStatus(std::string str,bool isNeedToWait = false);
        inline std::string getCurrentStatus(){return m_current.StatusName;};
        inline bool isInStatus(const std::string& last){return m_current.StatusName == last;};
        void nextStatus(bool isNeedToWait = false);

    protected:
        TO_STATUS_RULE m_rule;
        TO_THREAD_STATUS_UNIT m_current;
        bool isInRun;

    private:
        std::vector<TO_THREAD_STATUS_UNIT> m_baseStatusFuncList;
    };
}

#endif