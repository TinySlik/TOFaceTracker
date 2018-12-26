//
//  TOTrackerTwistedPair.h
//  TOTrackerTwistedPair
//
//  Created by Tiny Oh 2018.1.14
//  Copyright (c) 2018 White Sky. All rights reserved.
//
#include "TOTrackerTwistedPair.h"
#include "TOFrameManager.h"
#include "TOFaceDetector.h"
#include "TOTracker.h"
using namespace TO;

std::vector<TOTrackerTwistedPair*>TOTrackerTwistedPair::m_objs;
unsigned int  TOTrackerTwistedPair::m_minimumRefreshDetectionInterval = 200;
unsigned int  TOTrackerTwistedPair::m_blockWaitTime  = 50;
unsigned  long TOTrackerTwistedPair::m_urealOneFrameTime=0;

double TOTrackerTwistedPair::frameTimeQueue[10]  = {0};

TOTrackerTwistedPair::TOTrackerTwistedPair(TOFrameManager & frameManager,TOFaceDetector & detentor):m_frameManager(&frameManager),m_detector(&detentor),m_checkTag(false)
{
	m_outPutFace = Rect0;
	m_lastPutFace = Rect0;
	m_frameManager->addFlower(this);
	m_detector->addFolower(this);
	
	m_TOtrackerA = new TOTracker(*this);
	m_TOtrackerB = new TOTracker(*this);
	m_mainTOtracker  = m_TOtrackerB;
	m_minorTOtracker = m_TOtrackerA;
	m_objs.push_back(this);
	m_mainTOtracker->changeStatus("N2TO9TOTrackerE&STATUS_DORMANT",true);
}

TOTrackerTwistedPair::~TOTrackerTwistedPair()
{
	delete m_TOtrackerA;
	delete m_TOtrackerB;
	for (int i = 0; i < m_objs.size(); ++i)
	{
		if(m_objs[i]  ==this)
		{
			m_objs.erase(m_objs.begin()+i);
			break;
		}
	}
}

void TOTrackerTwistedPair::setChildrenTimeThreshold(unsigned int valueMutilPower, unsigned long cycleTime)
{
	m_mainTOtracker->setTimeThreshold(valueMutilPower,cycleTime);
	m_minorTOtracker->setTimeThreshold(valueMutilPower,cycleTime);
}

void TOTrackerTwistedPair::setChildrenSkipTrance(bool isNeed)
{
	m_mainTOtracker->setNeedSkipTrance(isNeed);
	m_minorTOtracker->setNeedSkipTrance(isNeed);
}

TOTracker* TOTrackerTwistedPair::toggleTwistedMain()
{
	if(m_mainTOtracker  ==  m_TOtrackerA)
	{
		m_mainTOtracker = m_TOtrackerB;
		m_minorTOtracker = m_TOtrackerA;
	}else if(m_TOtrackerB == m_mainTOtracker)
	{
		m_mainTOtracker = m_TOtrackerA;
		m_minorTOtracker = m_TOtrackerB;
	}else
	{
		cerr << "[TOTrackerTwistedPair::toggleTwistedMain]error" << endl;
		m_mainTOtracker  = NULL;
		m_minorTOtracker = NULL;
	}
	return m_mainTOtracker;
}

void TOTrackerTwistedPair::startToTrance(const cv::Rect& tg)
{
	m_localTargetFace = tg;
#if  _DEBUG_TO_TRACKER_TWISTED_PAIR
	cout <<  m_mainTOtracker->getCurrentStatus() << endl;
	cout <<  m_minorTOtracker->getCurrentStatus() << endl;
	cout  << m_localTargetFace << endl;
#endif
	m_minorTOtracker->changeStatus("N2TO9TOTrackerE&STATUS_TRANCE",false);
}

void TOTrackerTwistedPair::startToTrance(const cv::Rect& tg,TO_QUEUE_FRAME* target)
{
	setCurrentTime(target->timeStamp-1);
	startToTrance(tg);
}

void TOTrackerTwistedPair::dormantMinorTracker()
{
    addTrackerThreadCount();
	toggleTwistedMain();
	cv::Rect ll = m_minorTOtracker->getRes();
	if(ll == Rect0)
	{
		
	}else
	{
		m_outPutFace = ll;
	}
	
	m_minorTOtracker->changeStatus("N2TO9TOTrackerE&STATUS_WAIT");
}

