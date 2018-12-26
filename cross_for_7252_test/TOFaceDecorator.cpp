#include "TOFaceDecorator.h"
#include <iostream>
#include <cstdio>
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

TOFaceDecorator::TOFaceDecorator(int width,int height):
m_width(width),
m_height(height),
hatBuffer(NULL),
frameBoxBuffer(NULL)
{
	m_FrameBox  = imread("../cv_resource/frameBox3.png",-1);
	m_HatThings = imread("../cv_resource/gh.png",-1);
	resize(m_FrameBox, m_FrameBox,cv::Size(width,height));
	resize(m_HatThings, m_HatThings,cv::Size(300,300));
}

unsigned char * TOFaceDecorator::getGLHatImgBuffer(size_t& width,size_t& height)
{
	if(hatBuffer)
	{
		return hatBuffer;
	}else
	{
		hatBuffer = (uchar*) new char[m_HatThings.cols * m_HatThings.rows * 4];
		uchar*  outData= hatBuffer;
	    uchar* dataIn=const_cast<uchar*>(m_HatThings.ptr<uchar>(0));
	    for(int i=0;i<m_HatThings.rows;i++)
	    {
	        for(int j=0;j<m_HatThings.cols;j++)
	        {
	            *outData++= *dataIn++ ;
                *outData++= *dataIn++ ;
                *outData++= *dataIn++ ;
                *outData++= *dataIn++ ;
	        }
	    }
		//todo
	}
	width = m_HatThings.cols;
	height = m_HatThings.rows;
	return hatBuffer;
}

unsigned char * TOFaceDecorator::getGLFrameBoxImgBuffer()
{
	if(frameBoxBuffer)
	{
		return frameBoxBuffer;
	}else
	{
		frameBoxBuffer = (uchar*) new char[m_width * m_height * 4];
		uchar*  outData= frameBoxBuffer;
	    uchar* dataIn=const_cast<uchar*>(m_FrameBox.ptr<uchar>(0));
	    for(int i=0;i<m_height;i++)
	    {
	        for(int j=0;j<m_width;j++)
	        {
	            *outData++= *dataIn++ ;
                *outData++= *dataIn++ ;
                *outData++= *dataIn++ ;
                *outData++= *dataIn++ ;
	        }
	    }
	}
	return frameBoxBuffer;
}

TOFaceDecorator::~TOFaceDecorator()
{
	if(frameBoxBuffer)
	{
		delete frameBoxBuffer;
	}
	if(hatBuffer)
	{
		delete hatBuffer;
	}
}

bool TOFaceDecorator::ProcessFrameBox(uchar* dst)
{
#if _DEBUG_TO_FACE_DECORATOR
    double times = (double)getTickCount();
#endif
	addMat4Uchar4(dst,m_FrameBox);

#if _DEBUG_TO_FACE_DECORATOR
    times = (double)getTickCount() - times;
    printf( "\n[TOFaceDecorator][TIME]ProcessFrameBox times cust frame time  %lf \n",times*1000/getTickFrequency());
#endif
    return true;
}

bool TOFaceDecorator::ProcessHatThings(uchar* dst , const cv::Rect& faceRect,cv::Point & body)
{
#if _DEBUG_TO_FACE_DECORATOR
    double times = (double)getTickCount();
#endif
    if(faceRect.width   < 10 || faceRect.height < 10 || body.x == 0 || body.y == 0)
    {
        return false;
    }

    Point center =  Point(faceRect.x+ faceRect.width/2,faceRect.y+faceRect.height/2);

    double rt  = atan((double)(center.x-body.x)/(double)(center.y - body.y)) * 180.0/3.1415926;
#if _DEBUG_TO_FACE_DECORATOR
    cout <<"center:" << center << "face" << faceRect << "body" << body << endl;
#endif
    Mat ne;
    imageRotate(m_HatThings, ne, 0-rt, false);
    resize(ne, ne,cv::Size(150,180));

    uchar* dataIn = dst;
    Mat* frame = new Mat (m_height,m_width,CV_8UC3);
    uchar* outData=frame->ptr<uchar>(0);
    for(int i=0;i<m_height ;i++)
    {
        for(int j=0;j<m_width;j++)
        {
            *outData++= *dataIn++;
            *outData++= *dataIn++;
            *outData++= *dataIn++;
            dataIn++;
        }
    }

    Mat mt(*frame, Rect(center.x - ne.cols/2,center.y - ne.rows/2,ne.cols,ne.rows)); 
    cvAdd4cMat(mt,ne); 

    uchar* outData2 = dst;
    uchar* dataIn2=frame->ptr<uchar>(0);
    for(int i=0;i<m_height ;i++)
    {
        for(int j=0;j<m_width;j++)
        {
            *outData2++= *dataIn2++;
            *outData2++= *dataIn2++;
            *outData2++= *dataIn2++;
            outData2++;
        }
    }

    //double ss = atan((double)(leftEye.y - rightEye.y)/(double)(leftEye.x-rightEye.x)) * 180/3.1415926;
    //
    

    /*
    Mat ne ;
    if(abs(m_rtHat) < 20)
    {
        imageRotate(m_Hat, ne, m_rtHat, false);
    }
    else
    {
        return false;
    }*/
    //Size dsize = Size(faceRect.width,faceRect.height);
    //Mat hh ;
    //resize(m_HatThings, hh,dsize);

    //Mat newMat(frame, faceRect); 
    //cvAdd4cMat(newMat,hh);

#if _DEBUG_TO_FACE_DECORATOR
    times = (double)getTickCount() - times;
    printf( "\n[TOFaceDecorator][TIME]ProcessHatThings times cust frame time  %lf \n",times*1000/getTickFrequency());
#endif
    return true;
}

