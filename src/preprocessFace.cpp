
#include "detectObject.h"
#include "preprocessFace.h"
#include "TOFaceType.h"

const double DESIRED_LEFT_EYE_X = 0.16;     //控制预处理后人脸的可见部分
const double DESIRED_LEFT_EYE_Y = 0.14;
const double FACE_ELLIPSE_CY = 0.40;
const double FACE_ELLIPSE_W = 0.50;
const double FACE_ELLIPSE_H = 0.80;         //脸的高

// 去除人脸的外部边界，处理后不包括背景和头发；并保持矩形的中心在原来相同的位置，而不会因为缩放而改变
Rect scaleRectFromCenter(const Rect wholeFaceRect, float scale)
{
    float faceCenterX = wholeFaceRect.x + wholeFaceRect.width * 0.5f;
    float faceCenterY = wholeFaceRect.y + wholeFaceRect.height * 0.5f;
    float newWidth = wholeFaceRect.width * scale;
    float newHeight = wholeFaceRect.height * scale;
    Rect faceRect;
    faceRect.width = cvRound(newWidth);                        // 收缩该区域
    faceRect.height = cvRound(newHeight);
    faceRect.x = cvRound(faceCenterX - newWidth * 0.5f);    // 去除该区域使矩形中心不变
    faceRect.y = cvRound(faceCenterY - newHeight * 0.5f);

    return faceRect;
}

// 检测人眼
void detectBothEyes(const Mat &face, CascadeClassifier &eyeCascade1, CascadeClassifier &eyeCascade2, Point &leftEye, Point &rightEye, Rect *searchedLeftEye, Rect *searchedRightEye)
{
/*
    // 眼睛检测器为"2splits.xml": 检测到的眼睛大约为人脸的60%，并检测闭着的眼睛
    const float EYE_SX = 0.12f;
    const float EYE_SY = 0.17f;
    const float EYE_SW = 0.37f;
    const float EYE_SH = 0.36f;
*/
/*
    // 眼睛检测器为 mcs.xml: 检测到的眼睛大约为人脸的80%，并检测闭着的眼睛
    const float EYE_SX = 0.10f;
    const float EYE_SY = 0.19f;
    const float EYE_SW = 0.40f;
    const float EYE_SH = 0.36f;
*/

    // 对于默认的 eye.xml/ eyeglasses.xml: 检测到的眼睛大约为人脸的40%，不能检测闭着的眼睛,
    //这里的值是用检测到的人脸矩形框中的相对坐标来表示
	const float EYE_SX = 0.16f;
    const float EYE_SY = 0.26f;
    const float EYE_SW = 0.30f;
    const float EYE_SH = 0.28f;

	//从检测到的人脸中提取左眼和右眼区域（搜索眼睛的区域）
    int leftX = cvRound(face.cols * EYE_SX);
    int topY = cvRound(face.rows * EYE_SY);
    int widthX = cvRound(face.cols * EYE_SW);
    int heightY = cvRound(face.rows * EYE_SH);
    int rightX = cvRound(face.cols * (1.0-EYE_SX-EYE_SW) );  // 从右眼角开始
    Mat topLeftOfFace = face(Rect(leftX, topY, widthX, heightY));
    Mat topRightOfFace = face(Rect(rightX, topY, widthX, heightY));
    Rect leftEyeRect, rightEyeRect;

    // 返回检测结果
    if (searchedLeftEye)
        *searchedLeftEye = Rect(leftX, topY, widthX, heightY);
    if (searchedRightEye)
        *searchedRightEye = Rect(rightX, topY, widthX, heightY);

    // 检测左边的区域，右边区域用 eyeCascade1检测器；同一个检测器对左眼和右眼都进行检测，若没检测到，则换第二个检测器
    detectLargestObject(topLeftOfFace, eyeCascade1, leftEyeRect, topLeftOfFace.cols);
    detectLargestObject(topRightOfFace, eyeCascade1, rightEyeRect, topRightOfFace.cols);

    // 没检测到眼睛时，则用不同的cascade 分类器
    if (leftEyeRect.width <= 0 && !eyeCascade2.empty())
	{
        detectLargestObject(topLeftOfFace, eyeCascade2, leftEyeRect, topLeftOfFace.cols);
#if _DEBUG_TO_FACE_TRACKER
        if (leftEyeRect.width > 0)
            cout << "eyeCascade2 left eye check success!" << endl;
        else
            cout << "eyeCascade2 left eye check failed!" << endl;
#endif
    }
    else
    {
#if _DEBUG_TO_FACE_TRACKER
        cout << "eyeCascade2 left eye check success!" << endl;
#endif
    }

    //检测右眼
    if (rightEyeRect.width <= 0 && !eyeCascade2.empty())
	{
        detectLargestObject(topRightOfFace, eyeCascade2, rightEyeRect, topRightOfFace.cols);
#if _DEBUG_TO_FACE_TRACKER
        if (rightEyeRect.width > 0)
            cout << "eyeCascade2 right eye check success!" << endl;
        else
            cout << "eyeCascade2 right eye check failed!" << endl;
#endif
    }
    else
    {
#if _DEBUG_TO_FACE_TRACKER
        cout << "eyeCascade2 right eye check success!" << endl;
#endif
    }

    if (leftEyeRect.width > 0)
	{   // 是否检测到左眼
        leftEyeRect.x += leftX;    // 调整左眼的矩形（由于去除了人脸的边界）
        leftEyeRect.y += topY;
        leftEye = Point(leftEyeRect.x + leftEyeRect.width/2, leftEyeRect.y + leftEyeRect.height/2);
    }
    else
	{
        leftEye = Point(-1, -1);    // 无效
    }

    if (rightEyeRect.width > 0)
	{ // 是否检测到右眼
        rightEyeRect.x += rightX; // 调整左眼的矩形
        rightEyeRect.y += topY;
        rightEye = Point(rightEyeRect.x + rightEyeRect.width/2, rightEyeRect.y + rightEyeRect.height/2);
    }
    else
	{
        rightEye = Point(-1, -1);    // 无效点
    }
}

