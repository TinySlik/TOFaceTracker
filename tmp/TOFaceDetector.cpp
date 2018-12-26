//
//  FaceDetector.cpp
//  FaceDetector
//
//  Created by Tiny Oh 2018.1.14
//  Copyright (c) 2018 White Sky. All rights reserved.
//

#include "TOFaceDetector.h"
#include "TOTracker.h"
#include "TOSkinFiltering.h"

using  namespace TO;

TOFaceDetector::TOFaceDetector(TOFrameManager & frameManager,string locationG):
m_frameManager(&frameManager),
m_FaceExit(false),
m_activeTrackerCount(0),
m_faceDetectCountEnableThreshold(6),
m_faceDetectCountEnableCount(0),
m_timeStampTag(0),
m_isNeedSkinFilter(false)
{
	m_skinFilter = new TOSkinFilter();
	m_calssName = typeid(*this).name();
	TO_THREAD_STATUS_UNIT units[] =
	{
		{"N2TO14TOFaceDetectorE&STATUS_DORMANT",	(void (TOStatusThreadBase::*)())(&TOFaceDetector::dormant)	},
		{"N2TO14TOFaceDetectorE&STATUS_DETECT",		(void (TOStatusThreadBase::*)())(&TOFaceDetector::detect)	},
		{"N2TO14TOFaceDetectorE&STATUS_ERROR",		(void (TOStatusThreadBase::*)())(&TOFaceDetector::error)	}
	};
	
	for (int i = 0; i < sizeof(units)/sizeof(TO_THREAD_STATUS_UNIT); ++i)
	{
		addStatusUnit(units[i]);
	}
	defineStatusRule(&TOStatusThreadBase::rule);

	initDetectors(locationG);

	m_faceCascade->setMaskGenerator(m_skinFilter);

	m_frameManager->addFlower(this);
}

void TOFaceDetector::afterStartThread()
{
	changeStatus("N2TO14TOFaceDetectorE&STATUS_DETECT",true);
}

void TOFaceDetector::setFaceDetectCountEnableThreshold(unsigned int num)
{
	m_faceDetectCountEnableThreshold  = num;
}

size_t TOFaceDetector::getActiveTrackerCount()
{
	return m_activeTrackerCount;
}

std::string TOFaceDetector::rule(const std::string& last)
{
	if(last == "N2TO14TOFaceDetectorE&STATUS_DETECT")
	{
		return "N2TO14TOFaceDetectorE&STATUS_DORMANT";
	}else if(last == "N2TO14TOFaceDetectorE&STATUS_DORMANT")
	{
		return "N2TO14TOFaceDetectorE&STATUS_DETECT";
	}else
	{
		return "N2TO14TOFaceDetectorE&STATUS_ERROR";
	}
}

TOFaceDetector::~TOFaceDetector()
{
	delete m_skinFilter;
	changeStatus("N2TO14TOFaceDetectorE&STATUS_DORMANT",true);
}

void TOFaceDetector::error(void)
{
	printf("[TOFaceDetector::error]error\n");
}

