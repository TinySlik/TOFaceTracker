/*************************************************************************
                             All Rights Reserved
                                 R&D Dept.
*************************************************************************/


/*************************************************************************

Filename:       damd_main.c

Description:    

Author:         

Date:           

Modify Log:      
 ------------------------------------------------------------------------
1.
 ???? :                           
 ??????
 ?????:
 ??ķ???:
 ------------------------------------------------------------------------
2.
 ???? :                           
 ??????
 ?????:
 ??ķ???:
 ------------------------------------------------------------------------
*************************************************************************/
/***********************************************************************
*                                ????ͷ???
************************************************************************/
#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include "opencv_test.h"
#include <GLES2/gl2ext.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include <unistd.h>
#include <queue>
#include "NE10.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "TODynamiacBackgroundExtraction.h"

#include "esutil.h"
#include "LibTOFace.h"
#include "TOFaceDecorator.h"
using namespace cv;
using namespace std;

/***********************************************************************
*                                ???????궨?
***********************************************************************/
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define PIC_WIDTH 640
#define PIC_HEIGHT 480

#define PI 3.141592653

// define DRAW_CUBE to draw cube, other wise, draw texture

size_t g_hatWidth,g_hatHeight;

static unsigned char * g_FrameBufferData = NULL;

static unsigned int currentSzie = 0;

pthread_t testFunctionPauseResumeID ;

static bool isAfterBasckgroundCut = false;

static GLuint vbo[2];

static ESMatrix projection_matrix;
static ESMatrix modelview_matrix;
static ESMatrix mvp_matrix;
static ESMatrix3 hat_matrix;

static GLint mvp_matrix_loc;
static GLint hatMatrix_loc;
static GLint position_loc;
static GLint color_loc;
static GLint tex_loc;
static GLint tex_loc2;
static GLint tex_loc3;

static bool dataDoneTag = false;
   
long long sumTime  = 0;

pthread_mutex_t mutex_data_load = PTHREAD_MUTEX_INITIALIZER;

static unsigned int config_vpW = 1920;
unsigned int config_vpH = 1080;

static GLint program_object;

static GLfloat vertices[] = {
  -1.2f*2.0F, -0.8f*2.0F, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  
  1.2f*2.0F, -0.8f*2.0F, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  
  1.2f*2.0F, 0.8f*2.0F, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  
  -1.2f*2.0F, 0.8f*2.0F, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,

  -1.2f, -0.8f, 0.1f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  
  1.2f, -0.8f, 0.1f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  
  1.2f, 0.8f, 0.1f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,  
  -1.2f, 0.8f, 0.1f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
};
static GLushort indices[] = {
  0, 1, 2,  
  0, 2, 3,
//  4, 5, 6,
//  4, 6, 7    
};


GLuint VAOId, VBOId, EBOId ;
GLuint textureId = 0;
GLuint textureId2 = 0;
GLuint textureId3 = 0;

TO_RECT face;
// TO_POINT eyeL,eyeR;
// TO_POINT body;
/***********************************************************************
*                                ?ֲ????ԭ?
***********************************************************************/

TOFaceDecorator s_decorator(PIC_WIDTH,PIC_HEIGHT);

/* ----------------------------------------------------------------------
** Global defines
** ------------------------------------------------------------------- */

#define SRC_HEIGHT        512
#define SRC_WIDTH         512
#define DST_HEIGHT        734 //sqrt(512*512 + 512*512) + 10
#define DST_WIDTH         734 //sqrt(512*512 + 512*512) + 10
#define TEST_COUNT        5000

/* ----------------------------------------------------------------------
** Defines each of the tests performed
** ------------------------------------------------------------------- */


//input and output
static ne10_uint8_t * in_c = NULL;
static ne10_uint8_t * in_neon = NULL;

static ne10_uint8_t * out_c = NULL;
static ne10_uint8_t * out_neon = NULL;


extern void ne10_img_rotate_get_quad_rangle_subpix_rgba_neon (ne10_uint8_t *dst,
        ne10_uint8_t *src,
        ne10_int32_t srcw,
        ne10_int32_t srch,
        ne10_int32_t dstw,
        ne10_int32_t dsth,
        ne10_float32_t *matrix)
    asm("ne10_img_rotate_get_quad_rangle_subpix_rgba_neon");