void TOTrackerTwistedPair::addTrackerThreadCount(void)
{
	m_frameManager->addTrackerThreadCount();
}

TO_QUEUE_FRAME* TOTrackerTwistedPair::getCurrentFrameInUse()
{
	TO_QUEUE_FRAME*  target =  m_frameManager->getCurrentFrameInUse(this);
	if(NULL == target)
    {
    	target = m_frameManager->getCurrentFrameInUse();
    	this ->setCurrentTime(target->timeStamp -  1);
    }
    return target;
}

void TOTrackerTwistedPair::dormantUnit(bool isForce)
{
	if(isForce=true)
	{
		m_mainTOtracker->changeStatus("N2TO9TOTrackerE&STATUS_DORMANT",true);
		m_minorTOtracker->changeStatus("N2TO9TOTrackerE&STATUS_DORMANT",true);
		return;
	}
	if(isDormant())
	{
#if  _DEBUG_TO_TRACKER_TWISTED_PAIR
		cout << "already  in dormant  unit status"  << endl;
#endif
	}else
	{
		m_mainTOtracker->changeStatus("N2TO9TOTrackerE&STATUS_DORMANT",true);
		m_minorTOtracker->changeStatus("N2TO9TOTrackerE&STATUS_WAIT");
	}
}

void TOTrackerTwistedPair::activeUnit(cv::Rect& face)
{
	if(isDormant())
	{
		m_localTargetFace = face;
		m_mainTOtracker->changeStatus("N2TO9TOTrackerE&STATUS_WAIT");
		m_minorTOtracker->changeStatus("N2TO9TOTrackerE&STATUS_TRANCE");
	}else
	{
		cout << "not in dormant unit status"  << endl;
	}
}

void TOTrackerTwistedPair::activeUnit(cv::Rect& face,TO_QUEUE_FRAME* target)
{
	setCurrentTime(target->timeStamp-1);
	activeUnit(face);
}

TO_QUEUE_FRAME* TOTrackerTwistedPair::getLastFrameInUse()
{
	TO_QUEUE_FRAME*  target =  m_frameManager->getLastFrameInUse();
	return target;
}

void TOTrackerTwistedPair::releaseCurrentFrameInUse(TO_QUEUE_FRAME* target)
{
	m_frameManager->releaseCurrentFrameInUse(target,this);
}

size_t TOTrackerTwistedPair::getQueueSize()
{
	return m_frameManager->getQueueSize(this);
}

bool TOTrackerTwistedPair::getResault(std::vector<cv::Rect>& arry)
{
	return m_detector->getResault(arry);
}

bool TOTrackerTwistedPair::getResault(cv::Rect  & rg)
{
	rg  = m_localTargetFace;
	return true;
}

bool TOTrackerTwistedPair::getMainTrackerResault(cv::Rect  & rg)
{
	rg  = m_mainTOtracker->getRes();
	return true;
}

bool TOTrackerTwistedPair::getFitResault(cv::Rect& rg , float level)
{
	cv::Rect tmp = m_mainTOtracker->getRes();
	if(tmp == Rect0)
	{
		return false;
	}

	if(m_lastPutFace ==  m_outPutFace)
	{
		rg  =  m_outPutFace;
		m_lastPutFace = tmp;
		return true;
	}else
	{
		int oldRight =  m_outPutFace.x + m_outPutFace.width;
		int oldTop	= m_outPutFace.y + m_outPutFace.height;

		if(abs(m_outPutFace.x-tmp.x) < 4)
		{
			m_outPutFace.x = tmp.x;
		}else
		{
			m_outPutFace.x = (tmp.x - m_outPutFace.x)* level+ m_outPutFace.x;
		}
		
		if(abs(m_outPutFace.y-tmp.y) < 4)
		{
			m_outPutFace.y=tmp.y;
		}else
		{
			m_outPutFace.y = (tmp.y - m_outPutFace.y)* level+ m_outPutFace.y;
		}

		if(abs(tmp.x+tmp.width-oldRight) < 4)
		{
			m_outPutFace.width = tmp.x+tmp.width - m_outPutFace.x;
		}else
		{
			m_outPutFace.width = ((tmp.x+tmp.width) - oldRight)* level+ (oldRight) - m_outPutFace.x;
		}

		if(abs(tmp.y+tmp.height-oldTop) < 4)
		{
			m_outPutFace.height = tmp.y+tmp.height - m_outPutFace.y;
		}else
		{
			m_outPutFace.height = ((tmp.y+tmp.height) - oldTop)* level+ (oldTop) - m_outPutFace.y;
		}
		rg = m_outPutFace;
		m_lastPutFace = tmp;
		return false;
	}
}

