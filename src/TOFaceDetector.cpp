//
//  FaceDetector.cpp
//  FaceDetector
//
//  Created by Tiny Oh 2018.1.14
//  Copyright (c) 2018 White Sky. All rights reserved.
//

#include "TOFaceDetector.h"
#include "TOSkinFiltering.h"
#include "TOImageSimilarityContrast.hpp"

using  namespace TO;

TOFaceDetector::TOFaceDetector(TOFrameManager & frameManager,string locationG,const size_t width,const size_t height):
m_frameManager(&frameManager),

m_faceDetectCountEnableThreshold(1),
m_tmpFaceDetectContinuityCount(1),

m_isNeedSkinFilter(false),

m_width(width),
m_height(height),
m_ORIrect(0,0,width,height),
m_BridgeRealSize(0),

m_saveMatterThr(DETECT_SAVE_MATTER_THR),
m_chainsConfidenceThr(DETECT_CHAINS_CONFIDENCE_THR),
m_fixConfidenceSplitThr(DETECT_FIX_CONFIDENCE_SPLIT_THR),
m_fixConfidenceEnableThr(DETECT_FIX_CONFIDENCE_ENABLE_THR),
m_fixConfidenceSkipThr(DETECT_FIX_CONFIDENCE_SKIP_THR),
m_DetectActiveBetweenSimilarThr(DETECT_SIMILAR_ENBALE_THR),

m_firstTime(true)
{
    m_skinFilter = new TOSkinFilter();
    m_calssName = typeid(*this).name();
    TO_THREAD_STATUS_UNIT units[] =
    {
        {"N2TO14TOFaceDetectorE&STATUS_PREPARE",    (void (TOStatusThreadBase::*)())(&TOFaceDetector::prepare)  },
        {"N2TO14TOFaceDetectorE&STATUS_PREDETECT",  (void (TOStatusThreadBase::*)())(&TOFaceDetector::predetect)},
        {"N2TO14TOFaceDetectorE&STATUS_DORMANT",    (void (TOStatusThreadBase::*)())(&TOFaceDetector::dormant)  },
        {"N2TO14TOFaceDetectorE&STATUS_DETECT",     (void (TOStatusThreadBase::*)())(&TOFaceDetector::detect)   },
        {"N2TO14TOFaceDetectorE&STATUS_ANALYSIS",   (void (TOStatusThreadBase::*)())(&TOFaceDetector::analysis) },
        {"N2TO14TOFaceDetectorE&STATUS_ERROR",      (void (TOStatusThreadBase::*)())(&TOFaceDetector::error)    }
    };

    for (int i = 0; i < sizeof(units)/sizeof(TO_THREAD_STATUS_UNIT); ++i)
    {
        addStatusUnit(units[i]);
    }
    defineStatusRule(&TOStatusThreadBase::rule);

    initDetectors(locationG);

    m_faceCascade->setMaskGenerator(m_skinFilter);
}