/**
 * @brief Image rotate of 8-bit data.
 * @param[out]  *dst                  point to the destination image
 * @param[out]  *dst_width            width of destination image
 * @param[out]  *dst_height           height of destination image
 * @param[in]   *src                  point to the source image
 * @param[in]   src_width             width of source image
 * @param[in]   src_height            height of source image
 * @param[in]   angle                 angle of rotate
 *
 * The function extracts pixels from src at sub-pixel accuracy and stores them to dst.
 */
void ne10_img_rotate_rgba_neon (ne10_uint8_t* dst,
                                ne10_uint32_t* dst_width,
                                ne10_uint32_t* dst_height,
                                ne10_uint8_t* src,
                                ne10_uint32_t src_width,
                                ne10_uint32_t src_height,
                                ne10_int32_t angle)
{
    ne10_float32_t radian = (angle * NE10_PI / 180.0);
    ne10_float32_t a = sin (radian), b = cos (radian);
    ne10_int32_t srcw = src_width;
    ne10_int32_t srch = src_height;
    ne10_int32_t dstw = (srch * fabs (a)) + (srcw * fabs (b)) + 1;
    ne10_int32_t dsth = (srch * fabs (b)) + (srcw * fabs (a)) + 1;
    ne10_float32_t m[6];
    ne10_float32_t dx = (dstw - 1) * 0.5;
    ne10_float32_t dy = (dsth - 1) * 0.5;

    m[0] = b;
    m[1] = a;
    m[3] = -m[1];
    m[4] = m[0];
    m[2] = srcw * 0.5f - m[0] * dx - m[1] * dy;
    m[5] = srch * 0.5f - m[3] * dx - m[4] * dy;

    *dst_width = dstw;
    *dst_height = dsth;
    ne10_img_rotate_get_quad_rangle_subpix_rgba_neon (dst, src, srcw, srch, dstw, dsth, m);
}

//得出了neon完全没有卵用的结论。。。
void test_rotate_performance_case()
{
    ne10_int32_t i;
    ne10_int32_t channels = 4;
    ne10_int32_t in_size = SRC_HEIGHT * SRC_WIDTH * channels;
    ne10_int32_t out_size = DST_HEIGHT * DST_WIDTH * channels;
    ne10_int32_t srcw = SRC_WIDTH;
    ne10_int32_t srch = SRC_HEIGHT;
    ne10_uint32_t dstw_c, dsth_c;
    ne10_uint32_t dstw_neon, dsth_neon;
    ne10_int32_t angle;
    ne10_int64_t time_c = 0;
    ne10_int64_t time_neon = 0;

    /* init input memory */
    in_c = (ne10_uint8_t *)NE10_MALLOC (in_size * sizeof (ne10_uint8_t));
    in_neon = (ne10_uint8_t *)NE10_MALLOC (in_size * sizeof (ne10_uint8_t));

    /* init dst memory */
    out_c = (ne10_uint8_t *)NE10_MALLOC (out_size * sizeof (ne10_uint8_t));
    out_neon = (ne10_uint8_t *)NE10_MALLOC (out_size * sizeof (ne10_uint8_t));

    for (i = 0; i < in_size; i++)
    {
        in_c[i] = in_neon[i] = (rand() & 0xff);
    }
    double times1;
    double times2;
    //for (angle = -360; angle <= 360; angle += 5)
    for (angle = 100; angle <= 100; angle += 5)
    {
      printf ("rotate angle %d \n", angle);

      memset (out_c, 0, out_size);
      /* The current frame number */
      times1 = (double)getTickCount();
      for (i = 0; i < 100; i++)
         ne10_img_rotate_rgba_c (out_c, &dstw_c, &dsth_c, in_c, srcw, srch, angle);
      times1 = (double)getTickCount() - times1;

      memset (out_neon, 0, out_size);
      times2 = (double)getTickCount();
      for (i = 0; i < 100; i++)
         ne10_img_rotate_rgba_neon (out_neon, &dstw_neon, &dsth_neon, in_neon, srcw, srch, angle);
      times2 = (double)getTickCount() - times2;
      printf( "\ncost test time a %lf========b %lf\n",times1*1000/getTickFrequency(), times2*1000/getTickFrequency());
    }

    NE10_FREE (in_c);
    NE10_FREE (in_neon);
    NE10_FREE (out_c);
    NE10_FREE (out_neon);


    while(true);
}

