//
//  TOTimeStampObject.h
//  TOTimeStampObject
//
//  Created by Tiny Oh 2018.1.14
//  Copyright (c) 2018 White Sky. All rights reserved.
//

#ifndef TO_TIME_STAMP_OBJECT_H
#define TO_TIME_STAMP_OBJECT_H
#include "TOEventManager.h"

#define _DEBUG_TO_TIME_STAMP_OBJECT_BASE false

namespace TO {
	class TOTimeStampObject
	{
	public:
		inline TOTimeStampObject():m_timeStamp(0){};
		inline long long getCurrentTime(){return m_timeStamp;};
		inline void setCurrentTime(long long time){m_timeStamp  =  time;};
	private:
		long long m_timeStamp;
	};
}

#endif