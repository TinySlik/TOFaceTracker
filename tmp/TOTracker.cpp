//
//  TOTracker.cpp
//  TOTracker
//
//  Created by Tiny Oh 2018.1.14
//  Copyright (c) 2018 White Sky. All rights reserved.
//
//

#include "TOTracker.h"


using  namespace TO;

TOTracker::TOTracker(TOTrackerTwistedPair& pair):
m_tracker(HOG, FIXEDWINDOW, MULTISCALE, LAB),
m_father(&pair),
m_trackerCheckReadyTag(true),
m_selfAdaptTimeCycleThreshold(1000),
m_trackerNormalKeepTime(10000),
m_isNeedSkipTrance(false)
{
	m_calssName = typeid(*this).name();
	TO_THREAD_STATUS_UNIT units[] =
	{
        {"N2TO9TOTrackerE&STATUS_WAIT",         (void (TOStatusThreadBase::*)())(&TOTracker::wait)      },
		{"N2TO9TOTrackerE&STATUS_DORMANT",		(void (TOStatusThreadBase::*)())(&TOTracker::dormant)	},
		{"N2TO9TOTrackerE&STATUS_TRANCE",		(void (TOStatusThreadBase::*)())(&TOTracker::trance)	},
		{"N2TO9TOTrackerE&STATUS_ERROR",		(void (TOStatusThreadBase::*)())(&TOTracker::error)		},
		{"N2TO9TOTrackerE&STATUS_WORK",			(void (TOStatusThreadBase::*)())(&TOTracker::work)		}
	};
	
	for (int i = 0; i < sizeof(units)/sizeof(TO_THREAD_STATUS_UNIT); ++i)
	{
		addStatusUnit(units[i]);
	}
	defineStatusRule(&TOStatusThreadBase::rule);
}

std::string TOTracker::rule(const std::string& last)
{
	if(last == "N2TO9TOTrackerE&STATUS_DORMANT")
	{
		return "N2TO9TOTrackerE&STATUS_DORMANT";
	}else if(last == "N2TO9TOTrackerE&STATUS_WAIT")
	{
		return "N2TO9TOTrackerE&STATUS_TRANCE";
	}else if(last == "N2TO9TOTrackerE&STATUS_TRANCE")
	{
		return "N2TO9TOTrackerE&STATUS_WORK";
	}else if(last == "N2TO9TOTrackerE&STATUS_WORK")
	{
		return "N2TO9TOTrackerE&STATUS_WAIT";
	}else
	{
		return "N2TO9TOTrackerE&STATUS_ERROR";
	}
}

TOTracker::~TOTracker()
{
	changeStatus("N2TO9TOTrackerE&STATUS_DORMANT",true);
}

void TOTracker::wait(void)
{   
	usleep(m_threadResponseInterval);
}
void TOTracker::dormant(void)
{
	usleep(m_threadResponseInterval);
}
void TOTracker::trance(void)
{
    while(m_father->getQueueSize() == 0)
    {
        cerr << "[TOTracker::trance] no date in queue!"  << endl;
        usleep(m_threadResponseInterval/10);
    }
    Rect targetFaceRect;
    if(m_father->getResault(targetFaceRect))
    {
    	//
    }else
    {
    	changeStatus("N2TO9TOTrackerE&STATUS_ERROR");
    	return;
    }

    TO_QUEUE_FRAME*  target = m_father->getCurrentFrameInUse();
    InitKCFTracker(m_tracker, *target,targetFaceRect);
    m_father->releaseCurrentFrameInUse(target);

    if(m_isNeedSkipTrance)
    {
        unsigned int skipCount = 0;
        while(m_father->getQueueSize()>0)
        {
#if  _DEBUG_TO_TRACKER
            cout  << "[TOTracker::trance]" << "[m_father->getQueueSize()]" << m_father->getQueueSize() <<  endl;
#endif
            bool waitToBreaK= false;
            if(1 >= m_father->getQueueSize())
            {
                waitToBreaK = true;
            }
            target = m_father->getCurrentFrameInUse();
            
            if(0 == skipCount)
            {
                ProcessKCFTracker(m_tracker, *target,m_TrackerRect);
#if _DEBUG_TO_TRACKER
                cout << "[TOTracker::trance]  m_father->getQueueSize()"   << m_father->getQueueSize()  << endl;
#endif
                int countInMind = m_father->getQueueSize();
                skipCount  = 1;
                while(countInMind > 2 )
                {
                    countInMind/=2;
                    skipCount *=2;
                }
            }else
            {
                skipCount --;
            }
            m_father->releaseCurrentFrameInUse(target);
            if(waitToBreaK)
            {
                break;
            }
        }
    }else
    {
        target = m_father->getLastFrameInUse();
        ProcessKCFTracker(m_tracker, *target,m_TrackerRect);
        m_father->releaseCurrentFrameInUse(target);
    }
    
    nextStatus();
    m_father->dormantMinorTracker();
}

cv::Rect TOTracker::getRes()
{
	if(isInNormalStatus())
	{
		return m_TrackerRect;
	}else
	{
#if _DEBUG_TO_TRACKER
        cout << "[ TOTracker::getRes] check in unnormal  status\n" << endl;
#endif
		return Rect0;
	}
}

bool TOTracker::isInNormalStatus()
{
	if(m_current.StatusName == "N2TO9TOTrackerE&STATUS_WORK" || m_current.StatusName == "N2TO9TOTrackerE&STATUS_TRANCE" )
	{
		return  true;
	}
	return  false;
}