void TOFaceDetector::detect(void)
{
	EventManager *myEventManager = EventManager::Instance();
	m_frameManager->changeStatus("N2TO14TOFrameManagerE&STATUS_KEEP",true);

	bool isDetectOver =  false;
	unsigned short detectorTimesSum = 0;
	std::vector<cv::Rect> tempFaceArry;
	bool allLastTimeReResaultMatched = false;
	std::vector<FRAME_STRATEGY_FILTRATION_UNIT>filtrationArry;
	std::vector<FRAME_STRATEGY_TRACKER_UNIT> twPairsArray;
	std::vector<FRAME_STRATEGY_FACE_UNIT> faceFindAray;
	
	getTrackerValidArray(twPairsArray,m_trackerTwistedPairs);

	while((!m_threadExitTag) && detectorTimesSum < m_faceDetectCountEnableThreshold)
	{
#if _DEBUG_TO_FACE_DETECTOR
		cout << "[TOFaceDetector::detect] detector while start exec"  << endl;
#endif
		TO_QUEUE_FRAME* target  = NULL;

		while(!m_threadExitTag)
		{
			target = m_frameManager -> getLastFrameInUse();
			if(target)
			{
				break;
			}else
			{
				usleep(m_threadResponseInterval);
			}
		};

		if(m_threadExitTag || NULL == target)
		{
			m_FaceRectArray.clear();
			filtrationArry.clear();
			twPairsArray.clear();
			break;
		}
		
		if(m_timeStampTag == target -> timeStamp)
		{
			m_frameManager->releaseCurrentFrameInUse(target);
			usleep(m_threadResponseInterval*10);
			continue;
		}
		m_timeStampTag = target -> timeStamp;

		faceFindAray.clear();

		target->isUniqueKeyFrame = true;

		//usleep(100000);  //test slow case
		if(ProcessFaceDetect(*target,faceFindAray))
		{
			if(m_threadExitTag)
			{
				continue;
			}
			m_FaceExit = true;
			getTrackerValidArray(twPairsArray,m_trackerTwistedPairs);
			allLastTimeReResaultMatched = updateStrategyFiltrationArray(twPairsArray,filtrationArry,faceFindAray);
		}else
		{
			if(m_threadExitTag)
			{
				continue;
			}
			target->isUniqueKeyFrame = false;
		}
		
		m_frameManager->releaseCurrentFrameInUse(target);

		detectorTimesSum ++;
		if(allLastTimeReResaultMatched && detectorTimesSum >= m_faceDetectCountEnableThreshold/2)
		{
#if _DEBUG_TO_FACE_DETECTOR
			cout << "all match the old resault " << filtrationArry.size() <<  "faces have been found" << ",count times >=" << m_faceDetectCountEnableThreshold/2 << endl;
#else
			cout << "no new face found.\n" << filtrationArry.size() <<  " faces as last time.\n" <<  "\"m_faceDetectCountEnableThreshold/2\"  is " << m_faceDetectCountEnableThreshold/2 << " is been limitted to break out the while." <<  endl ;
#endif
			break;
		}
	}
	if(m_threadExitTag)
	{
		return;
	}
#if	_DEBUG_TO_FACE_DETECTOR
	printFiltrationArry(filtrationArry);
#endif

	if(0 >= filtrationArry.size())
	{
		m_FaceExit = false;
#if _DEBUG_TO_FACE_DETECTOR
		cout << "no face is been found at this time" << endl;
#endif
	}else
	{
		nextStatus();
	}

	m_FaceRectArray.clear();
	m_activeTrackerCount =  0;

	std::vector<TOTrackerTwistedPair*>  tempTW =  m_trackerTwistedPairs;

	for (int i = 0; i < filtrationArry.size(); ++i)
	{
		if((filtrationArry[i].targetFrame) && NULL == filtrationArry[i].twPair)
		{
			m_FaceRectArray.push_back(filtrationArry[i].newRes);
			for (int j = 0; j < tempTW.size(); ++j)
			{
				if(tempTW[j]->isDormant())
				{
					tempTW[j]->activeUnit(filtrationArry[i].newRes,filtrationArry[i].targetFrame);
					tempTW.erase(tempTW.begin()+j);
					filtrationArry.erase(filtrationArry.begin()+i);
					m_activeTrackerCount++;
					--i;
					break;
				}
			}
		}else if((NULL == filtrationArry[i].targetFrame) && filtrationArry[i].twPair)
		{
			filtrationArry[i].twPair->dormantUnit();
		}else if((filtrationArry[i].targetFrame) && filtrationArry[i].twPair)
		{
			m_FaceRectArray.push_back(filtrationArry[i].newRes);
			m_activeTrackerCount++;
			filtrationArry[i].twPair->startToTrance(filtrationArry[i].newRes,filtrationArry[i].targetFrame);
		}else
		{
			cout  << "error !!!" <<  endl;
		}
	}

	if(!m_threadExitTag)
	{
		cout << "[TOFaceDetector::detect] realTimeComputing exec"  << endl;
		m_frameManager->realTimeComputing();
	}
}

void TOFaceDetector::printFiltrationArry(const std::vector<FRAME_STRATEGY_FILTRATION_UNIT>& filtrationArry)
{
	for (int i = 0; i < filtrationArry.size(); ++i)
	{
		cout  << "filtrationArry[" << i <<"]  "<< "targetFrame" << filtrationArry[i].targetFrame << "twPair" << filtrationArry[i].twPair << "oldRes" << filtrationArry[i].oldRes << "newRes"  << filtrationArry[i].newRes << endl;
	}
}

