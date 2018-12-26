//
//  TOTracker.h
//  TOTracker
//
//  Created by Tiny Oh 2018.1.14
//  Copyright (c) 2018 White Sky. All rights reserved.
//

#ifndef TO_TRACKER_H
#define TO_TRACKER_H
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
#include "TOTimeStampObject.h"
#include "TOThreadBase.h"
#include "TOFaceType.h"
#include "TOTrackerTwistedPair.h"

#define  _DEBUG_TO_TRACKER false

namespace TO {
	class TOTrackerTwistedPair;
	class TOTracker:public TOStatusThreadBase
	{
	public:
		TOTracker(TOTrackerTwistedPair& pair);
		virtual ~TOTracker();
		cv::Rect getRes();
		bool isInNormalStatus();
		void setNeedSkipTrance(bool isNeed){m_isNeedSkipTrance = isNeed;};
		inline void setTrackerCheckReadyTag(bool is){m_trackerCheckReadyTag = is;};
		inline bool getTrackerCheckReadyTag(){return m_trackerCheckReadyTag;};
		inline void setTimeThreshold(unsigned int valueMutilPower, unsigned long cycleTime){m_selfAdaptTimeCycleThreshold  = valueMutilPower; m_trackerNormalKeepTime =cycleTime;};
	private:
		void wait(void);
		void dormant(void);
		void trance(void);
		void error(void);
		void work(void);
		inline bool isBigChangeHappen(){return m_tracker.getBigChangeOcour();};

		TOTrackerTwistedPair* m_father;
		virtual std::string rule(const std::string& last);
		cv::Rect m_TrackerRect;

		bool ProcessKCFTracker(KCFTracker& kcftracker, TO_QUEUE_FRAME& frame,cv::Rect& rect);
    	bool InitKCFTracker(KCFTracker& kcftracker,TO_QUEUE_FRAME& frame,const cv::Rect& rect);

    	KCFTracker m_tracker;
    	bool  m_trackerCheckReadyTag;

    	unsigned int m_selfAdaptTimeCycleThreshold;

    	long long m_trackerNormalKeepTime;//ms

    	double m_distanceSum;
    	cv::Rect m_tempRectNow;
    	bool m_checkTag;
    	bool m_isNeedSkipTrance;

	};
}

#endif