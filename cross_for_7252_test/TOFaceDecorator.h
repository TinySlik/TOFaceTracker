#ifndef TO_FACE_DECORATOR_H
#define TO_FACE_DECORATOR_H

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define _DEBUG_TO_FACE_DECORATOR true

class TOFaceDecorator
{
	public:
		TOFaceDecorator(int width,int height);
		virtual ~TOFaceDecorator();
		bool ProcessFrameBox(uchar* dst );
		bool ProcessHatThings(uchar* dst , const cv::Rect& faceRect,cv::Point& body);

		unsigned char * getGLHatImgBuffer(size_t& width,size_t& height);
		unsigned char * getGLFrameBoxImgBuffer();

	private:
		int imageRotate(cv::InputArray src, cv::OutputArray dst, double angle, bool isClip)  ;
		bool addMat4Uchar4(uchar* dst, const cv::Mat& frame);
		bool addMat4Uchar4(uchar* dst, const cv::Mat& frame , cv::Point pt);
		bool addMat4Uchar4(uchar* dst, const cv::Mat& frame,cv::Rect  rect);
		bool addMat4Uchar4(uchar* dst, const cv::Mat& frame,cv::Rect  rect,float rotation);
		int cvAdd4cMat(cv::Mat &dst, cv::Mat &scr) ;
		cv::Mat m_FrameBox;
		cv::Mat m_HatThings;

		int m_width;
		int m_height;

		unsigned char* hatBuffer;
		unsigned char* frameBoxBuffer;
};

#endif //TO_FACE_DECORATOR_H