bool TOFaceDetector::updateStrategyFiltrationArray(
	std::vector<FRAME_STRATEGY_TRACKER_UNIT>& twArray,
	std::vector<FRAME_STRATEGY_FILTRATION_UNIT>& stArray,
	std::vector<FRAME_STRATEGY_FACE_UNIT>& face)
{
	if(0 >= face.size())
	{
		cout  << "no face found" << endl;
		return false;
	}

	std::vector<FRAME_STRATEGY_TRACKER_UNIT> twArrayCp = twArray;
	std::vector<FRAME_STRATEGY_FILTRATION_UNIT>& stArrayCp = stArray;
	std::vector<FRAME_STRATEGY_FACE_UNIT> faceCp = face;

	if(0 >= twArrayCp.size())
	{
#if _DEBUG_TO_FACE_DETECTOR
		if(0 >= stArrayCp.size())
		{
			cout << "[FRAME_STRATEGY_FILTRATION_UNIT] in thread init" << endl;
		}else
		{
			cout << "[FRAME_STRATEGY_FILTRATION_UNIT] in thread init after update" << endl;
		}
#endif
		bool isSm = similarUpdate(stArrayCp,faceCp);
		return true;
	}else
	{
		if(0 >= stArrayCp.size())
		{
#if _DEBUG_TO_FACE_DETECTOR
			cout << "[FRAME_STRATEGY_FILTRATION_UNIT]in local init" << endl;
#endif
			bool isSm = similarUpdate(stArrayCp,twArrayCp);
			isSm = similarUpdate(stArrayCp,faceCp);
		}else
		{
#if _DEBUG_TO_FACE_DETECTOR
			cout << "[FRAME_STRATEGY_FILTRATION_UNIT]in local init after update" << endl;
#endif
			bool isSm = similarUpdate(stArrayCp,faceCp);
		}
	}

	stArray = stArrayCp ;
	int countOfMatchedTKFrame =  0;
	for (int i = 0; i < stArrayCp.size(); ++i)
	{
		if(stArrayCp[i].targetFrame &&  stArrayCp[i].twPair)
		{
			countOfMatchedTKFrame++;
		}
	}

	if(countOfMatchedTKFrame >= twArray.size())
	{
		return true;
	}
	return false;
}

bool TOFaceDetector::similarUpdate(std::vector<FRAME_STRATEGY_FILTRATION_UNIT>& targetArray,std::vector<FRAME_STRATEGY_TRACKER_UNIT> materialArray)
{
	int count  =  0;
	for (int j = 0; j < targetArray.size(); ++j)
	{
		for (int i = 0; i < materialArray.size(); ++i)
		{
			if(isSimilarTo(materialArray[i].res,targetArray[j].newRes))
			{
				if(targetArray[i].targetFrame)
				{
					targetArray[j].twPair = materialArray[i].twPair;
				}else
				{
					targetArray[j].oldRes = targetArray[j].newRes;
					targetArray[j].newRes = materialArray[i].res;
					targetArray[j].twPair = materialArray[i].twPair;
				}
				materialArray.erase(materialArray.begin()+i);

				count++;
			    break;
			}
		}
	}
	if(0 >= materialArray.size()  && count == targetArray.size())
	{
		return true;
	}
	for (int i = 0; i < materialArray.size(); ++i)
	{
		FRAME_STRATEGY_FILTRATION_UNIT unit;
		unit.oldRes = Rect0;
		unit.newRes = materialArray[i].res;
		unit.targetFrame = NULL;
		unit.twPair = materialArray[i].twPair;
		targetArray.push_back(unit);
	}
	return false;
}

bool TOFaceDetector::similarUpdate(std::vector<FRAME_STRATEGY_FILTRATION_UNIT>& targetArray,std::vector<FRAME_STRATEGY_FACE_UNIT> materialArray)
{
	int count  =  0;
	for (int j = 0; j < targetArray.size(); ++j)
	{
		for (int i = 0; i < materialArray.size(); ++i)
		{
			if(isSimilarTo(materialArray[i].res,targetArray[j].newRes,RECT_FACE_UPDATE_SIMAR_PTH))
			{
				targetArray[j].oldRes = targetArray[j].newRes;
				targetArray[j].newRes = materialArray[i].res;
				if(targetArray[j].targetFrame)
				{
					targetArray[j].targetFrame ->isUniqueKeyFrame =  false;
				}
				targetArray[j].targetFrame = materialArray[i].targetFrame;
				materialArray.erase(materialArray.begin()+i);

				count++;
			    break;
			}
		}
	}

	if(0 >= materialArray.size() && count == targetArray.size())
	{
		//cout  << "similarUpdate fully complete" << endl;
		return true;
	}
#if	_DEBUG_TO_FACE_DETECTOR
	cout << "similarUpdate ep complete with" << materialArray.size() << "more" << endl;
#endif
	for (int i = 0; i < materialArray.size(); ++i)
	{
		FRAME_STRATEGY_FILTRATION_UNIT unit;
		unit.oldRes = Rect0;
		unit.newRes = materialArray[i].res;
		unit.targetFrame = materialArray[i].targetFrame;
		unit.twPair = NULL;
		targetArray.push_back(unit);
	}

	return false;
}