void TOTrackerTwistedPair::check(TO_RECT* faceInTO,const unsigned int blockWaitTime ,uchar* pBGRA)
{
	Rect face = Rect0;

    std::vector<cv::Rect> vv;
    if(NULL == faceInTO)
    {
    	return;
    }

    if(!getResault(vv))
    {
#if _DEBUG_TO_TRACKER_TWISTED_PAIR
        printf("[TOFaceTracker check]no face found\n");
#endif
        return ;
    }
    static long long timeCounter ;

    m_mainTOtracker->setTrackerCheckReadyTag(false);
    timeCounter = 0;
    while(!m_mainTOtracker->getTrackerCheckReadyTag())
    {
        usleep(1000);
        timeCounter++;
        if(timeCounter > blockWaitTime)
        {
            break;
        }
    }

    face = m_mainTOtracker->getRes();

	faceInTO->x = face.x;
	faceInTO->y = face.y;
	faceInTO->width =  face.width;
	faceInTO->height = face.height;

    if(pBGRA)
    {
    	TO_QUEUE_FRAME*  target = m_frameManager->getLastFrameInUse();
        debugRes(pBGRA,target->data,face);
        m_frameManager->releaseCurrentFrameInUse(target);
    }
}

bool TOTrackerTwistedPair::isDormant()
{
	if (m_mainTOtracker->getCurrentStatus() ==  "N2TO9TOTrackerE&STATUS_DORMANT" || m_mainTOtracker->getCurrentStatus() ==  "N2TO9TOTrackerE&STATUS_WAIT" )
	{
		if(m_minorTOtracker->getCurrentStatus() ==  "N2TO9TOTrackerE&STATUS_DORMANT" || m_minorTOtracker->getCurrentStatus() ==  "N2TO9TOTrackerE&STATUS_WAIT")
		{
			return  true;
		}
	}
	return  false;
}

bool TOTrackerTwistedPair::isNormal()
{
	return  m_mainTOtracker->getCurrentStatus() ==  "N2TO9TOTrackerE&STATUS_WORK";
}

