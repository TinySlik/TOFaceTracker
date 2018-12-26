    //
    //  TOTrackerTwistedPair.h
    //  TOTrackerTwistedPair
    //
    //  Created by Tiny Oh 2018.1.14
    //  Copyright (c) 2018 White Sky. All rights reserved.
    //
    #include "TOTrackerTwistedPool.h"
    #include "TOFrameManager.h"
    #include "TOFaceDetector.h"
    #include "async++.h"
    #include "preprocessFace.h"

using namespace TO;

TOTrackerTwistedPool::TOTrackerTwistedPool(TOFrameManager & frameManager,TOFaceDetector & detector,const size_t width,const size_t height):
m_frameManager(&frameManager),
m_detector(&detector),
m_tmpTimeStampLast(0),
m_isSyncEnable(false),
m_isLandMarkEnable(false),
m_checkTimeStampLast(0),
m_width(width),
m_height(height),
m_tmpLoopFrameCountSum(0),
m_OneDetectLoopTag(false),
m_ShapeLandMarkFileName(SHAPE_PREDICTOR_DEFAULT),
m_outOfViewThr(TRACKER_OUT_OF_VIEW_THR),
m_ErroCountRiskToleranceThr(ERROR_COUNT_RISK_TOLERANCE_THR),
m_TrackerKeepCountThr(TRACKER_KEEP_COUNT_THR),
m_TrackerDormantCountEnableThr(TRACKER_DORMANT_COUNT_ENABLE_THR),
m_kcfHOG(HOG),
m_kcfFIXEDWINDOW(FIXEDWINDOW),
m_kcfMULTISCALE(MULTISCALE),
m_kcfSILENT(SILENT),
m_kcfLAB(LAB)
{
    m_calssName = typeid(*this).name();

    TO_THREAD_STATUS_UNIT units[] =
    {
        {"N2TO20TOTrackerTwistedPoolE&STATUS_PREPARE",  (void (TOStatusThreadBase::*)())(&TOTrackerTwistedPool::prepare)        },
        {"N2TO20TOTrackerTwistedPoolE&STATUS_TRACK",    (void (TOStatusThreadBase::*)())(&TOTrackerTwistedPool::track)          },
        {"N2TO20TOTrackerTwistedPoolE&STATUS_PRETRACK", (void (TOStatusThreadBase::*)())(&TOTrackerTwistedPool::pretrack)       },
        {"N2TO20TOTrackerTwistedPoolE&STATUS_DORMANT",  (void (TOStatusThreadBase::*)())(&TOTrackerTwistedPool::dormant)        },
        {"N2TO20TOTrackerTwistedPoolE&STATUS_ERROR",    (void (TOStatusThreadBase::*)())(&TOTrackerTwistedPool::error)          }
    };
    
    for (int i = 0; i < sizeof(units)/sizeof(TO_THREAD_STATUS_UNIT); ++i)
    {
        addStatusUnit(units[i]);
    }
    defineStatusRule(&TOStatusThreadBase::rule);

    KCFTracker tracker(m_kcfHOG, m_kcfFIXEDWINDOW, m_kcfMULTISCALE, m_kcfLAB);
    m_workerQueue.push_back(tracker);

    m_detector->addTrackerPool(*this);
}

std::string TOTrackerTwistedPool::rule(const std::string& last)
{
    if(last == "N2TO20TOTrackerTwistedPoolE&STATUS_PREPARE")
    {
        return "N2TO20TOTrackerTwistedPoolE&STATUS_PRETRACK";
    }else if(last == "N2TO20TOTrackerTwistedPoolE&STATUS_PRETRACK")
    {
        return "N2TO20TOTrackerTwistedPoolE&STATUS_TRACK";
    }else if(last == "N2TO20TOTrackerTwistedPoolE&STATUS_TRACK")
    {
        return "N2TO20TOTrackerTwistedPoolE&STATUS_PRETRACK";
    }else
    {
        return "N2TO20TOTrackerTwistedPoolE&STATUS_ERROR";
    }
}

void TOTrackerTwistedPool::afterStartThread()
{
    if(m_isLandMarkEnable)
    {
        dlib::deserialize(m_ShapeLandMarkFileName) >> m_dlibSp;
    }
}