//去内轮廓
void FillInternalContours(IplImage *pBinary, double dAreaThre)   
{   
    double dConArea;   
    CvSeq *pContour = NULL;   
    CvSeq *pConInner = NULL;   
    CvMemStorage *pStorage = NULL;   
    // 执行条件   
    if (pBinary)   
    {   
        // 查找所有轮廓
        pStorage = cvCreateMemStorage(0);   
        cvFindContours(pBinary, pStorage, &pContour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);   
        // 填充所有轮廓
        cvDrawContours(pBinary, pContour, CV_RGB(255, 255, 255), CV_RGB(255, 255, 255), 2, CV_FILLED, 8, cvPoint(0, 0));  
        // 外轮廓循环
        for (; pContour != NULL; pContour = pContour->h_next)   
        {
            // 内轮廓循环
            for (pConInner = pContour->v_next; pConInner != NULL; pConInner = pConInner->h_next)   
            {
                // 内轮廓面积
                dConArea = fabs(cvContourArea(pConInner, CV_WHOLE_SEQ));
                if (dConArea <= dAreaThre)
                {
                    cvDrawContours(pBinary, pConInner, CV_RGB(255, 255, 255), CV_RGB(255, 255, 255), 0, CV_FILLED, 8, cvPoint(0, 0));  
                }
            }
        }
        cvReleaseMemStorage(&pStorage);   
        pStorage = NULL;   
    }   
}  


void* testFunctionPauseResume(void* args)
{
   cout << "testFunctionPauseResume:\n";
   /*while(true)
   {
      usleep(30000000);
      //cout  << "testFunctionPauseResume pause begin:==============================================" << endl;
      //TOFaceTracker::getInstance()->pause();
      usleep(30000000);
      //cout  << "testFunctionPauseResume resume begin:==============================================" << endl;
      //TOFaceTracker::getInstance()->resume();
   }*/
   while(true)
   {
      //TO_Face_Tracker_Start("../cv_resource/haarcascade_frontalface_alt.xml",2000,5,1000,10000,PIC_WIDTH,PIC_HEIGHT,3);
      usleep(20000000);
       //det();
      //TO_Face_Tracker_Stop();
      usleep(20000000);
   }
   
}