//人脸预处理：左侧和右侧人脸直方图均衡化（标准化人脸左右两侧的亮度和对比度）
void equalizeLeftAndRightHalves(Mat &faceImg)
{
    // 对左侧和右侧人脸直方图均衡化;对中间部分的人脸进行直方图均衡化，最后混合三部分
	//分别进行均衡化后更容易处理光照的问题
    int w = faceImg.cols;
    int h = faceImg.rows;

    // 均衡化整个人脸
    Mat wholeFace;
    equalizeHist(faceImg, wholeFace);

    // 分别均衡化左侧和右侧人脸
    int midX = w/2;
    Mat leftSide = faceImg(Rect(0,0, midX,h));
    Mat rightSide = faceImg(Rect(midX,0, w-midX,h));
    equalizeHist(leftSide, leftSide);
    equalizeHist(rightSide, rightSide);

    //结合左侧、右侧和整个人脸均衡化后的图像, 使左右侧脸可以平滑过渡（左右侧脸分别进行均衡化后中间会出现一条线）
    for (int y=0; y<h; y++)
	{
        for (int x=0; x<w; x++)
		{
            int v;
			if (x < w / 4) // 左侧1/4
			{
                v = leftSide.at<uchar>(y,x);
            }
            else if (x < w*2/4) // 左侧偏中间1/1
			{
                int lv = leftSide.at<uchar>(y,x);
                int wv = wholeFace.at<uchar>(y,x);
                float f = (x - w*1/4) / (float)(w*0.25f);
                v = cvRound((1.0f - f) * lv + (f) * wv);
            }
            else if (x < w*3/4) // 右侧偏中间1/4
			{
                int rv = rightSide.at<uchar>(y,x-midX);
                int wv = wholeFace.at<uchar>(y,x);
                float f = (x - w*2/4) / (float)(w*0.25f);
                v = cvRound((1.0f - f) * wv + (f) * rv);
            }
            else // 右侧1/4
			{
                v = rightSide.at<uchar>(y,x-midX);
            }
            faceImg.at<uchar>(y,x) = v;
        }// end x
    }//end y
}