void TOTrackerTwistedPool::setLandMarkEnable(bool enable)
{
    if(getCurrentStatus() == "N2TO20TOTrackerTwistedPoolE&STATUS_TRACK"  || getCurrentStatus() == "N2TO20TOTrackerTwistedPoolE&STATUS_PRETRACK" )
    {
        cout << "[ERROR]forbidden the handdle when worker is runing!,use it before the thread runing" << endl;
        return;
    }

    m_isLandMarkEnable = enable;
}

void TOTrackerTwistedPool::beforeStopThread()
{
    changeStatus("N2TO20TOTrackerTwistedPoolE&STATUS_DORMANT");
}

void TOTrackerTwistedPool::setMaxFaceCount(size_t num)
{
    if(num > THEORETICAL_UPPER_LIMIT)
    {
        cout << "[ERROR] big than THEORETICAL_UPPER_LIMIT is invalid!" << endl;
        return;
    }

    if(getCurrentStatus() == "N2TO20TOTrackerTwistedPoolE&STATUS_TRACK"  || getCurrentStatus() == "N2TO20TOTrackerTwistedPoolE&STATUS_PRETRACK" )
    {
        cout << "[ERROR]forbidden the handdle when worker is runing!,use it before the thread runing" << endl;
        return;
    }
    int idxC = m_workerQueue.size();

    while(idxC < num)
    {
        KCFTracker tracker(m_kcfHOG, m_kcfFIXEDWINDOW, m_kcfMULTISCALE, m_kcfLAB);
        m_workerQueue.push_back(tracker);
        idxC++;
    }

    while(idxC > num)
    {
        m_workerQueue.erase(m_workerQueue.begin());
        idxC--;
    }

    m_bridge.clear();
    for (int i = 0; i < idxC; ++i)
    {
        FRAME_STRATEGY_FILTRATION_UNIT st;
        st.initWithEmpty();
        m_bridge.push_back(st);
        m_bridgeDormantCount.push_back(0);
    }
}

void TOTrackerTwistedPool::getBridgeFromDetector(std::vector<FRAME_STRATEGY_FILTRATION_UNIT>& src)
{
    m_OneDetectLoopTag = true;
    for (int i = 0; i < src.size(); ++i)
    {
        if(i >= m_bridge.size())
        {
            cout << "setMaxFaceCount can be use at begin when you want to detector more than " << i << " people" << endl;
            break;
        }
        m_bridge[i] = src[i];
    }
    
    if( getCurrentStatus() == "N2TO20TOTrackerTwistedPoolE&STATUS_PREPARE" || getCurrentStatus() == "N2TO20TOTrackerTwistedPoolE&STATUS_TRACK")
    {
        nextStatus();
    }
}

TOTrackerTwistedPool::~TOTrackerTwistedPool()
{
    m_workerQueue.clear();
}

void TOTrackerTwistedPool::pretrack(void)
{
    #if 0
        cout << "[TOTrackerTwistedPool::pretrack]" << m_bridge.size() << endl;
    #endif
    TO_QUEUE_FRAME current = m_frameManager -> extractCurrentFrameInUse();
    m_tmpTimeStampLast = current.getTimeStamp();
    m_tmpRectPreRess.clear();

    int loopSum = getBridgeRealSize(m_bridge);
    for (int i = 0; i < loopSum; ++i)
    {
        m_tmpRectPreRess.push_back(0);
    }

    if (m_isSyncEnable)
    {
        async::parallel_for(async::irange(0, loopSum), [&](int i)
        {
            InitKCFTracker(m_workerQueue[i],current,m_bridge[i].getRectWithTimeStamp(current.getTimeStamp()),m_tmpRectPreRess[i]);
        });
    }else
    {
        for (int i = 0; i < loopSum; i++)
        {
            InitKCFTracker(m_workerQueue[i],current,m_bridge[i].getRectWithTimeStamp(current.getTimeStamp()),m_tmpRectPreRess[i]);
        }
    }

    double designedTimeInterval = 1000.0*1000.0 / getFPSSuperiorLimit();
    while(true)
    {
        double wait = 1000.0;
        bool tag = true;
        for (int i = 0; i < m_tmpRectRess.size(); ++i)
        {
            if(m_tmpRectPreRess[i])
            {
                    //
            }else
            {
                tag = false;
            }
        }

        if(tag)
        {
            break;
        }

        if(designedTimeInterval < 0)
        {
    #if 1
            cout << "ERROR: [TOTrackerTwistedPool::pretrack] the tracker is too slow than the designed time or something wrong in the kcf tracker!{too slow}" << endl;
    #endif      
        }

        usleep(wait);
        designedTimeInterval -= wait;
    }
    
    m_frameManager->relieveFrameInUse(current);
    nextStatus();
}

