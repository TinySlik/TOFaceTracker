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
#include "TOTrackerTwistedPair.h"
#include "TOSkinFiltering.h"


//#define FACE_CASCADE_FILENAME_DEFAULT "../cv_resource/haarcascade_frontalface_alt.xml" //path*
#define FACE_CASCADE_FILENAME_DEFAULT "../cv_resource/lbpcascade_frontalface.xml" //path*

#define EYE1_CASCADE_FILENAME_DEFAULT "../cv_resource/haarcascade_eye.xml"  //path*
#define EYE2_CASCADE_FILENAME_DEFAULT "../cv_resource/haarcascade_eye_tree_eyeglasses.xml" //path*

#define  _DEBUG_TO_FACE_DETECTOR false

#define RECT_FACE_UPDATE_SIMAR_PTH 2.0

namespace TO {

	typedef struct{
	TO_QUEUE_FRAME* targetFrame;
	TO::TOTrackerTwistedPair* twPair;
	cv::Rect oldRes;
	cv::Rect newRes;
	}FRAME_STRATEGY_FILTRATION_UNIT;

	typedef struct{
		TO_QUEUE_FRAME* targetFrame;
		cv::Rect res;
	}FRAME_STRATEGY_FACE_UNIT;

	typedef struct{
		TO::TOTrackerTwistedPair* twPair;
		cv::Rect res;
	}FRAME_STRATEGY_TRACKER_UNIT;

	class TOFrameManager;
	class TOFaceDetector:public  TOStatusThreadBase
	{
	public:
		TOFaceDetector(TOFrameManager & frameManager,string faceDefaultCascadeLocation);
		virtual ~TOFaceDetector();
		bool getResault(std::vector<cv::Rect>& vv);

		void addFolower(TOTrackerTwistedPair* tk);

		bool isSimilarTo(cv::Rect& g1,cv::Rect& g2,float level = 4.0);
		size_t getActiveTrackerCount();
		inline bool isFaceExit(){return m_FaceExit;};
		void setFaceDetectCountEnableThreshold(unsigned int num);
		void setFilterEnable(bool is){m_isNeedSkinFilter = is;};

	protected:
		virtual void afterStartThread();
	private:
		void printFiltrationArry(const std::vector<FRAME_STRATEGY_FILTRATION_UNIT>& filtrationArry);

		bool updateStrategyFiltrationArray(std::vector<FRAME_STRATEGY_TRACKER_UNIT>& twArray,std::vector<FRAME_STRATEGY_FILTRATION_UNIT>& stArray,std::vector<FRAME_STRATEGY_FACE_UNIT>& face);
		void getTrackerValidArray(std::vector<FRAME_STRATEGY_TRACKER_UNIT>& twArray,std::vector<TOTrackerTwistedPair*>& twArrayI);

		int initDetectors(const std::string& name);
		bool ProcessFaceDetect(TO_QUEUE_FRAME& frame,std::vector<cv::Rect>& res );
		bool ProcessFaceDetect(TO_QUEUE_FRAME& frame,std::vector<FRAME_STRATEGY_FACE_UNIT>& res );
		void detect(void);
		void dormant(void);
		void error(void);

		bool similarUpdate(std::vector<FRAME_STRATEGY_FILTRATION_UNIT>& targetArray,std::vector<FRAME_STRATEGY_TRACKER_UNIT> materialArray);
		bool similarUpdate(std::vector<FRAME_STRATEGY_FILTRATION_UNIT>& targetArray,std::vector<FRAME_STRATEGY_FACE_UNIT> materialArray);

		cv::CascadeClassifier* m_faceCascade;

		TOFrameManager* m_frameManager;

		virtual std::string rule(const std::string& last);

		std::vector<cv::Rect> m_FaceRectArray;

		bool m_FaceExit;

		std::vector<TOTrackerTwistedPair*> m_trackerTwistedPairs;

		size_t m_activeTrackerCount   ;
		int m_faceDetectCountEnableThreshold;
		int m_faceDetectCountEnableCount;
		long long m_timeStampTag;
		bool m_isNeedSkinFilter;

		TOSkinFilter* m_skinFilter;
	};
}


#endif