/*************************************************************************
                             All Rights Reserved
                                 R&D Dept.
*************************************************************************/


/*************************************************************************

Filename:       osd_cube_test.c

Description:    

Author:         

Date:           

Modify Log:      
 ------------------------------------------------------------------------
1.
 修改人  :                           
 修改时间:
 修改原因:
 修改方法:
 ------------------------------------------------------------------------
2.
 修改人  :                           
 修改时间:
 修改原因:
 修改方法:
 ------------------------------------------------------------------------
*************************************************************************/
/***********************************************************************
*                                包含头文件
************************************************************************/
#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "esutil.h"
#include "osd_cube_test.h"


/***********************************************************************
*                                文件内部常量定义
***********************************************************************/


/***********************************************************************
*                                文件内部宏定义
***********************************************************************/
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

// define DRAW_CUBE to draw cube, other wise, draw texture
#define DRAW_CUBE




/***********************************************************************
*                                文件内部数据结构定义
***********************************************************************/


/***********************************************************************
*                                外部变量引用
***********************************************************************/


/***********************************************************************
*                                全局变量声明
***********************************************************************/


/***********************************************************************
*                                局部变量声明
***********************************************************************/
#ifdef DRAW_CUBE
static GLuint vbo[2];

static ESMatrix projection_matrix;
static ESMatrix modelview_matrix;
static ESMatrix mvp_matrix;

static GLint mvp_matrix_loc;
static GLint position_loc;
static GLint color_loc;

static GLint program_object;


static const GLfloat cube[] = {
   /*          POSITION                            COLOR                */
   1.000000f, 1.000000f, -1.000000f,     1.000000f, 0.000000f, 0.000000f, 0.500000f,
   1.000000f, -1.000000f, -1.000000f,    1.000000f, 0.000000f, 0.000000f, 0.500000f,
   -1.000000f, -1.000000f, -1.000000f,   1.000000f, 0.000000f, 0.000000f,  0.500000f,
   -1.000000f, 1.000000f, -1.000000f,    1.000000f, 0.000000f, 0.000000f,  0.500000f,

   -1.000000f, -1.000000f, 1.000000f,    1.000000f, 1.000000f, 0.000000f, 0.500000f,
   -1.000000f, 1.000000f, 1.000000f,     1.000000f, 1.000000f, 0.000000f, 0.500000f,
   -1.000000f, 1.000000f, -1.000000f,    1.000000f, 1.000000f, 0.000000f,  0.500000f,
   -1.000000f, -1.000000f, -1.000000f,   1.000000f, 1.000000f, 0.000000f,  0.500000f,

   1.000000f, -1.000000f, 1.000000f,     0.000000f, 0.000000f, 1.000000f, 0.500000f,
   1.000000f, 1.000000f, 1.000001f,      0.000000f, 0.000000f, 1.000000f, 0.500000f,
   -1.000000f, -1.000000f, 1.000000f,    0.000000f, 0.000000f, 1.000000f,  0.500000f,
   -1.000000f, 1.000000f, 1.000000f,     0.000000f, 0.000000f, 1.000000f,  0.500000f,

   1.000000f, -1.000000f, -1.000000f,    1.000000f, 0.000000f, 1.000000f, 0.500000f,
   1.000000f, 1.000000f, -1.000000f,     1.000000f, 0.000000f, 1.000000f, 0.500000f,
   1.000000f, -1.000000f, 1.000000f,     1.000000f, 0.000000f, 1.000000f,  0.500000f,
   1.000000f, 1.000000f, 1.000001f,      1.000000f, 0.000000f, 1.000000f,  0.500000f,

   1.000000f, 1.000000f, -1.000000f,     0.000000f, 1.000000f, 0.000000f, 0.500000f,
   -1.000000f, 1.000000f, -1.000000f,    0.000000f, 1.000000f, 0.000000f, 0.500000f,
   1.000000f, 1.000000f, 1.000001f,      0.000000f, 1.000000f, 0.000000f,  0.500000f,
   -1.000000f, 1.000000f, 1.000000f,     0.000000f, 1.000000f, 0.000000f,  0.500000f,

   1.000000f, -1.000000f, -1.000000f,    0.000000f, 1.000000f, 1.000000f, 0.500000f,
   1.000000f, -1.000000f, 1.000000f,     0.000000f, 1.000000f, 1.000000f, 0.500000f,
   -1.000000f, -1.000000f, 1.000000f,    0.000000f, 1.000000f, 1.000000f,  0.500000f,
   -1.000000f, -1.000000f, -1.000000f,   0.000000f, 1.000000f, 1.000000f,  0.500000f,
};

static const GLushort cube_idx[] = {
   0, 1, 2,
   3, 0, 2,
   4, 5, 6,
   7, 4, 6,
   8, 9, 10,
   9, 11, 10,
   12, 13, 14,
   13, 15, 14,
   16, 17, 18,
   17, 19, 18,
   20, 21, 22,
   23, 20, 22,
};

#else
static GLshort textureCoord[] = {
		// Mapping coordinates for the vertices
		0, 1, // top left (V2)
		0, 0, // bottom left (V1)
		1, 1, // top right (V4)
		1, 0 // bottom right (V3)
};