void TOTrackerTwistedPair::checkConcurrence(std::vector<TO_RECT> & vv,const unsigned int blockWaitTime,uchar* pBGRA)
{
	if(m_objs.size() <=  0)
	{
		cout  << "no TOTrackerTwistedPair object in  memeory"  <<  endl;
		return;
	}

	if(!(m_objs[0]-> m_detector->isFaceExit()))
	{
#if _DEBUG_TO_TRACKER_TWISTED_PAIR
        printf("[TOFaceTracker check]no face found\n");
#endif
		return;
	}
	std::vector<cv::Rect> vvF;
	if(!m_objs[0] ->getResault(vvF))
    {
#if _DEBUG_TO_TRACKER_TWISTED_PAIR
        printf("[TOFaceTracker check]no face found\n");
#endif
        return ;
    }

    vector <int> numMe;
    for (int i = 0; i < m_objs.size(); ++i)
    {
    	if(m_objs[i]->isNormal())
    	{
    		numMe.push_back(i);
    		m_objs[i]->m_mainTOtracker->setTrackerCheckReadyTag(false);
    	}
    }

    long long timeCounter = 0;
    while(true)
    {
    	unsigned int completeCount  = 0;
    	for (int i = 0; i < numMe.size(); ++i)
    	{
    		if(!m_objs[numMe[i]]->m_mainTOtracker->getTrackerCheckReadyTag())
    		{
				completeCount++;
    		}
    	}

    	if(completeCount >= numMe.size())
    	{
    		break;
    	}

        usleep(1000);
        timeCounter++;
        if(timeCounter > blockWaitTime)
        {
            break;
        }
    }

    for (int i = 0; i < numMe.size(); ++i)
    {
    	cv::Rect face =Rect0;
    	face = m_objs[numMe[i]]->m_mainTOtracker->getRes();

    	TO_RECT faceInTO = {face.x,face.y,face.width,face.height};
    	vv.push_back(faceInTO);
    }
    std::vector<TO_RECT>  vv2;

    for (int i = 0; i < numMe.size(); ++i)
    {
    	cv::Rect faceFit =Rect0;
    	m_objs[numMe[i]]->getFitResault(faceFit);
    	TO_RECT faceInTO = {faceFit.x,faceFit.y,faceFit.width,faceFit.height};
    	vv2.push_back(faceInTO);
    }


    if(numMe.size()>0 && pBGRA)
	{
		std::vector<cv::Rect> cvRects;

		std::vector<cv::Rect> cvFitRects;

		for (int i = 0; i < vv.size(); ++i)
		{
			cv::Rect rt =  Rect0;
			rt.x = vv[i].x;
			rt.y = vv[i].y;
			rt.width = vv[i].width;
			rt.height = vv[i].height;
			cvRects.push_back(rt);
		}

		for (int i = 0; i < vv2.size(); ++i)
		{
			cv::Rect rt =  Rect0;
			rt.x = vv2[i].x;
			rt.y = vv2[i].y;
			rt.width = vv2[i].width;
			rt.height = vv2[i].height;
			cvFitRects.push_back(rt);
		}

		TO_QUEUE_FRAME* target = m_objs[0] ->m_frameManager->getLastFrameInUse();
		std::vector<cv::Rect> vvF;
	    if(m_objs[0] ->getResault(vvF))
	    {
	        for (int i = 0; i < vvF.size(); ++i)
		    {
		        rectangle(*(target->data), vvF[i], CV_RGB(255,0, 0), 2, CV_AA);
		    }
	    }

	    // //m_objs[0] ->getFitResault();
	    // for (int i = 0; i < cvFitRects.size(); ++i)
	    // {
	    // 	rectangle(*(target->data), cvFitRects[i], CV_RGB(0,255, 0), 2, CV_AA);
	    // }

	    for (int i = 0;i < cvRects.size(); ++i)
	    {
	    	rectangle(*(target->data), cvRects[i], CV_RGB(255,255, 0), 2, CV_AA);
	    }


	    uchar* outData=target->data->ptr<uchar>(0);
	    for(int i=0;i< target->data->rows;i++)
	    {
	        for(int j=0;j< target->data->cols;j++)
	        {
	            *pBGRA++= *outData++;
	            *pBGRA++= *outData++;
	            *pBGRA++= *outData++;
	            *pBGRA++= 255;
	        }
	    }
	    m_objs[0] ->m_frameManager->releaseCurrentFrameInUse(target);
	}
}

void TOTrackerTwistedPair::setCheckConfig(const unsigned int blockWaitTime)
{
	m_blockWaitTime =blockWaitTime;
}

void TOTrackerTwistedPair::debugRes(uchar* pBGRA,cv::Mat* in,cv::Rect face )
{
	std::vector<cv::Rect> vv;

    if(getResault(vv))
    {
        for (int i = 0; i < vv.size(); ++i)
	    {
	        rectangle(*in, vv[i], CV_RGB(255,0, 0), 2, CV_AA);
	    }
    }

    rectangle(*in, face, CV_RGB(255,255, 0), 2, CV_AA);
        
    uchar* outData=in->ptr<uchar>(0);
    for(int i=0;i< in->rows;i++)
    {
        for(int j=0;j< in->cols;j++)
        {
            *pBGRA++= *outData++;
            *pBGRA++= *outData++;
            *pBGRA++= *outData++;
            *pBGRA++= 255;
        }
    }
}

void TOTrackerTwistedPair::noticeDetectorWork()
{
	if(m_detector->getCurrentStatus() == "N2TO14TOFaceDetectorE&STATUS_DORMANT")
	{
		m_detector->changeStatus("N2TO14TOFaceDetectorE&STATUS_DETECT");
	}
}
