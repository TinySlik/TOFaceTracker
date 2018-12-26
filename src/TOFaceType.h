#ifndef TO_FACE_TYPE_H
#define TO_FACE_TYPE_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect.hpp>

#include "../include/LibTOFace.h"
#include "kcftracker.hpp"
#include "preprocessFace.h"
#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include <typeinfo>
#include <vector>
#include <map>
#include <string>
#include <climits>

#define Rect0 cv::Rect(0,0,0,0)
#define Point0 cv::Point(0,0)
#define PI 3.141592653

#define _TIME_DEBUG false

#define _REF_COUNT_DEBUG false

// kcf tracker config

namespace TO {
    static long long getTimeStampInterval(const long long & t1,const long long & t2)
    {
        if(abs(t1-t2) < LONG_LONG_MAX/2)
        {
            return t2 - t1;
        }else
        {
            return t1 > t2 ? t2 + LONG_LONG_MAX - t1 : t2 - LONG_LONG_MAX - t1;
        }
    }

    static long long getTimeStampIntervalABS(const long long & t1,const long long & t2)
    {
        return abs(getTimeStampInterval(t1,t2));
    }

// out analisys
    typedef struct{
        long long oldTimeStamp;
        long long newTimeStamp;

        cv::Rect  oldRes;
        cv::Rect  newRes;

        bool addNewTail(long long timeStamp, const cv::Rect& tail)
        {
            long long tv = getTimeStampInterval(newTimeStamp,timeStamp);
            if(tv < 0)
            {
                if(timeStamp <= newTimeStamp)
                {
                    std::cout << "ERROR! nothing can be allowed to input!" << std::endl;
                    return false;
                }
            }

            oldTimeStamp = newTimeStamp;
            oldRes = newRes;

            newTimeStamp = timeStamp;
            newRes = tail;
            return true;
        }

        cv::Rect getRectWithTimeStamp()
        {
            return getRectWithTimeStamp(cv::getTickCount());
        }

        cv::Rect getRectWithTimeStamp(long long time)
        {
            if(isEmpty())
            {
#if 0
std::cout << "empty" << std::endl;
#endif
                return Rect0;
            }

            if(newTimeStamp >= time)
            {
                std::cout << "WARNING: the get rect time is small than sample to calculate!" << std::endl;
            }
            cv::Rect res ;
            long long tv1 = getTimeStampInterval(oldTimeStamp,newTimeStamp);
            long long tv2 = getTimeStampInterval(newTimeStamp,time);

            double rate = (double)tv2/(double)tv1;

            if(abs (rate) > 5.0)
            {
                cout << "WARNING! the timeStamp is too old for the time Stamp now ,use the last rect instead" << endl; 
                return newRes;
            }
#if 0
std::cout <<"tv1: " <<  tv1 << "tv2:  " << tv2 << "rate:  " << rate << std::endl;
#endif           
            res.x = newRes.x + (newRes.x - oldRes.x)* rate;
            res.y = newRes.y + (newRes.y - oldRes.y)* rate;
            res.width = newRes.width + (newRes.width - oldRes.width)* rate;
            res.height = newRes.height + (newRes.height - oldRes.height)* rate;
            return res;
        }

        const bool isEmpty()
        {
            return oldRes == Rect0 || newRes == Rect0;
        }

        const bool isWaitToStop()
        {
            return oldRes != Rect0 && newRes == Rect0;
        }
        void setWaitToStop()
        {
            newRes = Rect0;
        }

        const bool isWaitToActive()
        {
            return oldRes == Rect0 && newRes != Rect0;
        }
        void setWaitToActive()
        {
            oldRes = Rect0;
        }

        const bool isDomant()
        {
            return oldRes == Rect0 && newRes == Rect0;
        }
        void setDomant()
        {
            oldRes = Rect0;
            newRes = Rect0;
        }

        const bool isWorking()
        {
            return !isEmpty();
        }

        void initWithEmpty()
        {
            oldRes = Rect0;
            newRes = Rect0;
            oldTimeStamp = 0;
            newTimeStamp = 0;
        }
    }FRAME_STRATEGY_FILTRATION_UNIT;

    static size_t getBridgeRealSize(std::vector<FRAME_STRATEGY_FILTRATION_UNIT>& src)
    {
        size_t res = 0;
        for (int i = 0; i < src.size(); ++i)
        {
            if(src[i].isEmpty())
            {
//
            }else
            {
                res ++;
            }
        }
        return res;
    }