bool TOFaceDecorator::addMat4Uchar4(uchar* dst, const cv::Mat& frame)
{
	uchar*  outData= dst;
    uchar* dataIn=const_cast<uchar*>(frame.ptr<uchar>(0));
    for(int i=0;i<m_height;i++)
    {
        for(int j=0;j<m_width;j++)
        {
        	uchar alpha = dataIn[3];
        	if(alpha)
        	{
        		uchar alpha_ = 255 - dataIn[3];
                float alp_ad  =  alpha/255.0f;
                float alp_mi  =  alpha_/255.0f;

	            *outData++= *dataIn++ * (alp_ad) + *outData *(alp_mi);
                *outData++= *dataIn++ * (alp_ad) + *outData *(alp_mi);
                *outData++= *dataIn++ * (alp_ad) + *outData *(alp_mi);
                outData++;
                dataIn++;
        	}else
        	{
        		outData+=4;
                dataIn+=4;
        	}
        }
    }
	return true;
}

bool TOFaceDecorator::addMat4Uchar4(uchar* dst, const cv::Mat& frame , cv::Point pt)
{
	return  true;
}

bool TOFaceDecorator::addMat4Uchar4(uchar* dst, const cv::Mat& frame , cv::Rect rect)
{
	return  true;
}

bool addMat4Uchar4(uchar* dst, const cv::Mat& frame,cv::Rect  rect,float rotation)
{
	return  true;
}

int TOFaceDecorator::cvAdd4cMat(cv::Mat &dst, cv::Mat &scr)   
{     
    if (dst.channels() != 3 || scr.channels() != 4)    
    {    
        return -1;
    } 

    if(scr.isContinuous()  &&  dst.isContinuous())
    {
        uchar* sc=scr.ptr<uchar>(0);
        uchar* dt=dst.ptr<uchar>(0);
        for(int i=0;i<scr.rows;i++)
        {
            for(int j=0;j<scr.cols;j++)
            {
                uchar alpha = sc[3];
                if(alpha)
                {
                    uchar alpha_ = 255 - sc[3];
                    float alp_ad  =  alpha/255.0f;
                    float alp_mi  =  alpha_/255.0f;
                    *dt++= *sc++ * (alp_ad) + *dt *(alp_mi);
                    *dt++= *sc++ * (alp_ad) + *dt *(alp_mi);
                    *dt++= *sc++ * (alp_ad) + *dt *(alp_mi);
                    sc++;
                }else
                {
                    dt+=3;
                    sc+=4;
                }
            }
        }
        return 1;
    }else
    {
        // 创建与原图像等尺寸的图像
        int nr=scr.rows;
        // 将3通道转换为1通道
        int nl=scr.cols;
        for(int k=0;k<nr;k++)
        {
            // 每一行图像的指针
            const uchar* inData=scr.ptr<uchar>(k);
            uchar* outData=dst.ptr<uchar>(k);
            for(int i=0;i<nl;i++)
            {
                uchar alpha = inData[i*4 + 3];
                if(alpha)
                {
                    uchar alpha_ = 255 - inData[i*4 + 3];
                    float alp_ad  =  alpha/255.0f;
                    float alp_mi  =  alpha_/255.0f;
                    outData[i*3 + 0] = inData[i*4 + 0];
                    outData[i*3 + 1] = inData[i*4 + 1];
                    outData[i*3 + 2] = inData[i*4 + 2];
                }else
                {
                    continue;
                }
            }
        }
    }
}   

//图像旋转: src为原图像， dst为新图像, angle为旋转角度, isClip表示是采取缩小图片的方式  
int TOFaceDecorator::imageRotate(InputArray src, OutputArray dst, double angle, bool isClip)  
{  
    Mat input = src.getMat();  
    if( input.empty() ) {  
        return -1;  
    }  
  
    //得到图像大小  
    int width = input.cols;  
    int height = input.rows;  
  
    //计算图像中心点  
    Point2f center;  
    center.x = width / 2.0;  
    center.y = height / 2.0;  
  
    //获得旋转变换矩阵  
    double scale = 1.0;  
    Mat trans_mat = getRotationMatrix2D( center, -angle, scale ); 
  
    //计算新图像大小  
    double angle1 = angle  * CV_PI / 180. ;  
    double a = sin(angle1) * scale;  
    double b = cos(angle1) * scale;  
    double out_width = height * fabs(a) + width * fabs(b); //外边框长度  
    double out_height = width * fabs(a) + height * fabs(b);//外边框高度  
  
    int new_width, new_height;  
    if ( ! isClip ) {  
        new_width = cvRound(out_width);  
        new_height = cvRound(out_height);  
    } else {  
        //calculate width and height of clip rect  
        double angle2 = fabs(atan(height * 1.0 / width)); //即角度 b  
        double len = width * fabs(b);  
        double Y = len / ( 1 / fabs(tan(angle1)) + 1 / fabs(tan(angle2)) );  
        double X = Y * 1 / fabs(tan(angle2));  
        new_width = cvRound(out_width - X * 2);  
        new_height= cvRound(out_height - Y * 2);  
    }  
  
    //在旋转变换矩阵中加入平移量  
    trans_mat.at<double>(0, 2) += cvRound( (new_width - width) / 2 );  
    trans_mat.at<double>(1, 2) += cvRound( (new_height - height) / 2);  
  
    //仿射变换  
    warpAffine( input, dst, trans_mat, Size(new_width, new_height));  
  
    return 0;  
}  