void InitOpenCVGLState(void)
{
   /* The shaders */
   const char vShaderStr[] =
      "uniform mat4   u_mvpMatrix;                 \n"
      "uniform mat3   hatMatrix;                   \n"
      "attribute vec4 a_position;                  \n"
      "attribute vec4 a_color;                     \n"
      "varying vec4   v_color;                     \n"
      "attribute vec2 textCoord;                   \n"
      "varying vec2 TextCoord;                     \n"
      "                                            \n"
      "void main()                                 \n"
      "{                                           \n"
      "  gl_Position = u_mvpMatrix * a_position;   \n"
      "  v_color = a_color;                        \n"
      "  TextCoord = textCoord;                    \n"
      "}                                           \n";

   const char fShaderStr[] =

      "precision mediump float;                    \n"
      "uniform mat3   hatMatrix;                   \n"
      "varying vec4 v_color;                       \n"
      "varying vec2 TextCoord;                     \n"
      "uniform sampler2D tex;                      \n"
      "uniform sampler2D tex2;                     \n"
      "uniform sampler2D texHat;                   \n"
      "void main()                                 \n"
      "{                                           \n"
      "vec4 argbRes = texture2D(tex, vec2(TextCoord.x,1.0 -TextCoord.y));\n"
      "vec4 rgbaOri = vec4(argbRes.b,argbRes.g,argbRes.r,argbRes.a);\n"
      "vec4 frameBoxRes = texture2D(tex2, vec2(TextCoord.x,1.0 -TextCoord.y));\n"
      "vec3 hatMat = hatMatrix * vec3(TextCoord.x,1.0 -TextCoord.y,1.0);\n"
      "vec4 hatRes = texture2D(texHat, hatMat.xy);\n"
//      "vec3 rgb = mix(vec3(argbRes.b ,argbRes.g  , argbRes.r),frameBoxRes.rgb,0.5);\n"
//      "gl_FragColor = mix(vec4(argbRes.b ,argbRes.g  , argbRes.r , argbRes.a),texture2D(tex2, vec2(TextCoord.x,1.0 -TextCoord.y)),0.1); \n"
      "vec3 tmp = mix(rgbaOri.rgb,frameBoxRes.rgb,frameBoxRes.a);\n" 
      "gl_FragColor =vec4 (mix(tmp,hatRes.rgb,hatRes.a),rgbaOri.a + frameBoxRes.a + hatRes.a);\n"
//      "gl_FragColor =vec4 (mix(rgbaOri.rgb,frameBoxRes.rgb,frameBoxRes.a),rgbaOri.a);\n"
//      "gl_FragColor = v_color;\n"
      "}                                           \n";

   GLuint     v, f;
   GLint      ret;
   const char *ff;
   const char *vv;
   char       *p, *q;

   glClearDepthf(1.0f);
   glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  
   /* Gray background */

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

   /*glGenVertexArrays(1, &VAOId);
   glBindVertexArray(VAOId);*/
   glGenBuffers(1, &VBOId);
   glBindBuffer(GL_ARRAY_BUFFER, VBOId);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
   glGenBuffers(1, &EBOId);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOId);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
   // ????????
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
     8 * sizeof(GL_FLOAT), (GLvoid*)0);
   glEnableVertexAttribArray(0);
   // ????????
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
     8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GL_FLOAT)));
   glEnableVertexAttribArray(1);
   // ?????????   
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
     8 * sizeof(GL_FLOAT), (GLvoid*)(6 * sizeof(GL_FLOAT)));
   glEnableVertexAttribArray(2);

   glBindBuffer(GL_ARRAY_BUFFER, 0);
   //glBindVertexArray(0);

   v = glCreateShader(GL_VERTEX_SHADER);
   f = glCreateShader(GL_FRAGMENT_SHADER);

   ff = fShaderStr;
   vv = vShaderStr;
   glShaderSource(v, 1, &vv, NULL);
   glShaderSource(f, 1, &ff, NULL);

   /* Compile the shaders */
   glCompileShader(v);
   
   glGetShaderiv(v, GL_COMPILE_STATUS, &ret);
   if (ret == GL_FALSE)
   {
      glGetShaderiv(v, GL_INFO_LOG_LENGTH, &ret);
      p = (char *)alloca(ret);
      glGetShaderInfoLog(v, ret, NULL, p);
      assert(0);
   }
   glCompileShader(f);
   glGetShaderiv(f, GL_COMPILE_STATUS, &ret);
   if (ret == GL_FALSE)
   {
      glGetShaderiv(f, GL_INFO_LOG_LENGTH, &ret);
      q = (char *)alloca(ret);
      glGetShaderInfoLog(f, ret, NULL, q);
      assert(0);
   }

   program_object = glCreateProgram();
   glAttachShader(program_object, v);
   glAttachShader(program_object, f);

   /* Link the program */
   glLinkProgram(program_object);

   /* Get the attribute locations */
   position_loc = glGetAttribLocation(program_object, "a_position");
   color_loc = glGetAttribLocation(program_object, "a_color");

   /* Get the uniform locations */
   mvp_matrix_loc = glGetUniformLocation(program_object, "u_mvpMatrix");
   hatMatrix_loc = glGetUniformLocation(program_object, "hatMatrix");

   tex_loc = glGetUniformLocation(program_object, "tex");
   tex_loc2 = glGetUniformLocation(program_object, "tex2");
   tex_loc3 = glGetUniformLocation(program_object, "texHat");

   esMatrixLoadIdentity(&projection_matrix);
   esMatrixLoadIdentity(&modelview_matrix);
   esMatrixLoadIdentity(&mvp_matrix);
   esMatrix3LoadIdentity(&hat_matrix);

   esMatrixLoadIdentity(&modelview_matrix);
   esTranslate(&modelview_matrix, 0, 0, -500);
   esScale(&modelview_matrix, 100, 100, 100);

   glGenTextures(1, &textureId2);
   glBindTexture(GL_TEXTURE_2D, textureId2);
   
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   GLubyte * dt = (GLubyte *)s_decorator.getGLFrameBoxImgBuffer();
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, PIC_WIDTH, PIC_HEIGHT, 
         0, GL_RGBA, GL_UNSIGNED_BYTE, dt);

   glGenerateMipmap(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 0);

   glGenTextures(1, &textureId);
   glBindTexture(GL_TEXTURE_2D, textureId);
   
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 

   glGenTextures(1, &textureId3);
   glBindTexture(GL_TEXTURE_2D, textureId3);
   
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   
   GLubyte * dtHat = (GLubyte *)s_decorator.getGLHatImgBuffer(g_hatWidth,g_hatHeight);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_hatWidth, g_hatHeight, 
         0, GL_RGBA, GL_UNSIGNED_BYTE, dtHat);

   glGenerateMipmap(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, 0);

   set2DCanavaEnvironment(PIC_WIDTH,PIC_HEIGHT);
   set2DCanavaEnvironmentTarget(g_hatWidth,g_hatHeight);

   // TO_Face_Tracker_Init(
   //    "../cv_resource/haarcascade_frontalface_alt.xml",
   //    "../cv_resource/haarcascade_eye.xml",
   //    "../cv_resource/haarcascade_eye_tree_eyeglasses.xml",20000,1000000,15,1000);
   TO_Face_Tracker_Start("../cv_resource/haarcascade_frontalface_alt.xml",2000,5,1000,10000,PIC_WIDTH,PIC_HEIGHT,3);
   pthread_create(&testFunctionPauseResumeID, NULL, testFunctionPauseResume, NULL);
   //test_rotate_performance_case();
   
   
}