void TOFaceDetector::getTrackerValidArray(std::vector<FRAME_STRATEGY_TRACKER_UNIT>& twArray,std::vector<TOTrackerTwistedPair*>& twArrayI)
{
	twArray.clear();
	TO_QUEUE_FRAME*  target = m_frameManager->getLastFrameInUse();
	for (int i = 0; i <  twArrayI.size() ; i++)
	{
		cv::Rect curTar = Rect0;
		twArrayI[i]->getMainTrackerResault(curTar);
		if(Rect0 == curTar)
		{
			continue;
		}
		FRAME_STRATEGY_TRACKER_UNIT  unit;
		unit.twPair = twArrayI[i];
		unit.res = curTar;

		twArray.push_back(unit);
	}
	m_frameManager->releaseCurrentFrameInUse(target);
}

bool TOFaceDetector::isSimilarTo(cv::Rect& g1,cv::Rect& g2,float level)
{
	Point c1 = Point(g1.x+g1.width/2,g1.y+g1.height/2);
	Point c2 = Point(g2.x+g2.width/2,g2.y+g2.height/2);

	double x1  = c1.x;
	double x2  = c2.x;
	double y1  = c1.y;
	double y2  = c2.y;

	double w1  = g1.width;
	double w2  = g2.width;
	double h1  = g1.height;
	double h2  = g2.height;

	double dist = sqrt ( pow((x1-x2), 2) + pow((y1-y2), 2) );
	
	double widthAVG  =  (w1+w2)/2;
	double heightAVG  =  (h1+h2)/2;
	double whAVG =  (widthAVG+heightAVG)/2;

	return dist  < whAVG/level;
}

void TOFaceDetector::dormant(void)
{
	usleep(m_threadResponseInterval);
}

void TOFaceDetector::addFolower(TOTrackerTwistedPair* tk)
{
	m_trackerTwistedPairs.push_back(tk);
}

bool TOFaceDetector::getResault(std::vector<cv::Rect>& vv)
{
	if(m_FaceExit)
	{
		vv = m_FaceRectArray;
		return true;
	}
	return false;
}

int TOFaceDetector::initDetectors(const std::string&  faceCascadeFilename)
{
	try 
    {   
    	m_faceCascade = new CascadeClassifier();
        m_faceCascade->load(faceCascadeFilename);
    } 
    catch (cv::Exception &e) 
    { }
    if ( m_faceCascade->empty() ) 
    {
        cerr << "ERROR:[initDetectors] Cascade classifier carrying face detection[" << faceCascadeFilename << "]failed!" << endl;
        return -1;
    }
    cout << "[initDetectors] Cascade classifier carrying face detection[" << faceCascadeFilename << "]success!" << endl;

    return 0;
}

bool TOFaceDetector::ProcessFaceDetect(TO_QUEUE_FRAME& frame,std::vector<cv::Rect>& res )
{
    cv::Mat* in   = frame.data;
#if _TIME_DEBUG
    double times = (double)getTickCount();
#endif
    if (NULL == in)
    {
        return false;
    }
    cv::Mat* srcData = in;
    if(m_isNeedSkinFilter)
    {
    	*srcData = YCrCb_Otsu_detect(*srcData);
    }
    //frame.refCount++;
    bool isOK = getPreprocessedFaceMutil(*srcData, *m_faceCascade,res);
    //frame.refCount--;
    if(isOK)
    {
#if _TIME_DEBUG
        times = (double)getTickCount() - times;
        printf( "\n[ProcessFaceDetect][TIME]ProcessFaceDetect face sum : %d \n success times cust frame time  %lf \n",res.size(),times*1000/getTickFrequency());
#endif
        return true;
    }

#if _TIME_DEBUG
    times = (double)getTickCount() - times;
    printf( "\n[ProcessFaceDetect][TIME]ProcessFaceDetect faild times cust frame time  %lf \n",times*1000/getTickFrequency());
#endif
    return false;
}

bool TOFaceDetector::ProcessFaceDetect(TO_QUEUE_FRAME& frame,std::vector<FRAME_STRATEGY_FACE_UNIT>& res )
{
	std::vector<cv::Rect> vv;
	ProcessFaceDetect(frame,vv);
	for (int i = 0; i < vv.size(); ++i)
	{
		FRAME_STRATEGY_FACE_UNIT unit;
		unit.res  =  vv[i];
		unit.targetFrame = &frame;
		res.push_back(unit);
	}
	if(vv.size() <= 0)
	{
		return false;
	}else
	{
		return true;
	}
}