// 获得预处理的人脸
Mat getPreprocessedFace(Mat &srcImg, int desiredFaceWidth, CascadeClassifier &faceCascade, CascadeClassifier &eyeCascade1, CascadeClassifier &eyeCascade2, bool doLeftAndRightSeparately, Rect *storeFaceRect, Point *storeLeftEye, Point *storeRightEye, Rect *searchedLeftEye, Rect *searchedRightEye)
{
    // 正方形人脸
    int desiredFaceHeight = desiredFaceWidth;

    // 当不能检测到时，使人脸区域和眼睛的搜索区域无效
    if (storeFaceRect)
        storeFaceRect->width = -1;
    if (storeLeftEye)
        storeLeftEye->x = -1;
    if (storeRightEye)
        storeRightEye->x= -1;
    if (searchedLeftEye)
        searchedLeftEye->width = -1;
    if (searchedRightEye)
        searchedRightEye->width = -1;

    // 查找最大的人脸
    Rect faceRect;
    detectLargestObject(srcImg, faceCascade, faceRect);
    // 检测是否检测到人脸
    if (faceRect.width > 0)
	{
        // 返回检测结果
        if (storeFaceRect)
            *storeFaceRect = faceRect;

        Mat faceImg = srcImg(faceRect);    // 获得检测到的人脸图像

        Mat gray;
        if (faceImg.channels() == 3) {
            cvtColor(faceImg, gray, CV_BGR2GRAY);
        }
        else if (faceImg.channels() == 4) {
            cvtColor(faceImg, gray, CV_BGRA2GRAY);
        }
        else {
            gray = faceImg;
        }

        // 在原始图像中检测眼睛
        Point leftEye, rightEye;
        detectBothEyes(gray, eyeCascade1, eyeCascade2, leftEye, rightEye, searchedLeftEye, searchedRightEye);

        // 返回结果
        if (storeLeftEye)
            *storeLeftEye = leftEye;
        if (storeRightEye)
            *storeRightEye = rightEye;

        // 左眼和右眼都检测到了
        if (leftEye.x >= 0 && rightEye.x >= 0)
		{//当两眼不在同一直线上时，将其变换到同一直线

            // 两眼的中心
            Point2f eyesCenter = Point2f( (leftEye.x + rightEye.x) * 0.5f, (leftEye.y + rightEye.y) * 0.5f );
            // 两眼之间的角度
            double dy = (rightEye.y - leftEye.y);
            double dx = (rightEye.x - leftEye.x);
            double len = sqrt(dx*dx + dy*dy);
            double angle = atan2(dy, dx) * 180.0/CV_PI; //弧度转角度

            //
            const double DESIRED_RIGHT_EYE_X = (1.0f - DESIRED_LEFT_EYE_X);
            //
            double desiredLen = (DESIRED_RIGHT_EYE_X - DESIRED_LEFT_EYE_X) * desiredFaceWidth;
            double scale = desiredLen / len;
            // 获得仿射变换矩阵
            Mat rot_mat = getRotationMatrix2D(eyesCenter, angle, scale);
            // 将眼睛的中心转换到期望的位置
            rot_mat.at<double>(0, 2) += desiredFaceWidth * 0.5f - eyesCenter.x;
            rot_mat.at<double>(1, 2) += desiredFaceHeight * DESIRED_LEFT_EYE_Y - eyesCenter.y;

            // 仿射变换：旋转、缩放和平移图像
            Mat warped = Mat(desiredFaceHeight, desiredFaceWidth, CV_8U, Scalar(128)); // 变换后的矩阵
            warpAffine(gray, warped, rot_mat, warped.size());
            //imshow("warped eye", warped);

            // 根据图像本身的亮度确定均衡化的方式
            if (!doLeftAndRightSeparately)
            {
                equalizeHist(warped, warped);//整个图像
            }
            else
            {
                equalizeLeftAndRightHalves(warped);//分别均衡化
            }

            // 滤波，去除噪点；bilateralFilter-保持图像边缘
            Mat filtered = Mat(warped.size(), CV_8U);
            bilateralFilter(warped, filtered, 0, 20.0, 2.0);
            //imshow("filtered warped", filtered);

            // 使用椭圆掩码去除图像的拐角区域, 保留中间部分
            Mat mask = Mat(warped.size(), CV_8U, Scalar(0)); // 掩码
            Point faceCenter = Point( desiredFaceWidth/2, cvRound(desiredFaceHeight * FACE_ELLIPSE_CY) );
            Size size = Size( cvRound(desiredFaceWidth * FACE_ELLIPSE_W), cvRound(desiredFaceHeight * FACE_ELLIPSE_H) );
            ellipse(mask, faceCenter, size, 0, 0, 360, Scalar(255), CV_FILLED);

            // 使用掩码去除外边界像素
            Mat dstImg = Mat(warped.size(), CV_8U, Scalar(128)); //填充椭圆和矩形多余的部分为灰色

            // 使用掩码后的图像
            filtered.copyTo(dstImg, mask);  // 得到经过掩码处理后的人脸
            //imshow("dstImg", dstImg);

            return dstImg;
        }
    }
    return Mat();
}


// 获得单张预处理的人脸
bool getPreprocessedFaceOnly(Mat &srcImg, CascadeClassifier &faceCascade, Rect&storeFaceRect)
{
    detectLargestObject(srcImg, faceCascade, storeFaceRect);
    // 检测是否检测到人脸
    if (storeFaceRect.width * storeFaceRect.height > 100)
    {
        return true;
    }
    return false;
}

// 获得多个预处理的人脸
bool getPreprocessedFaceMutil(Mat &srcImg, CascadeClassifier &faceCascade, std::vector<Rect>& rectS)
{
    detectManyObjects(srcImg, faceCascade, rectS, 170.0f);
    // 检测是否检测到人脸
    if (rectS.size() > 0)
    {
        return true;
    }
    return false;
}