static GLfloat vertices[] = {
		-2.0f, 2.0f, -2.0f, // V2 - top left
		-2.0f, -2.0f, -2.0f, // V1 - bottom left
		2.0f, 2.0f, -2.0f, // V4 - top right
		2.0f, -2.0f, -2.0f, // V3 - bottom right
};
static GLuint texture;
#define GLTEXTURE_WIDTH 	1920
#define GLTEXTURE_HEIGHT 1080

#define GLVIEW_WIDTH 	1920
#define GLVIEW_HEIGHT 	1080
#endif
/***********************************************************************
*                                局部函数原型
***********************************************************************/


/***********************************************************************
*                                全局函数实现
***********************************************************************/
#ifdef DRAW_CUBE
void InitOSDGLState(void)
{
   /* The shaders */
   const char vShaderStr[] =
      "uniform mat4   u_mvpMatrix;               \n"
      "attribute vec4 a_position;                \n"
      "attribute vec4 a_color;                   \n"
      "varying vec4   v_color;                   \n"
      "                                          \n"
      "void main()                               \n"
      "{                                         \n"
      "  gl_Position = u_mvpMatrix * a_position; \n"
      "  v_color = a_color;                      \n"
      "}                                         \n";

   const char fShaderStr[] =
      "precision mediump float;                  \n"
      "varying vec4 v_color;                     \n"
      "                                          \n"
      "void main()                               \n"
      "{                                         \n"
      "  gl_FragColor = v_color;                 \n"
      "}                                         \n";

   GLuint     v, f;
   GLint      ret;
   const char *ff;
   const char *vv;
   char       *p, *q;

   glClearDepthf(1.0f);
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);  /* transparent background */

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

   /* Create vertex buffer objects */
   glGenBuffers(2, vbo);
   glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
   glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_idx), cube_idx, GL_STATIC_DRAW);

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

   esMatrixLoadIdentity(&projection_matrix);
   esMatrixLoadIdentity(&modelview_matrix);
   esMatrixLoadIdentity(&mvp_matrix);

   esMatrixLoadIdentity(&modelview_matrix);
   esTranslate(&modelview_matrix, 0, 0, -500);
   esScale(&modelview_matrix, 100, 100, 100);
}


void InitOSDGLViewPort(unsigned int width, unsigned int height, float panelAspect, unsigned char stretch)
{
   glViewport(0, 0, width, height);

   esMatrixLoadIdentity(&projection_matrix);

   if (stretch)
      esPerspective(&projection_matrix, 45.0f, panelAspect, 100, 1000);
   else
      esPerspective(&projection_matrix, 45.0f, (float)width / (float)height, 100, 1000);
}

static void DrawCube()
{
	/* Rotate the cube */
	esRotate(&modelview_matrix, 1.0f, 1, 0, 0);
	esRotate(&modelview_matrix, 0.5f, 0, 1, 0);

	/* Compute the final MVP by multiplying the model-view and perspective matrices together */
	esMatrixMultiply(&mvp_matrix, &modelview_matrix, &projection_matrix);

	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Clear all the buffers we asked for during config to ensure fast-path */
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program_object);

	/* Enable cube array */
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), BUFFER_OFFSET(0));
	glVertexAttribPointer(color_loc, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), BUFFER_OFFSET(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(position_loc);
	glEnableVertexAttribArray(color_loc);

	/* Load the MVP matrix */
	glUniformMatrix4fv(mvp_matrix_loc, 1, GL_FALSE, (GLfloat*)&mvp_matrix.m[0][0]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);

	/* Finally draw the elements */
	glDrawElements(GL_TRIANGLES, sizeof(cube_idx) / sizeof(GLushort), GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
}

#else

static void load_texture(unsigned char *pImageData, int nWidth, int nHeight, GLuint *texture)
{
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		nWidth,
		nHeight,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE, 
		pImageData);
}

static void LoadTextureFromBuffer(GLuint *texture)
{
	unsigned int* pData = (unsigned int*)malloc(GLTEXTURE_WIDTH*GLTEXTURE_HEIGHT*4);
	for (int i = 0; i < GLTEXTURE_HEIGHT; i++)
	{
		for (int j = 0; j < GLTEXTURE_WIDTH; j++)
		{
			pData[i*GLTEXTURE_WIDTH+j] = 0x7FFFFFFF;
		}
	}

	load_texture((unsigned char*)pData, GLTEXTURE_WIDTH, GLTEXTURE_HEIGHT, texture);
	printf("texture=%d\n", *texture);
	free(pData);
}

static void SetPerspective()
{	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustumf(-1, 1, -1, 1, 1.0f, 200.0f);
	glViewport(0, 0, GLVIEW_WIDTH, GLVIEW_HEIGHT);
	glMatrixMode(GL_MODELVIEW);
}

void InitOSDGLState()
{
   	LoadTextureFromBuffer(&texture);
}

void InitOSDGLViewPort(unsigned int width, unsigned int height, float panelAspect, unsigned char stretch)
{}

static void DrawTexture()
{
	glClearColorx(0, 0, 0, 0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	SetPerspective();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, vertices);	
	glTexCoordPointer(2, GL_SHORT,0, textureCoord);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
}

#endif

void DisplayOSD(void)
{
#ifdef DRAW_CUBE
	DrawCube();
#else
	DrawTexture();
#endif
   /* Don not call eglSwapBuffers for OSD */
}
/***********************************************************************
*                                局部函数实现
***********************************************************************/