    class TO_QUEUE_FRAME{
    public:
// error way
        static TO_QUEUE_FRAME createEmptyFrameInError()
        {
            TO_QUEUE_FRAME n;
#if 0
std::cout << "error frame" << std::endl;
assert(0);
#endif
            return n;
        }

// 1 copy slow
        static TO_QUEUE_FRAME create(cv::Mat& data)
        {
            TO_QUEUE_FRAME n;
            n.m_timeStamp = cv::getTickCount();
            n.m_data = new cv::Mat(data);
#if 0
std::cout << n.m_data << "new" << std::endl;
#endif
            return n;
        }

// nocopy but should with dangrous 'new'
// 
        static TO_QUEUE_FRAME create(cv::Mat* data)
        {
            TO_QUEUE_FRAME n;
            n.m_timeStamp = cv::getTickCount();
            n.m_data = data;
            return n;
        }

// nocopy with special format
        static TO_QUEUE_FRAME create(const uchar* pBGRA,const size_t width,const size_t height,const bool isDetectAlpha)
        {
            TO_QUEUE_FRAME n;
            n.m_timeStamp = cv::getTickCount();

            uchar* dataIn = const_cast<uchar*>(pBGRA);
            n.m_data = new cv::Mat (height,width,CV_8UC3);
#if 0
cout <<"m_timeStamp:  "<< n.m_timeStamp << endl;
cout << n.m_data << "new" << endl;
#endif
            uchar* outData=n.m_data->ptr<uchar>(0);
            if(isDetectAlpha)
            {
                for(int i=0;i<height ;i++)
                {
                    for(int j=0;j<width;j++)
                    {
                        *outData++= *dataIn++;
                        *outData++= *dataIn++;
                        *outData++= *dataIn++;
                        dataIn++;
                    }
                }
            }else
            {
                for(int i=0;i<height ;i++)
                {
                    for(int j=0;j<width;j++)
                    {
                        if(dataIn[3])
                        {
                            *outData++= *dataIn++;
                            *outData++= *dataIn++;
                            *outData++= *dataIn++;
                            dataIn++;
                        }else
                        {
                            outData += 3;
                            dataIn += 4;
                        }
                    }
                }
            }

            return n;
        }

        static bool remove(TO_QUEUE_FRAME* pFrame,bool isWaiting = false)
        {
            if(NULL == pFrame)
            {
                std::cerr << "null pFrame " << std::endl;
                return false;
            }

            if(isWaiting)
            {
                int j = 100;
                while(pFrame-> m_refCount > 0) 
                {
#if 1
                    if(j < 10000)
                    {
                        std::cout << "waiting frame not in use" << j << endl;
                        pFrame-> print();
                    }
#endif
                    usleep(100);
                    j+=100;
                }
#if 0
std::cout << "wait end bingo" << std::endl;
cout << pFrame-> m_data << "delete" << endl;
#endif
                delete pFrame-> m_data;
                return true;
            }else
            {
                if(pFrame-> m_refCount > 0)
                {
//std::cerr << "in use" << std::endl;
                    return false;
                }else
                {
#if 0
cout << pFrame-> m_data << "delete" << endl;
#endif
                    delete pFrame-> m_data;
                    return true;
                }
            }
        }

        TO_QUEUE_FRAME(const TO_QUEUE_FRAME& cp)
        {
#if 0
std::cout << "cp" << std::endl;
#endif
            m_timeStamp = cp.m_timeStamp;
            m_refCount = cp.m_refCount;
            m_isUniqueKeyFrame = cp.m_isUniqueKeyFrame;
            m_data = cp.m_data;

            if (NULL == cp.m_data)
            {
                std::cerr << "NULL data pointer" << std::endl;
                print();
                assert( cp.m_data );
            }
            if(cp.m_timeStamp == 0)
            {
                std::cerr << "refCount invalid operate" << std::endl;
                print();
                assert( false );
            }
        }

        virtual ~TO_QUEUE_FRAME()
        { 
// not remove the real data buff.
        }

        inline long long getTimeStamp(){return m_timeStamp;}
        inline void setTimeStamp(long long timeStamp ){m_timeStamp = timeStamp;}


        inline void setUniqueKeyFrame(bool isTrue){m_isUniqueKeyFrame = isTrue;}


        inline void print()
        {
            std::cout << "m_timeStamp: " << m_timeStamp 
            << "m_refCount"    << m_refCount
            << "m_isUniqueKeyFrame" << m_isUniqueKeyFrame
            << "m_data" << m_data
            << std::endl;
        }

        inline void setUsingExtract()
        {

#if 0
if(m_refCount > 1)
{

std::cout << "WARING! refCount big than 1.\n" << endl;
print();

}
#endif
#if _REF_COUNT_DEBUG
            std::cout << "[frame:]"<< m_timeStamp << "setUsingExtract with ref count " << m_refCount << "to" << m_refCount+1 <<std::endl;
#endif
            m_refCount++;
        }

        inline void setRelieveAble()
        {
#if _REF_COUNT_DEBUG
            std::cout << "[frame:]"<< m_timeStamp << "setRelieveAble with ref count " << m_refCount << "to" << m_refCount-1 <<std::endl;
#endif
            m_refCount--;
        }
        inline size_t getRefCount(){return m_refCount;}

        inline cv::Mat* getData(){return m_data;}

    private:
        long long m_timeStamp;
        size_t m_refCount;
        bool m_isUniqueKeyFrame;
        cv::Mat* m_data;
        TO_QUEUE_FRAME():m_refCount(0),m_timeStamp(0),m_isUniqueKeyFrame(false){};
    };
}

#endif //TO_FACE_TYPE_H
