//
//  TOTrackerTwistedPair.h
//  TOTrackerTwistedPair
//
//  Created by Tiny Oh 2018.1.14
//  Copyright (c) 2018 White Sky. All rights reserved.
//

#ifndef TO_TRACKER_TWISTED_H
#define TO_TRACKER_TWISTED_H

#include "TOTimeStampObject.h"
#include "TOThreadBase.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include "TOFaceType.h"

#define _DEBUG_TO_TRACKER_TWISTED_PAIR false

namespace TO {
	class TOTracker;
	class TOFrameManager;
	class TOFaceDetector;
	class TOTrackerTwistedPair :public TOTimeStampObject
	{
	public:
		TOTrackerTwistedPair(TOFrameManager & frameManager,TOFaceDetector & detentor);
		~TOTrackerTwistedPair();

		TOTracker* toggleTwistedMain();

		TO_QUEUE_FRAME* getCurrentFrameInUse();
		TO_QUEUE_FRAME* getLastFrameInUse();
		void releaseCurrentFrameInUse(TO_QUEUE_FRAME* target);
		size_t getQueueSize();

		bool getResault(std::vector<cv::Rect>& arry);
		bool getResault(cv::Rect  & rg);

		bool getMainTrackerResault(cv::Rect  & rg);
		bool getFitResault(cv::Rect& rg,float = 0.3f);
		void noticeDetectorWork();
		void dormantMinorTracker();
		void dormantUnit(bool isForce =false);

		void activeUnit(cv::Rect& face);

		void activeUnit(cv::Rect& face,TO_QUEUE_FRAME* target);

		void startToTrance(const cv::Rect& tg);

		void startToTrance(const cv::Rect& tg,TO_QUEUE_FRAME* target);

		void addTrackerThreadCount(void);

		void check(TO_RECT* face,const unsigned int blockWaitTime = 50 ,uchar* pBGRA = NULL);

		static void checkConcurrence(std::vector<TO_RECT> & vv,const unsigned int blockWaitTime = 50 ,uchar* pBGRA = NULL);
		static void setCheckConfig(const unsigned int blockWaitTime);
		static std::vector<TOTrackerTwistedPair*>m_objs;
		static unsigned int  m_minimumRefreshDetectionInterval;
		static unsigned int  m_blockWaitTime;

		bool isDormant();
		bool isNormal();

		void setChildrenTimeThreshold(unsigned int valueMutilPower, unsigned long cycleTime);

		void setChildrenSkipTrance(bool isNeed);
	private:
		void debugRes(uchar* pBGRA,cv::Mat* in,cv::Rect face );
		TO::TOTracker* m_TOtrackerA;
		TO::TOTracker* m_TOtrackerB;

		TOFrameManager* m_frameManager;

		TOFaceDetector* m_detector;

		TOTracker* m_mainTOtracker;
		TOTracker* m_minorTOtracker;

		cv::Rect m_localTargetFace;

		cv::Rect m_outPutFace;
		cv::Rect m_lastPutFace;

		std::vector<TO_RECT> m_rects;

	 	static unsigned long m_urealOneFrameTime;

	 	static double frameTimeQueue[10];

		bool m_checkTag;
	};
}
#endif