void InitOpenCVGLViewPort(unsigned int width, unsigned int height, float panelAspect, unsigned char stretch)
{
   glViewport(0, 0, width, height);

   esMatrixLoadIdentity(&projection_matrix);

   if (stretch)
      esPerspective(&projection_matrix, 45.0f, panelAspect, 100, 1000);
   else
      esPerspective(&projection_matrix, 45.0f, (float)width / (float)height, 100, 1000);
}

void OpenCVViewResize(void)
{
   EGLint w = 0, h = 0;

   /* As this is just an example, and we don't have any kind of resize event, we will
      check whether the underlying window has changed size and adjust our viewport at the start of
      each frame. Obviously, this would be more efficient if event driven. */
   eglQuerySurface(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW), EGL_WIDTH, &w);
   eglQuerySurface(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW), EGL_HEIGHT, &h);

   if (w != config_vpW || h != config_vpH)
   {
      config_vpW = w;
      config_vpH = h;

      /* Ignore the panelAspect and stretch - if we resized we are window based anyway */
      InitOpenCVGLViewPort(w, h, (float)w / (float)h, false);
   }
}

void OpenCVViewDisplay()
{
   if(!dataDoneTag)
   {
      return;
   }
   pthread_mutex_lock (&mutex_data_load);
   OpenCVViewResize();

   /* Compute the final MVP by multiplying the model-view and perspective matrices together */
   esMatrixMultiply(&mvp_matrix, &modelview_matrix, &projection_matrix);

   /* Clear all the buffers we asked for during config to ensure fast-path */
   glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glUseProgram(program_object);
   glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, (GLfloat*)&mvp_matrix.m[0][0]);
   
   //glBindVertexArray(VAOId);
   
   glActiveTexture(GL_TEXTURE0);
   glUniform1i(tex_loc, 0); 

   GLubyte *imageData = (GLubyte *)g_FrameBufferData;
   
   //skip 10 frames to make g_FrameBufferData stable.
   static int begin_frame_count = 1;
   if(begin_frame_count < 10)
   {
      begin_frame_count++;
   }else
   {
      if (imageData == NULL)
      {
         printf("Error::Texture could not load texture file:") ;
      }else
      {
         /* Variables. */
         static int frameNumber = 1;

         /* The current frame number */
         double times = (double)getTickCount();

         static double times_s = times;
         if(times > times_s)
         {
            printf("\n[frame time interval] %lf\n",(times-times_s)*1000/getTickFrequency());
            times_s = times;
         }

         TO_Face_Tracker_Sampling(g_FrameBufferData);
         std::vector<TO_RECT>  faceAry;
         //TO_Face_Tracker_Update(faceAry,50,g_FrameBufferData);
         TO_Face_Tracker_Check(faceAry,50,g_FrameBufferData);
         
         
         if(STATUS_TO_LIB_YES == TO_Face_is_Face_Exist_Now() && faceAry.size() > 0)
         {
            TO_RECT face  = faceAry[0];
            esMatrix3LoadIdentityWith2DInit(&hat_matrix);

            Point center = Point(face.x + face.width /2,face.y + face.height /2);

            float scale = 1.5F;
            
            esMatrix3ScaleOrigin(&hat_matrix, (float)face.width / g_hatWidth * scale,(float)face.height / g_hatHeight *scale);

            // if(isAfterBasckgroundCut)
            // {
            //    if (abs(center.y - body.y) < 0.01)
            //    {
            //       //
            //    }else
            //    {
            //       float rotation = (float)atan((float)(center.x - body.x)/(float)(center.y - body.y)) * 180.0F / PI;

            //       esMatrix3RotateOrigin(&hat_matrix, rotation);
            //    }
            //    esMatrix3Translate(&hat_matrix,0 - (double)(center.x + center.x - body.x)/ (double)PIC_WIDTH ,  0 - (double)(center.y + center.y - body.y)/ (double)PIC_HEIGHT );
            // }else
            // {
            esMatrix3Translate(&hat_matrix,0 - (double)(center.x)/ (double)PIC_WIDTH ,  0 - (double)(center.y  - face.height / 2)/ (double)PIC_HEIGHT );
            // }

            glUniformMatrix3fv(hatMatrix_loc, 1, GL_FALSE, (GLfloat*)&hat_matrix.m[0][0]);
         }else
         {
            esMatrix3LoadIdentityWith2DInit(&hat_matrix);

            esMatrix3Translate(&hat_matrix,0 - (double)(1000.0F)/ (double)PIC_WIDTH ,  0 - (double)(1000.0F)/ (double)PIC_HEIGHT ); 
            glUniformMatrix3fv(hatMatrix_loc, 1, GL_FALSE, (GLfloat*)&hat_matrix.m[0][0]);
         }

         esMatrix3LoadIdentityWith2DInit(&hat_matrix);

         // esMatrix3Translate(&hat_matrix,0 - (double)(1000.0F)/ (double)PIC_WIDTH ,  0 - (double)(1000.0F)/ (double)PIC_HEIGHT ); 
         // glUniformMatrix3fv(hatMatrix_loc, 1, GL_FALSE, (GLfloat*)&hat_matrix.m[0][0]);
         ++frameNumber;

         times = (double)getTickCount() - times;
         sumTime += times*1000/getTickFrequency();
         //printf( "Running time is: %g ms.\n", times*1000/getTickFrequency());
         //printf( "\nMain process logic cost frame time  %lf ,avg cost time is: %lld ms.\n",times*1000/getTickFrequency(), sumTime/frameNumber);
         //printf("%d::%d",frame.rows,frame.cols);

         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, PIC_WIDTH, PIC_HEIGHT, 
         0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

         glGenerateMipmap(GL_TEXTURE_2D);
         glBindTexture(GL_TEXTURE_2D, textureId);
      }
   }

   // 启用纹理单元 绑定纹理对象
   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D, textureId2);
   glUniform1i(tex_loc2, 1); // 设置纹理单元为1号

   // 启用纹理单元 绑定纹理对象
   glActiveTexture(GL_TEXTURE2);
   glBindTexture(GL_TEXTURE_2D, textureId3);
   glUniform1i(tex_loc3, 2); // 设置纹理单元为1号
   
   dataDoneTag = false;
   pthread_mutex_unlock(&mutex_data_load);
   
   glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
   //glBindVertexArray(0);

   eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_READ));
}

void restoreDateStatic(unsigned char *pBGRA32 , unsigned int size)
{
   if(size  > currentSzie)
   {
      printf("restoreDateStatic: new size: %d\n",size);
      if(g_FrameBufferData)
      {
         free(g_FrameBufferData);
         g_FrameBufferData = NULL;
      }
      g_FrameBufferData = (unsigned char *)malloc(size);
      currentSzie = size;
   }
   
   if(pBGRA32)
   {
      if(dataDoneTag)
      {
         return;
      }
      pthread_mutex_lock (&mutex_data_load);
      
      memcpy(g_FrameBufferData,pBGRA32,size);
      dataDoneTag = true;
      pthread_mutex_unlock(&mutex_data_load);
   }
}

/***********************************************************************
*                                ?ֲ????ʵ?
***********************************************************************/

void afterBasckgroundCut()
{
   isAfterBasckgroundCut =  true;
}