void TOTrackerTwistedPool::track(void)
{
    static int errCount = 0;
    #if 0
        cout << "[TOTrackerTwistedPool::track]" << endl;
    #endif
    TO_QUEUE_FRAME current = m_frameManager -> extractCurrentFrameInUse();

    if(m_tmpTimeStampLast == current.getTimeStamp() && m_tmpTimeStampLast != 0)
    {
    #if 1
        cout << "WARNING: [TOTrackerTwistedPool::track] same frame track , skip {too fast}" << endl;
    #endif
        m_frameManager->relieveFrameInUse(current);
        return;
    }
    m_tmpTimeStampLast = current.getTimeStamp();
    

    for (int i = 0; i < m_bridge.size() && i < m_workerQueue.size(); i++)
    {
        m_tmpRectPreRess.push_back(0);
        m_tmpRectRess.push_back(Rect0);
    }
    if (m_isSyncEnable)
    {
        int mx = m_bridge.size() < m_workerQueue.size() ? m_bridge.size() :m_workerQueue.size();
        async::parallel_for(async::irange(0, mx ), [&](int i) 
        {
            if(m_bridge[i].isWorking())
            {
                ProcessKCFTracker(m_workerQueue[i],current,m_tmpRectRess[i]);
            }else if(m_bridge[i].isWaitToStop())
            {
                m_bridge[i].oldRes = Rect0;
                m_bridge[i].newRes = Rect0;
            }else if(m_bridge[i].isWaitToActive())
            {
                m_bridge[i].oldRes = m_bridge[i].newRes;
                InitKCFTracker(m_workerQueue[i],current,m_bridge[i].newRes,m_tmpRectPreRess[i]);
                }else // dormant
                {
                    m_tmpRectRess[i].x = -1;
                }
            });
    }else
    {
        for (int i = 0; i < m_bridge.size() && i < m_workerQueue.size(); i++)
        {
            if(m_bridge[i].isWorking())
            {
                ProcessKCFTracker(m_workerQueue[i],current,m_tmpRectRess[i]);
            }else if(m_bridge[i].isWaitToStop())
            {
                m_bridge[i].oldRes = Rect0;
                m_bridge[i].newRes = Rect0;
            }else if(m_bridge[i].isWaitToActive())
            {
                m_bridge[i].oldRes = m_bridge[i].newRes;
                InitKCFTracker(m_workerQueue[i],current,m_bridge[i].newRes,m_tmpRectPreRess[i]);
                }else // dormant
                {
                    m_tmpRectRess[i].x = -1;
                }
            }
        }
        
        double designedTimeInterval = 1000.0*1000.0 / getFPSSuperiorLimit();
        while(true)
        {
            double wait = 1000.0;
            bool tag = true;
            for (int i = 0; i < m_tmpRectRess.size(); ++i)
            {
                if(m_tmpRectRess[i] == Rect0 && m_tmpRectPreRess[i] == 0 && m_bridge[i].isWorking())
                {
                    tag = false;
                }
            }

            if(tag)
            {
                break;
            }

            if(designedTimeInterval < 0)
            {
                for (int i = 0; i < m_tmpRectRess.size(); ++i)
                {
                    if(m_tmpRectRess[i] == Rect0)
                    {
                        cout <<"WARNING" << i << "is broken" << endl << "now we skip it once to keep loop normal" << endl;
                    }
                }
                errCount ++;

                cout << "ERROR: [TOTrackerTwistedPool::track] the tracker is too slow than the designed time or something wrong in the kcf tracker!{too slow}" << endl;

                if(errCount > m_ErroCountRiskToleranceThr)
                {
                    cout << "ERROR: more than 3 continuely frame tracker sync error" << endl;
                    changeStatus("N2TO20TOTrackerTwistedPoolE&STATUS_ERROR");
                }
            }

            usleep(wait);
            designedTimeInterval -= wait;
        }
        errCount = 0;
        
        m_frameManager->relieveFrameInUse(current);

        for (int i = 0; i < m_tmpRectRess.size(); ++i)
        {
            if(m_bridge[i].isWorking() && m_tmpRectRess[i] != Rect0)
            {
                m_bridge[i].addNewTail(current.getTimeStamp(), m_tmpRectRess[i]);
            }
        }
        checkAndResetSkipTag();
        
    #if 0
        for (int i = 0; i < m_tmpRectRess.size(); ++i)
        {
            cout << "RES:" << i << "===" << m_tmpRectRess[i] << endl;
        }
    #endif
        m_tmpRectRess.clear();
    }

    void TOTrackerTwistedPool::prepare(void)
    {
        static bool preTag = true;
        if(preTag){
            cout << "[TOTrackerTwistedPool::prepare]" << endl;
            preTag = false;
        }
    }

    void TOTrackerTwistedPool::checkAndJudgeStopOrActive()
    {
        checkIsOutOfView();
    }

    void TOTrackerTwistedPool::checkIsOutOfView()
    {
        //出外边界m_outOfViewThr时视作不再进入视野
        for (int i = 0; i < m_bridge.size(); ++i)
        {
            if(m_bridge[i].isWorking())
            {
                cv::Rect  rt = m_bridge[i].getRectWithTimeStamp();
                double ar = (rt & cv::Rect(0,0,m_width,m_height)).area();
                double az = rt.area();
                if(ar/az < m_outOfViewThr)
                {
    #if 1
                    cout << "m_bridge[" << i <<"]" << "is out of view , now set stop tag to the tracker" << endl;
    #endif
                    m_bridge[i].setWaitToStop();
                }
            }
        }
    }

    void TOTrackerTwistedPool::checkAndResetSkipTag()
    {
        checkAndJudgeStopOrActive();
        
        if(m_tmpLoopFrameCountSum++ > m_TrackerKeepCountThr && m_OneDetectLoopTag) 
        {
            noticeLoopEnd();
    #if 0
            cout << "[TOTrackerTwistedPool::checkAndResetSkipTag] checkAndResetSkipTag success reached!" << endl;
    #endif
            m_detector->getBridgeFromTracker(m_bridge);
            m_tmpLoopFrameCountSum = 0;
        }
    }

    void TOTrackerTwistedPool::dormant(void)
    {
        //todo
    }
    void TOTrackerTwistedPool::error(void)
    {
        //todo
    }

    bool TOTrackerTwistedPool::ProcessKCFTracker(KCFTracker& kcftracker, TO_QUEUE_FRAME& frame,cv::Rect& rect)
    {
        cv::Mat* in  =  frame.getData();
        assert(in);
    #if _TIME_DEBUG
        double times = (double)getTickCount();
    #endif
        //cout << *in << endl;
        rect = kcftracker.update(*in);
    #if _TIME_DEBUG
        times = (double)getTickCount() - times;
        printf( "\n[TOTrackerTwistedPool::ProcessKCFTracker][TIME]ProcessKCFTracker times cust frame time  %lf \n",times*1000/getTickFrequency());
    #endif
        return true;
    }

    bool TOTrackerTwistedPool::InitKCFTracker(KCFTracker& kcftracker,TO_QUEUE_FRAME& frame,const cv::Rect& rect,int& resTag)
    {
        cv::Mat* in  =  frame.getData();
        assert(in);
    #if _TIME_DEBUG
        double times = (double)getTickCount();
    #endif
        kcftracker.init( rect , *in);
        resTag = 1;
    #if _TIME_DEBUG
        times = (double)getTickCount() - times;
        printf( "\n[TOTrackerTwistedPool::InitKCFTracker][TIME]InitKCFTracker times cust frame time  %lf \n",times*1000/getTickFrequency());
    #endif
        return true;
    }

    #define _DEBUG_SIGNAL_FROM_DETECTOR false

    void TOTrackerTwistedPool::tryToActiveObject(const cv::Rect& src,size_t index)
    {
    #if _DEBUG_SIGNAL_FROM_DETECTOR
        cout << "[TOTrackerTwistedPool::tryToActiveObject]:"<< src << "index:" << index << endl;
    #endif
        if(m_bridge[index].isDomant())
        {
            m_bridgeDormantCount[index] = 0;
            m_bridge[index].newRes = src;
        }
    }

    void TOTrackerTwistedPool::tryToDormantObject(size_t index)
    {
    #if _DEBUG_SIGNAL_FROM_DETECTOR
        cout << "[TOTrackerTwistedPool::tryToDormantObject]" <<  index <<  endl;
    #endif
        for (int i = 0; i < m_bridgeDormantCount.size(); ++i)
        {
            m_bridgeDormantCount[i]++;
            if(m_bridge[index].isWorking() && m_bridgeDormantCount[i] > m_TrackerDormantCountEnableThr)
            {
                m_bridge[index].newRes = Rect0;
            }
        }
    }

    void TOTrackerTwistedPool::tryToFixObject(const cv::Rect& src,size_t index)
    {
    #if _DEBUG_SIGNAL_FROM_DETECTOR
        cout << "[TOTrackerTwistedPool::tryToFixObject]"<< src << index << endl;
    #endif
        if(m_bridge[index].isWorking())
        {
            m_bridgeDormantCount[index] = 0;
            m_bridge[index].oldRes = Rect0;
            m_bridge[index].newRes = src;
        }
    }

    void TOTrackerTwistedPool::keepObjectWell(size_t index)
    {
        m_bridgeDormantCount[index] = 0;
    }

    dlib::array2d<dlib::rgb_pixel> TOTrackerTwistedPool::cvMatToDLibRgb(const cv::Mat & cvMatRgb)
    {
        // BGR cv::Mat -> dlib::array2d
        dlib::array2d<dlib::rgb_pixel> dlibPixelsRGBA1;
        dlib::assign_image(dlibPixelsRGBA1, dlib::cv_image<dlib::rgb_pixel>(cvMatRgb));
        return dlibPixelsRGBA1;
    }

    dlib::array2d<dlib::bgr_pixel> TOTrackerTwistedPool::cvMatToDLibBgr(const cv::Mat & cvMatBgr)
    {
        // BGR cv::Mat -> dlib::array2d
        dlib::array2d<dlib::bgr_pixel> dlibPixelsRGBA1;
        dlib::assign_image(dlibPixelsRGBA1, dlib::cv_image<dlib::bgr_pixel>(cvMatBgr));
        return dlibPixelsRGBA1;
    }

    void TOTrackerTwistedPool::checkConcurrence(std::vector<TO_RECT> & vv,std::vector<TO_LANDMARK_68>& landMark,uchar* pBGRA)
    {
        if(getBridgeRealSize(m_bridge) <= 0)
        {
    #if 0
            printf("[TOFaceTracker checkConcurrence]no face found\n");
    #endif
            return;
        }

        std::vector<dlib::full_object_detection> shapes;
        std::vector<cv::Rect> vvF;
        TO_QUEUE_FRAME target = m_frameManager->extractCurrentFrameInUse();
        cv::Mat mat = *(target.getData());

        bool mkTag = m_isLandMarkEnable;
        
        if(m_checkTimeStampLast == target.getTimeStamp())
        {
            vv = m_resFaceLast;
            landMark = m_resLandMarkLast;
    #if 1
            printf("same frame in check , skip and use the last res instead.\n");
    #endif
            m_frameManager->relieveFrameInUse(target);
            return;
        }

        m_checkTimeStampLast = target.getTimeStamp();
        m_frameManager->relieveFrameInUse(target);

        for (int i = 0; i < m_bridge.size(); ++i)
        {
            if(m_bridge[i].isWorking())
            {
                cv::Rect face =m_bridge[i].getRectWithTimeStamp();
                vvF.push_back(face);
                TO_RECT faceInTO = {face.x,face.y,face.width,face.height};
                vv.push_back(faceInTO);
            }else
            {
                vvF.push_back(Rect0);
                TO_RECT faceInTO = {0,0,0,0};
                vv.push_back(faceInTO);
            }
        }

        if(mkTag)
        {
            if(m_resFaceLast.size() > 0 && m_resLandMarkLast.size() > 0)
            {
                bool kk = true;
                for (int i = 0; i < vv.size(); ++i)
                {
                    if(vv[i].x == m_resFaceLast[i].x && vv[i].y == m_resFaceLast[i].y && vv[i].width == m_resFaceLast[i].width && vv[i].height == m_resFaceLast[i].height )
                    {
    #if 0
                        if(m_bridge[i].isWorking())
                        {
                            cout << "index:" << i << "sameRes faces as last time,skip,may use last time landMark instead." << endl;
                        }
    #endif
                    }else
                    {
                        kk = false;
                    }
                }

                if(kk)
                {
                    landMark = m_resLandMarkLast;
                    mkTag = false;
                }
            }
        }
        

        if(mkTag)
        {
            int loopCount = vv.size();

            m_dlibImg = cvMatToDLibBgr(mat);
        #if _TIME_DEBUG
            double times = (double)getTickCount();
        #endif
            
            dlib::full_object_detection shape0;
            for (int i = 0; i < loopCount; ++i)
            {
                shapes.push_back(shape0);
            }
            if (m_isSyncEnable)
            {
                async::parallel_for(async::irange(0, loopCount), [&](int i)
                {
                    if(m_bridge[i].isWorking())
                    {
                        dlib::rectangle dF;
                        dF.set_top(vv[i].y);
                        dF.set_right(vv[i].x+vv[i].width);
                        dF.set_left(vv[i].x);
                        dF.set_bottom(vv[i].y+vv[i].height);
                        try
                        {
                            shapes[i] = m_dlibSp(m_dlibImg, dF);
                        }catch(cv::Exception &e)
                        {
                            cerr << "ERROR:[dlib]============================================================="  << endl;
                        }
                    }
                });

                double WAIT_TIME = 20.0;
                double espTimeInterval = 1000.0* WAIT_TIME;
                while(true)
                {
                    double wait = 1000.0;
                    bool brkTag = true;
                    for (int i = 0; i < loopCount; ++i)
                    {
                        if(m_bridge[i].isWorking())
                        {
                            if(shapes[i] == shape0)
                            {
                                brkTag = false;
                            }else
                            {
                                //
                            }
                        }
                    }
                    if(brkTag)
                    {
                        break;
                    }

                    if(espTimeInterval < 0)
                    {
    #if 1
                        cout << "ERROR: [TOTrackerTwistedPool::checkConcurrence]  dlib land mark out of time" << WAIT_TIME <<  " {too slow}" << endl;
    #endif      
                        break;
                    }
                    usleep(wait);
                    espTimeInterval -= wait;
                }
            }else
            {
                for (int i = 0; i < loopCount; ++i)
                {
                    if(m_bridge[i].isWorking())
                    {
                        dlib::rectangle dF;
                        dF.set_top(vv[i].y);
                        dF.set_right(vv[i].x+vv[i].width);
                        dF.set_left(vv[i].x);
                        dF.set_bottom(vv[i].y+vv[i].height);

                        shapes[i] = m_dlibSp(m_dlibImg, dF);
                    }
                }
            }
            
            #if _TIME_DEBUG
            times = (double)getTickCount() - times;
            printf( "\n[TOTrackerTwistedPool::Dlib LandMark][TIME]LandMark times cust frame time  %lf \n",times*1000/getTickFrequency());
            #endif

            for (int i = 0; i < vvF.size(); ++i)
            {
                TO_LANDMARK_68 tosp;
                if(m_bridge[i].isWorking())
                {
                    for (int j = 0; j < 68; ++j)
                    {
                        dlib::point pt = shapes[i].part(j);
                        tosp.shape[j].x = pt.x();
                        tosp.shape[j].y = pt.y();
                    }
                }
                landMark.push_back(tosp);
            }
            m_resLandMarkLast = landMark;
        }

        m_resFaceLast = vv;

        if(pBGRA)
        {
            for (int i = 0; i < vvF.size(); ++i)
            {
                if(m_bridge[i].isEmpty())
                {
                    continue;
                }
                rectangle(mat, vvF[i], CV_RGB(255,0, 0), 2, CV_AA);
                if(m_isLandMarkEnable)
                {
                    for (int j = 0; j < 68; ++j)
                    {
                        cv::putText(mat,to_string(j),cv::Point(landMark[i].shape[j].x,landMark[i].shape[j].y),FONT_HERSHEY_COMPLEX,0.3,(0,0,255),0.3);
                        circle(mat,cv::Point(landMark[i].shape[j].x,landMark[i].shape[j].y),2, CV_RGB(100,255, 0));
                    }
                }
            }

            uchar* outData=mat.ptr<uchar>(0);
            for(int i=0;i< mat.rows;i++)
            {
                for(int j=0;j< mat.cols;j++)
                {
                    *pBGRA++= *outData++;
                    *pBGRA++= *outData++;
                    *pBGRA++= *outData++;
                    *pBGRA++= 255;
                }
            }
        }
    }