std::string TOFaceDetector::rule(const std::string& last)
{
    if(last == "N2TO14TOFaceDetectorE&STATUS_PREPARE")
    {
        return "N2TO14TOFaceDetectorE&STATUS_PREDETECT";
    }else if(last == "N2TO14TOFaceDetectorE&STATUS_PREDETECT")
    {
        return "N2TO14TOFaceDetectorE&STATUS_DETECT";
    }else if(last == "N2TO14TOFaceDetectorE&STATUS_DETECT")
    {
        return "N2TO14TOFaceDetectorE&STATUS_ANALYSIS";
    }else if(last == "N2TO14TOFaceDetectorE&STATUS_ANALYSIS")
    {
        return "N2TO14TOFaceDetectorE&STATUS_DORMANT";
    }
    else if(last == "N2TO14TOFaceDetectorE&STATUS_DORMANT")
    {
        return "N2TO14TOFaceDetectorE&STATUS_PREDETECT";
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

void TOFaceDetector::afterStartThread()
{
//to do
}

void TOFaceDetector::getBridgeFromTracker(std::vector<FRAME_STRATEGY_FILTRATION_UNIT>& src)
{
    if(getBridgeRealSize(src) == 0)
    {
        TO_QUEUE_FRAME current = m_frameManager -> extractCurrentFrameInUse();
        int xsss = aHash(*(current.getData()), m_frameManager->getSimlarSample());
        m_frameManager->setSimlarSample(*(current.getData()));
        if (xsss <= m_DetectActiveBetweenSimilarThr)
        {
            cout << "the aHash similar contract between 2 frame" << xsss << "is smaler than m_DetectActiveBetweenSimilarThr:" << m_DetectActiveBetweenSimilarThr << " ,so wait until next time" <<  endl;
            m_frameManager->relieveFrameInUse(current);
            return;
        }
        m_frameManager->relieveFrameInUse(current);
    }

    m_bridge = src;
    m_BridgeRealSize = getBridgeRealSize(m_bridge);
#if 0
cout << "[TOFaceDetector::getBridgeFromTracker] now analisys the rect array!" << endl;
#endif
    if(getCurrentStatus() == "N2TO14TOFaceDetectorE&STATUS_DORMANT" )
    {
        nextStatus();
    }else
    {
        cout << "ERROR:[TOFaceDetector::getBridgeFromTracker] not in DORMANT status!" << endl;
        changeStatus("N2TO14TOFaceDetectorE&STATUS_ERROR",true);
    }
}

void TOFaceDetector::setFaceDetectCountEnableThreshold(int num)
{
    m_faceDetectCountEnableThreshold  = num;
    m_resOnceST1.reserve(m_faceDetectCountEnableThreshold);
}

void TOFaceDetector::error(void)
{
    printf("[TOFaceDetector::error]error\n");
}

void TOFaceDetector::predetect(void)
{

#if 0
cout << "[TOFaceDetector::predetect]" << m_tmpFaceDetectContinuityCount << endl;
#endif

// 曾思考使用如下做法，头次或者原本没有有效脸识别的情况下 使用全图识别，取头框并集，然后与原本tracker结果并集，用于之后的 ct-1次的face detector的检索，最后再计算实际框位置并进入analysis ,但发现底层实现并无效率提升
    if(m_BridgeRealSize > 0)
    {
        m_tmpFaceDetectContinuityCount = 1;
#if 1
        cout << "[TOFaceDetector::predetect] normal predect  " << m_tmpFaceDetectContinuityCount << endl;
#endif
    }else
    {
        m_firstTime = true;
        m_tmpFaceDetectContinuityCount = m_faceDetectCountEnableThreshold;
#if 1
        cout << "[TOFaceDetector::predetect] first predect  " << m_tmpFaceDetectContinuityCount << endl;
#endif
    }
    m_resOnceST1.clear();
    nextStatus();
}

void TOFaceDetector::analysis(void)
{
    if(m_firstTime)
    {
        cout << "[TOFaceDetector::analysis]: first time analysis!" << endl;
#if 1 //近距离内接壤判定(权重 > r1 & r2 / r1| r2)  判定为一个追踪对象,分链   // 分链
        makeChains(m_resOnceST1,m_chainsConfidenceThr);
#endif

#if 1// 接散断链（头尾端四点回归 取可信度权重）
// todo
#endif

#if 1// 排除次要链 [且不与其他区域接壤(权重 < r1 & r2 / r1| r2)]
        prognosisChainHeadMergeReliability(m_resOnceST1);
#endif

#if 1 //尾链预判 当使用 tracker 时能够预判tracker的位置在 [ (t-t2)* xRate,(t-t2)* yRate,(t-t2)* width,(t-t2)* height] 故只保留最后两个链节点作为计算依据，链长度作为可靠性依据
        m_bridge = get2NodeTails(m_resOnceST1);
#endif
        if(m_trackerTwistedPool)
        {
            m_trackerTwistedPool->getBridgeFromDetector(m_bridge);
        }else
        {
            cout << "[ERROR] may be in debug now" << endl;
        }

        m_bridge.clear();
        m_resOnceST1.clear();
        m_resOnceST1.reserve(m_faceDetectCountEnableThreshold);

        nextStatus();

        m_firstTime = false;
    }else
    {
        cout << "[TOFaceDetector::analysis]: after the first time analysis!" << endl;
        nextStatus();
    }
}

void TOFaceDetector::dormant(void)
{
//todo
}

void TOFaceDetector::prepare(void)
{
    static bool preTag = true;
    if(preTag){
        cout << "[TOFaceDetector::prepare]" << endl;
        preTag = false;
    }

    if(m_frameManager->isWorking())
    {
        nextStatus();
    }
}

void TOFaceDetector::detect(void)
{
    static long long timeStampLast = 0;

    std::vector<cv::Rect> faceFindAray;

    TO_QUEUE_FRAME current = m_frameManager -> extractCurrentFrameInUse();

    if(timeStampLast == current.getTimeStamp() && timeStampLast != 0)
    {
        cout << "WARNING: same frame detect , skip" << endl;
        m_frameManager->relieveFrameInUse(current);
        return;
    }

    timeStampLast = current.getTimeStamp();

    ProcessFaceDetect(current,faceFindAray);

#if 0 //debug res:faceFindAray
    if(faceFindAray.size() > 0)
    {
        for (int i = 0; i < faceFindAray.size(); ++i)
    {
        cout << "find face" << i << " :  ";
        cout << faceFindAray[i];
    }
        cout << endl;
    }
#endif
    std::vector<STRATEGY_CHAIN_HOLDER_UNIT> tmpSTfaces;
    for (size_t i = 0; i < faceFindAray.size(); ++i)
    {
        std::vector<int> leftHand;
        std::vector<int> rightHand;
        STRATEGY_CHAIN_HOLDER_UNIT sm = {(size_t)m_tmpFaceDetectContinuityCount,i,leftHand,rightHand,faceFindAray[i]};
        tmpSTfaces.push_back(sm);
    }

    m_frameManager->relieveFrameInUse(current);

    std::vector<cv::Rect> sameTimeTrackerResesBF;
    if (m_trackerTwistedPool)
    {
        vector <FRAME_STRATEGY_FILTRATION_UNIT> trackerRes = m_trackerTwistedPool->getBridge();
        for (int i = 0; i < trackerRes.size(); ++i)
        {
            sameTimeTrackerResesBF.push_back(trackerRes[i].getRectWithTimeStamp());
        }
    }

    FRAME_STRATEGY_FACE_UNIT n = {timeStampLast,tmpSTfaces,sameTimeTrackerResesBF};
    if(!m_firstTime && m_trackerTwistedPool)
    {
        analysisTAndD(n);
    }

    m_resOnceST1.push_back(n);

    m_tmpFaceDetectContinuityCount --;

    if( m_tmpFaceDetectContinuityCount <= 0)
    {
        nextStatus();
    }
}
#define _DEBUG_STRATEGY_MATCH false

void TOFaceDetector::analysisTAndD(const FRAME_STRATEGY_FACE_UNIT & src)
{
    std::vector<FRAME_STRATEGY_FILTRATION_UNIT> res;
#if _DEBUG_STRATEGY_MATCH
    cout << "[TOFaceDetector::analysisTAndD]" << endl
    << "FRAME_STRATEGY_FACE_UNIT:" << endl
    <<  "timeStamp:" << src.timeStamp << endl
    << "stFaces:" << endl;
    for (int i = 0; i < src.STfaces.size(); ++i)
    {
        cout << i << src.STfaces[i].rect << endl;
    }

    cout << "sameTimeTrackerReses: " << endl;
    for (int i = 0; i < src.sameTimeTrackerReses.size(); ++i)
    {
        cout << i << src.sameTimeTrackerReses[i] << endl;
    }
#endif

    std::vector<std::vector<char>> tagTable;
    std::vector<int> bigSQueue;

    for (int i = 0; i < src.sameTimeTrackerReses.size(); ++i)
    {
        std::vector<char> line;
        double rsBst = -1.0;
        size_t dx = 0;

        for (int j = 0; j < src.STfaces.size(); ++j)
        {
            if(src.sameTimeTrackerReses[i] == Rect0)
            {
                line.push_back('a');
            }else
            {
                char tagRes = 'a';
                double thr = isBorderOnPer(src.sameTimeTrackerReses[i],src.STfaces[j].rect);
                if(thr > rsBst)
                {
                    dx = j;
                    rsBst = thr;
                }
                rsBst = thr > rsBst ? thr : rsBst;
                if(m_fixConfidenceSkipThr < thr)
                {
                    tagRes = 'e';
                }else if(m_fixConfidenceEnableThr < thr)
                {
                    tagRes = 'd';
                }else if(m_fixConfidenceSplitThr < thr)
                {
                    tagRes = 'c';
                }else
                {
                    tagRes = 'b';
                }
                line.push_back(tagRes);
            }
        }
        bigSQueue.push_back(dx);
        tagTable.push_back(line);
    }

#if _DEBUG_STRATEGY_MATCH
    for (int i = 0; i < tagTable.size(); ++i)
    {
        std::vector<char> line=tagTable[i];
        for (int j = 0; j < line.size(); ++j)
        {
            cout  << line[j] <<'|';
        }
        cout << endl;
    }
#endif

    std::vector<int> statusQueueAct;
    for (int i = 0; i < src.STfaces.size(); ++i)
    {
        int status = -1; // 0/waitToBeActive 1/nothingToDo
        for (int j = 0; j < tagTable.size(); ++j)
        {
            switch(tagTable[j][i])
            {
                case 'e':
                case 'd':
                case 'c':
                status = status < 1 ? 1 : status;
                break;
                case 'b':
                case 'a':
                status = status < 0 ? 0 : status;
                break;
                default:
                break;
            }
        }
        statusQueueAct.push_back(status);
        #if _DEBUG_STRATEGY_MATCH
        cout<< "queue " << i << "status: " << status << endl;
        #endif
    }

    std::vector<int> statusQueue;
    bool isAllClean = false;
    for (int i = 0; i < tagTable.size(); ++i)
    {
        std::vector<char> line = tagTable[i];
        int status = -1; // 0/dormant 1/waittostop (waring) 2/waittofix 3/nice -1/no face found

        for (int j = 0; j < line.size(); ++j)
        {
            switch(line[j])
            {
                case 'e':
                status = status < 3 ? 3 : status;
                break;
                case 'd':
                status = status < 2 ? 2 : status;
                break;
                case 'c':
                case 'b':
                status = status < 1 ? 1 : status;
                break;
                case 'a':
                status = status < 0 ? 0 : status;
                break;
                default:
                break;
            }
        }
#if _DEBUG_STRATEGY_MATCH
        cout<< "line " << i << "status: " << status << endl;
#endif
        statusQueue.push_back(status);
        if (status == 1)
        {
            m_trackerTwistedPool->tryToDormantObject(i);
        }else if(status == 2)
        {
            m_trackerTwistedPool->tryToFixObject(src.STfaces[bigSQueue[i]].rect,i);
        }else if(status == 3)
        {
            m_trackerTwistedPool->keepObjectWell(i);
        }else if(status == 0)
        {
        //
        }else if(status == -1)
        {
            isAllClean = true;
        }
    }


    if(isAllClean)
    {
    #if _DEBUG_STRATEGY_MATCH
        cout << "clean all tracker" << endl;
    #endif
        for (int i = 0; i < src.sameTimeTrackerReses.size(); ++i)
        {
            if(src.sameTimeTrackerReses[i] == Rect0)
            {
    //
            }else
            {
                m_trackerTwistedPool->tryToDormantObject(i);
            }
        }
    }

    for (int i = 0; i < statusQueueAct.size(); ++i)
    {
        for (int j = 0; j < statusQueue.size(); ++j)
        {
            if(statusQueueAct[i] == 0 && statusQueue[j] == 0)
            {
                statusQueueAct[i] = 1;//wait to active
                statusQueue[j] = 4;
                m_trackerTwistedPool->tryToActiveObject(src.STfaces[i].rect,j);
            }
        }
    }
    m_trackerTwistedPool->noticeLoopEnd();
}

size_t TOFaceDetector::getMinistIntervalIndex(std::vector<FRAME_STRATEGY_FACE_UNIT>& array)
{
    vector<long long> timeIntervals;
    long long last = 0;
    for (int i = 0; i < array.size(); ++i)
    {
        cout << i << ":  " << last << "||||" << array[i].timeStamp << endl;
//FRAME_STRATEGY_TRACKER_UNIT ;
        if(last == 0)
        {
//
        }else
        {
            if(array[i].timeStamp == last)
            {
                cout << "WARNING:" << " same time stamp when analysis,may get frame speed is too slow than detector" << endl;
            }
            timeIntervals.push_back(TO::getTimeStampIntervalABS(array[i].timeStamp,last));
        }
        last = array[i].timeStamp;
    }

//pick the smallest one
    int smResIndex = 0;
    assert(timeIntervals.size() > 1);
    for (int i = 1; i < timeIntervals.size(); ++i)
    {
        if(timeIntervals[smResIndex] > timeIntervals[i])
        {
            smResIndex = i;
        }
    }
    cout << "smResIndex:  " << smResIndex << "|| smallest interval:" << timeIntervals[smResIndex]*1000/getTickFrequency() << "ms" << endl;
    return smResIndex;
// array[smResIndex]
// array[smResIndex+1]
}

void TOFaceDetector::makeChains(vector<FRAME_STRATEGY_FACE_UNIT>& array,float trustThreshold)
{
    for (size_t scanIndex = 0; scanIndex < array.size()-1 ; ++scanIndex)
    {
        for (int i = 0; i < array[scanIndex].STfaces.size(); ++i)
        {
            for (int j = 0; j < array[scanIndex+1].STfaces.size(); ++j)
            {
                if(trustThreshold < isBorderOnPer(array[scanIndex].STfaces[i].rect,array[scanIndex+1].STfaces[j].rect))
                {
                    array[scanIndex].STfaces[i].rightHand.push_back(j);
                    array[scanIndex+1].STfaces[j].leftHand.push_back(i);
#if 0
cout  <<scanIndex<< ":" << i << "--" << scanIndex +1 << ":" << j << endl <<
"border persent" << isBorderOnPer(array[scanIndex].STfaces[i].rect,array[scanIndex+1].STfaces[j].rect) << endl;
#endif
                }
            }
        }
    }

#if 0
for (int i = 0; i < array.size(); ++i)
{
cout << "timeStamp:" << array[i].timeStamp << "STfaces:" << endl;
for (int j = 0; j < array[i].STfaces.size(); ++j)
{
cout << "frameIndex:" << array[i].STfaces[j].frameIndex << "rectIndex:" <<  array[i].STfaces[j].rectIndex << endl;
for (int k = 0; k < array[i].STfaces[j].leftHand.size(); ++k)
{
cout << "leftHand:" <<  k << ":" << array[i].STfaces[j].leftHand[k] << endl;
}
for (int k = 0; k < array[i].STfaces[j].rightHand.size(); ++k)
{
cout << "rightHand:" <<  k << ":" << array[i].STfaces[j].rightHand[k] << endl;
}
cout << "rect:" << array[i].STfaces[j].rect << endl;
} 
}
#endif
}

void  TOFaceDetector::prognosisChainHeadMergeReliability(vector<FRAME_STRATEGY_FACE_UNIT>& array)
{
//
}

std::vector<FRAME_STRATEGY_FILTRATION_UNIT> TOFaceDetector::get2NodeTails(const std::vector<FRAME_STRATEGY_FACE_UNIT>& array)
{
    std::vector<FRAME_STRATEGY_FILTRATION_UNIT> res;
    for (int i = 0; i < array.size(); ++i)
    {
        for (int j = 0; j < array[i].STfaces.size(); ++j)
        {
            if(array[i].STfaces[j].leftHand.size() == 0 && array[i].STfaces[j].rightHand.size() == 1)
            {
                size_t frmOrg = i;
                size_t idxOrg = j;
                int level = 1;
                STRATEGY_CHAIN_HOLDER_UNIT last = array[frmOrg].STfaces[idxOrg];
                long long lastTimeStamp = array[frmOrg].timeStamp;
                do
                {
                    idxOrg = array[frmOrg].STfaces[idxOrg].rightHand[0];
                    ++frmOrg;
                    if(array[frmOrg].STfaces[idxOrg].rightHand.size() == 1)
                    {
                        level ++;
                        last = array[frmOrg].STfaces[idxOrg];
                        lastTimeStamp = array[frmOrg].timeStamp;
                    }else if(array[frmOrg].STfaces[idxOrg].rightHand.size() == 0)
                    {
                        FRAME_STRATEGY_FILTRATION_UNIT out= {lastTimeStamp,array[frmOrg].timeStamp,last.rect,array[frmOrg].STfaces[idxOrg].rect};
                        res.push_back(out);
                        break;
                    }
                }while(true);
            }
        }
    }
#if 0
for (int i = 0; i < res.size(); ++i)
{
cout<<"res" 
<< i 
<<  ":  oldTimeStamp: " 
<< res[i].oldTimeStamp 
<< "newTimeStamp:  " 
<< res[i].newTimeStamp 
<< "oldRes:  " 
<< res[i].oldRes 
<< "newRes:  "
<< res[i].newRes
<< endl;
}
#endif
    return res;
}

bool TOFaceDetector::isBorderOn(const cv::Rect& c1,const cv::Rect& c2) // |
{
    return  (c1&c2).area() > 0;
}
bool TOFaceDetector::isBorderEqul(const cv::Rect& c1,const cv::Rect& c2) //&
{
    return c1 == c2;
}
float TOFaceDetector::isBorderOnPer(const cv::Rect& c1,const cv::Rect& c2)//%
{
    return (float)((c1 & c2).area()) / (float)((c1 | c2).area());
}

cv::Rect TOFaceDetector::rectCenterScale(cv::Rect rect, cv::Size size)
{  
    rect = rect + size;   
    Point pt;  
    pt.x = cvRound(size.width/2.0);  
    pt.y = cvRound(size.height/2.0);  
    cv::Rect res = (rect-pt);
    return res & cv::Rect(0,0,m_width,m_height);
}
// int  TOFaceDetector::chainMerge(std::vector < std::vector <FRAME_STRATEGY_CHAIN_UNIT> >& chains,const vector<FRAME_STRATEGY_FACE_UNIT>& array)
// {
//  //
// }


bool TOFaceDetector::FaceDetectResesToTracker(const FRAME_STRATEGY_FILTRATION_UNIT & to)
{
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
    cv::Mat* in   = frame.getData();
#if _TIME_DEBUG
    double times = (double)getTickCount();
#endif
    assert(in);
    cv::Mat* srcData = in;
    if(m_isNeedSkinFilter)
    {
        *srcData = YCrCb_Otsu_detect(*srcData);
    }

    bool isOK = getPreprocessedFaceMutil(*srcData, *m_faceCascade,res);

    if(isOK)
    {
#if _TIME_DEBUG
        times = (double)getTickCount() - times;
        printf( "\n[ProcessFaceDetect][TIME]ProcessFaceDetect face sum : %d \n success times cust frame time  %lf \n",(int)(res.size()),times*1000/getTickFrequency());
#endif
        return true;
    }

#if _TIME_DEBUG
    times = (double)getTickCount() - times;
    printf( "\n[ProcessFaceDetect][TIME]ProcessFaceDetect faild times cust frame time  %lf \n",times*1000/getTickFrequency());
#endif
    return false;
}