void TOTracker::work()
{
#if _DEBUG_TO_TRACKER
    cout  <<"[TOTracker::work]" << endl;
#endif
	long long startTime = getTickCount();

    m_distanceSum = 0;
    bool m_checkTag =  false;
    m_tempRectNow =  Rect0;
    while(m_current.StatusName == "N2TO9TOTrackerE&STATUS_WORK"   &&  !m_threadExitTag)
    {
#if _DEBUG_TO_TRACKER
        cout  <<"[TOTracker::work] in" << endl;
#endif 
        bool skipTag =false;
        long long kkTime = getTickCount();
        while(m_trackerCheckReadyTag || m_father->getQueueSize() <= 0)
        {
            
#if _DEBUG_TO_TRACKER
            cout  <<  "[TOTracker::work]m_father->getQueueSize() " << m_father->getQueueSize() << " ; wait to check" <<endl;
#endif 
            if(m_current.StatusName == "N2TO9TOTrackerE&STATUS_WORK"  &&  !m_threadExitTag)
            {
            }else
            {
                skipTag  =  true;
                break;
            }
            if(((double)getTickCount() - kkTime)*1000/getTickFrequency()  > 1000 )
            {
                break;
            }
            usleep(m_threadResponseInterval/2);
        }
        if(skipTag)
        {
            continue;
        }

        TO_QUEUE_FRAME*  target = m_father->getLastFrameInUse();
        ProcessKCFTracker(m_tracker, *target,m_TrackerRect);
        m_father->    releaseCurrentFrameInUse(target);
        
        if(Rect0  == m_tempRectNow)
        {
            m_tempRectNow = m_TrackerRect;
        }

        m_trackerCheckReadyTag   = true;
        
        if(!m_checkTag && m_tempRectNow != m_TrackerRect)
        {
            long long now = getTickCount();
            Point center1 = Point(m_tempRectNow.x + m_tempRectNow.width/2 , m_tempRectNow.y + m_tempRectNow.height/2);
            Point center2 = Point(m_TrackerRect.x + m_TrackerRect.width/2 , m_TrackerRect.y + m_TrackerRect.height/2);
            double distance = sqrtf(powf((center1.x - center2.x),2) + powf((center1.y - center2.y),2));
            
            double areaValue1 = sqrtf(m_tempRectNow.width * m_tempRectNow.height);
            double areaValue2 = sqrtf(m_TrackerRect.width * m_TrackerRect.height);
            double areaValue = abs(areaValue1 - areaValue2)*10;

            m_distanceSum += distance;
            m_distanceSum += areaValue;
#if _DEBUG_TO_TRACKER            
            cout <<"[STATUS_FACE_TRACKER_RUN_NORMAL] m_distanceSum :" << m_distanceSum << endl;
            cout <<"[STATUS_FACE_TRACKER_RUN_NORMAL] areaValue :" << areaValue << endl;
            cout <<"[STATUS_FACE_TRACKER_RUN_NORMAL] distance :" << distance << endl;
#endif 
            target = m_father->getLastFrameInUse();
            if(isBigChangeHappen())
            {
#if _DEBUG_TO_TRACKER
                cout  << "[TOTracker::work] to fast frame object position  may  be occur." << endl;
#endif
            }
            if( m_TrackerRect.x < 0 || 
                m_TrackerRect.y < 0 || 
                (m_TrackerRect.y + m_TrackerRect.height) > (target->data->rows) ||
                (m_TrackerRect.x + m_TrackerRect.width) > (target->data->cols)  ||
                (!m_isNeedSkipTrance  && (m_distanceSum > m_selfAdaptTimeCycleThreshold ||
                (double)(now - startTime)*1000/getTickFrequency() > m_trackerNormalKeepTime 
                || isBigChangeHappen()))
                )
            {
                m_tracker.setBigChange(false);

                m_checkTag = true;
#if _DEBUG_TO_TRACKER 
                cout << "=====================================it's time to check ====================================="  <<  endl;
#endif 
                m_father->noticeDetectorWork();
            }else
            {
                m_tempRectNow =  m_TrackerRect;
            }
            m_father->releaseCurrentFrameInUse(target);
        }
    }
    m_distanceSum  = 0;
}

bool TOTracker::ProcessKCFTracker(KCFTracker& kcftracker, TO_QUEUE_FRAME& frame,cv::Rect& rect)
{
	cv::Mat* in  =  frame.data;
#if _TIME_DEBUG
    double times = (double)getTickCount();
#endif
    rect = kcftracker.update(*in);
#if _TIME_DEBUG
    times = (double)getTickCount() - times;
    printf( "\n[TOTracker::ProcessKCFTracker][TIME]ProcessKCFTracker times cust frame time  %lf \n",times*1000/getTickFrequency());
    cout  << "resault" << rect <<endl;
#endif
}

bool TOTracker::InitKCFTracker(KCFTracker& kcftracker,TO_QUEUE_FRAME& frame,const cv::Rect& rect)
{
	cv::Mat* in  =  frame.data;
#if _TIME_DEBUG
    double times = (double)getTickCount();
#endif
    kcftracker.init( rect , *in);
#if _TIME_DEBUG
    times = (double)getTickCount() - times;
    printf( "\n[TOTracker::InitKCFTracker][TIME]InitKCFTracker times cust frame time  %lf \n",times*1000/getTickFrequency());
#endif
}

void TOTracker::error(void)
{
	printf("error\n");
}

