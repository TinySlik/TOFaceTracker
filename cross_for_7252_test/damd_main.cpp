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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/serio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "damd_base_type.h"
#include "damd_porting_debug.h"
#include "damd_porting_platform.h"
#include "damd_porting_stream.h"
#include "damd_porting_audio.h"
#include "damd_porting_gfx.h"
#include "damd_porting_osd.h"
#include "damd_porting_record.h"
#include "damd_porting_audioenc.h"
#include "damd_porting_videoenc.h"
#include "damd_porting_singer.h"
#include "damd_porting_cloud.h"
#include "damd_porting_roland.h"
#include "damd_porting_camera.h"
#include "damd_porting_io.h"
#include "damd_porting_guide.h"
#include "damd_porting_i2c.h"
#include "opencv_test.h"
#include "osd_cube_test.h"


/***********************************************************************
*                                文件内部常量定义
***********************************************************************/


/***********************************************************************
*                                文件内部宏定义
***********************************************************************/
//#define TEST_FOR_ENCODER

/***********************************************************************
*                                文件内部数据结构定义
***********************************************************************/


/***********************************************************************
*                                外部变量引用
***********************************************************************/


/***********************************************************************
*                                全局变量声明
***********************************************************************/
int gMainPlayBackStatus = 0;
int gPipPlayBackStatus = 0;

/***********************************************************************
*                                局部变量声明
***********************************************************************/
typedef struct{
	char * video_name;
	int video_index;
}video_info;


/***********************************************************************
*                                局部函数原型
***********************************************************************/

int gSaveCameraYUV422Data = 0;
D_BOOL CameraYUV422Callbak(
	void* pUserData,
	int nWidth,
	int nHeight,
	unsigned char *pYUV422,
	int nYUVLength)
{
	if (gSaveCameraYUV422Data)
	{
		FILE *fp = fopen("/app/pictures/camera_yuv422.dat", "wb");
		if (fp)
		{
			fwrite(pYUV422, 1, nYUVLength, fp);
			fclose(fp);
		}
		gSaveCameraYUV422Data = 0;
	}
	return D_FALSE;
}

int gSaveCameraARGB32Data = 0;
D_BOOL CameraARGB32Callbak(
	void* pUserData,
	int nWidth,
	int nHeight,
	unsigned char *pARGB32,
	int nARGB32Length)
{
	restoreDateStatic(pARGB32 , nARGB32Length);
	if (gSaveCameraARGB32Data)
	{
		FILE *fp = fopen("/app/pictures/camera_argb32.dat", "wb");
		if (fp)
		{
			fwrite(pARGB32, 1, nARGB32Length, fp);
			fclose(fp);
		}
		gSaveCameraARGB32Data = 0;
	}
	return D_FALSE;
}

int gCameraColorKeyEnable = 0;
void OnCameraColorKeyLearnDoneCallback(
	void* pUserData)
{
	printf("OnCameraColorKeyLearnDoneCallback\n");
	gCameraColorKeyEnable = 1;
}

// if the network un-reachable too long time( >2 hours ), 
// this callback will be called on every cloud api calls when network is availble
D_BOOL OnLoginExpiredCallback(D_VOID* pParam)
{
	printf("token expired, if you have sign in, please sign out, and re-signin again\n");
	WSD_AL_CLOUD_User_SignOut();

	WSD_AL_CLOUD_USER_DETAIL_T user={0};
	WSD_AL_CLOUD_User_SignIn(WSD_AL_CLOUD_USER_SIGNIN_BY_ACCOUNT_E, "Test", "1234", &user);

	// here, if you return D_FALSE, this function will called again on next cloud api called
	return D_TRUE;
}

int gCancelUploading = 0;

int OnUploadProgressCallback(void *pUserData, int nTotalUploadBytes,int nDroppedBytes,int nSizeToUpload, float fSpeed,D_BOOL bStopped,int nStopReason)
{
	printf("upload data, remain: %d, speed=%.2f kbps\n", nSizeToUpload, fSpeed);

	// return: 0: cancel uploading,  other: continue upload
	if (gCancelUploading)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int WSD_HDMI_CALLBACK(WSD_SYS_EVENT_E event)
{
	printf("WSD_HDMI_CALLBACK>>>>>event=%d \n",event);

	switch(event)
	{
		case SYS_EVENT_HDMI_OUT_CONNECT:
			 printf("HDMI OUT connect\n");
			break;
		case SYS_EVENT_HDMI_OUT_DISCONNECT:
			 printf("HDMI OUT disconnect\n");
			break;
		case SYS_EVENT_HDMI_IN_CONNECT:
			 printf("HDMI IN connect\n");
			break;
		case SYS_EVENT_HDMI_IN_DISCONNECT:
			 printf("HDMI IN disconnect\n");
			break;
		default:
			break;
	}

	return 0;
}


int WSD_STREAM_MainCALLBACK(enum stream_event_type event)
{
    printf("WSD_STREAM_MainCALLBACK>>>>>event=%d \n",event);
    switch(event)
    {
        case STREAM_PLAYBACK_COMPLETE:
             gMainPlayBackStatus = STREAM_PLAYBACK_COMPLETE;
        	break;
		default:
		    break;
    }
}

int WSD_STREAM_PipCALLBACK(enum stream_event_type event)
{
    printf("WSD_STREAM_PipCALLBACK>>>>>event=%d \n",event);
    switch(event)
    {
        case STREAM_PLAYBACK_COMPLETE:
             gPipPlayBackStatus = STREAM_PLAYBACK_COMPLETE;
        	break;
		default:
		    break;
    }
}

typedef unsigned long long UINT64;

UINT64 GetTickCountUS()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (((UINT64)ts.tv_sec)*1000000 + ((UINT64)ts.tv_nsec)/1000);
}

UINT64 GetTickCount()
{
	return GetTickCountUS() / 1000;
}

void* draw_thread(void* param)
{
    WSD_AL_GFX_EGL_WINDOW EglWindow;
	D_INT32 ret;

	EglWindow = WSD_AL_GFX_CreateEglWindow(D_SCREEN_VGA_WIDTH, D_SCREEN_VGA_HEIGHT);
	if(NULL == EglWindow)
	{
	    printf("WSD_AL_GFX_CreateEglWindow fail !\n");
	    //return D_FAILURE;
	}

	ret = WSD_AL_GFX_InitEgl(EglWindow);
	if (D_SUCCESS != ret)
	{
	    printf("WSD_AL_GFX_InitEgl fail !\n");
	    //return D_FAILURE;
	}

    InitOpenCVGLState();
    InitOpenCVGLViewPort(D_SCREEN_VGA_WIDTH, D_SCREEN_VGA_HEIGHT, (float)D_SCREEN_VGA_WIDTH / (float)D_SCREEN_VGA_HEIGHT, 1);

	/*alphaֵ*/
    //WSD_AL_GFX_SetAlpha(128);

	UINT64 uTimeStart = GetTickCount();
	int frames = 0;

	while(1)
	{
	    OpenCVViewDisplay();
	    frames++;
		UINT64 uTimeEnd = GetTickCount();
	    if (uTimeEnd - uTimeStart > 2000)
	    {
			//printf("gfx fps = %d\n", frames/2);
			frames = 0;
			uTimeStart = uTimeEnd;
		}
	    usleep(16*1000);
	};
	return  NULL;
}

void* osd_draw(void* param)
{
    D_INT32 width = 0;
    D_INT32 height = 0;
    int i;
    D_INT32 ret;

    WSD_AL_OSD_GetScreenSize(&width, &height);

    ret = WSD_AL_OSD_InitEgl(width, height);
	if (D_SUCCESS != ret)
	{
	    printf("WSD_AL_OSD_InitEgl failed !\n");
	    //return D_FAILURE;
	}

    InitOSDGLState();
    InitOSDGLViewPort(width, height, (float)width / (float)height, 1);

	UINT64 uTimeStart = GetTickCount();
	int frames = 0;

	while(1)
	{
		/* 画2D */
		//WSD_AL_OSD_FillRect(0, 0, 1920, 540, 0x7F00FF00);

		/* 画3D动画 */
		DisplayOSD();

		/* 将OSD缓存刷到HDMI, opengl在osd之上 */
		WSD_AL_OSD_UpdateWithEgl(1);
		//WSD_AL_OSD_Update();

	    frames++;
		UINT64 uTimeEnd = GetTickCount();
	    if (uTimeEnd - uTimeStart > 2000)
	    {
			//printf("osd fps = %d\n", frames/2);
			frames = 0;
			uTimeStart = uTimeEnd;
		}

		/* OSD线程最好不要占满CPU, 按需调整sleep时长 */
		usleep(16*1000);
	}
	return  NULL;
}

void* video_thread(video_info* videoInfo)
{
    int ret = 0;
	WSD_AL_PLAY_SOURCE_T PlaySource;

RESTART:
	printf("play stream[%d] play\n", videoInfo->video_index);
	memset(&PlaySource, 0x00, sizeof(PlaySource));
	WSD_AL_PLAYER_ProbeFile((D_UINT8*)videoInfo->video_name, &(PlaySource.MediaParam));
	PlaySource.PlayMode = FILE_STREAM;
	strncpy((char *)PlaySource.SourceInfo.File.FilePath, videoInfo->video_name, URL_LEN-1);
	WSD_AL_PLAYER_SetSource(videoInfo->video_index, &PlaySource);
	WSD_AL_PLAYER_Play(videoInfo->video_index);

	while(1)
	{
	    if(videoInfo->video_index == 0)
    	{
			if(gMainPlayBackStatus == STREAM_PLAYBACK_COMPLETE)
			{
			    gMainPlayBackStatus = 0;
				break;
			}
    	}
		else
		{
			if(gPipPlayBackStatus == STREAM_PLAYBACK_COMPLETE)
			{
				gPipPlayBackStatus = 0;
				break;
			}
		}

		usleep(1000);
	}

	WSD_AL_PLAYER_FlushBuffer(videoInfo->video_index);
	WSD_AL_PLAYER_Stop(videoInfo->video_index);
	printf("stop stream[%d] play\n", videoInfo->video_index);
	goto RESTART;
}


D_INT32 OnSingerNotificationCallback(
	WSD_AL_SINGER_MSG_TYPE_E eMsgType,
	D_VOID* pParam,
	D_VOID* pData)
{
	if (eMsgType == WSD_AL_SINGER_MSG_SING_EVENT_E)
	{
		WSD_AL_SINGER_SING_EVENT_PARAM_T *pEventParam =
			(WSD_AL_SINGER_SING_EVENT_PARAM_T*) pData;
		switch(pEventParam->event)
		{
		case WSD_AL_SINGER_EVENT_OUT_OF_TUNE_E:
			printf("out of tune detect!\n");
			break;
		case WSD_AL_SINGER_EVENT_OUT_OF_SYNC_E:
			printf("out of sync detect!\n");
			break;
		case WSD_AL_SINGER_EVENT_HIT_PERFECT_E:
			printf("hit perfect!\n");
			break;
		case WSD_AL_SINGER_EVENT_HIT_NORMAL_E:
			printf("hit normal!\n");
			break;
		case WSD_AL_SINGER_EVENT_HIT_BAD_E:
			printf("hit bad!\n");
			break;
		default:
			break;
		}
	}

	return D_SUCCESS;
}

/***********************************************************************
*                                全局函数实现
***********************************************************************/
/******************************************************************
函数原型: main

功能描述: 程序入口函数

参    数:
    argc(in): 参数个数
    argv(in): 参数信息

返 回 值:
    0 : 成功
    -1: 失败

注    意:
******************************************************************/
int main(int argc, char *argv[])
{
	WSD_AL_GFX_EGL_WINDOW EglWindow;
	WSD_OUTPUT_PARAM_T Param;
	pthread_t video[3];
	pthread_t draw[2];
	video_info videoInfo[3];
    D_INT32 index;
    D_INT32 ret;
	D_UINT8 *pID;
	WSD_AL_PLAY_SOURCE_T PlaySource;
	WSD_AL_PLAY_PARAM_T sParam;
	WSD_PLAT_INIT_PARAM_T InitParam;

	char cmd;
    printf("******************************\n");
    printf("***   欢迎使用厂测程序    ****\n");
    printf("******************************\n");

	ret = WSD_DBG_Init();
	if (D_SUCCESS != ret)
	{
		return D_FAILURE;
	}

	WSD_AL_PLT_GetDefInitParam(&InitParam);
	InitParam.OutFormat[WSD_PORT_HDMI_E] = WSD_FORMAT_1080P_E;
	InitParam.pSysNotify = WSD_HDMI_CALLBACK;
	InitParam.UHDEnable = D_TRUE;
	ret = WSD_AL_PLT_Init(&InitParam);
	if (D_SUCCESS != ret)
	{
		return D_FAILURE;
	}

	printf("WSD_AL_IO_Init\n");
	ret = WSD_AL_IO_Init();
	if (D_SUCCESS != ret)
	{
		//return D_FAILURE;
	}

	WSD_AL_GFX_SetUseOpenglES2(1);

	printf("WSD_AL_GFX_Init\n");
	ret = WSD_AL_GFX_Init();
	if (D_SUCCESS != ret)
	{
		return D_FAILURE;
	}

	printf("WSD_AL_RECORD_Init\n");
	ret = WSD_AL_RECORD_Init();
	if (D_SUCCESS != ret)
	{
		return D_FAILURE;
	}

	printf("WSD_AL_GUIDE_Init\n");
	ret = WSD_AL_GUIDE_Init();
	if (D_SUCCESS != ret)
	{
		return D_FAILURE;
	}

	printf("WSD_AL_I2C_Init\n");
	ret = WSD_AL_I2C_Init();
	if (D_SUCCESS != ret)
	{
		return D_FAILURE;
	}

	printf("WSD_AL_OSD_Init\n");
	ret = WSD_AL_OSD_Init(32);
	if (NULL == ret)
	{
		return D_FAILURE;
	}

	printf("WSD_AL_PLAYER_Init\n");
	ret = WSD_AL_PLAYER_Init();
	if (D_SUCCESS != ret)
	{
		return D_FAILURE;
	}

	WSD_AL_AE_Output5bandPeqEnable(WSD_AE_REAR_OUTPUT, 0);
	WSD_AL_AE_OutputSetup(WSD_AE_REAR_OUTPUT, 0x7f, 0xff, 0xff);
	WSD_AL_AE_MixerMixSetup(WSD_AE_REAR_OUTPUT, 0xff, 0xff, 0xff, 0x7f);

	WSD_AL_AE_Output5bandPeqEnable(WSD_AE_FRONT_OUTPUT, 0);
	WSD_AL_AE_OutputSetup(WSD_AE_FRONT_OUTPUT, 0x7f, 0xff, 0xff);
	WSD_AL_AE_MixerMixSetup(WSD_AE_FRONT_OUTPUT, 0xff, 0xff, 0xff, 0x7f);

	printf("WSD_AL_CLOUD_Init\n");
	WSD_AL_CLOUD_Init("VINAMUSIC_7252S_2GDDR_4GEMMC", OnLoginExpiredCallback, NULL);

#ifndef TEST_FOR_ENCODER
	/* VGA OpenGL Drawing Thread */
	pthread_create(&draw[0], NULL,  draw_thread, NULL);

	/* OSD OpenGL Drawing Thread */
	pthread_create(&draw[1], NULL,  osd_draw, NULL);
#endif

	printf("WSD_AL_AUDIO_SetVolume\n");
	WSD_AL_AUDIO_SetVolume(400);

	sParam.Cycle 		= 1;
	sParam.Sync 		= 1;
	sParam.MuitleChannel= 1;
	sParam.PassThrough 	= 0;

#ifndef TEST_FOR_ENCODER
	printf("play main\n");
	memset(&PlaySource, 0x00, sizeof(PlaySource));
	WSD_AL_PLAYER_ProbeFile((D_UINT8*)argv[1], &(PlaySource.MediaParam));
	PlaySource.PlayMode = FILE_STREAM;
	strncpy((char*)PlaySource.SourceInfo.File.FilePath, argv[1],URL_LEN-1);
#else
	printf("play hdmi\n");
	WSD_AL_HDMIIN_Init();
	memset(&PlaySource, 0x00, sizeof(PlaySource));
	PlaySource.PlayMode = HDMIIN_STREAM;
#endif
	WSD_AL_PLAYER_SetSource(PLAYER_MAIN, &PlaySource);
	WSD_AL_PLAYER_SetPlayWindow(PLAYER_MAIN, 0, 0, 640, 360);
	WSD_AL_PLAYER_ShowPlayWindow(PLAYER_MAIN, 1);
	WSD_AL_PLAYER_SetParam(PLAYER_MAIN, &sParam);
	WSD_AL_PLAYER_Play(PLAYER_MAIN);
	WSD_AL_AUDIO_SetTrack(WSD_AL_AUDIO_TRACK_STEREO_E);

#ifndef TEST_FOR_ENCODER
	printf("play PIP\n");
	memset(&PlaySource, 0x00, sizeof(PlaySource));
	WSD_AL_PLAYER_ProbeFile((D_UINT8*)argv[2], &(PlaySource.MediaParam));
	PlaySource.PlayMode = FILE_STREAM;
	strncpy((char*)PlaySource.SourceInfo.File.FilePath, argv[2], URL_LEN-1);
	WSD_AL_PLAYER_SetSource(PLAYER_PIP, &PlaySource);
	WSD_AL_PLAYER_SetPlayWindow(PLAYER_PIP, 640, 0, 640, 360);
	WSD_AL_PLAYER_ShowPlayWindow(PLAYER_PIP, 1);
	WSD_AL_PLAYER_SetParam(PLAYER_PIP, &sParam);
	WSD_AL_PLAYER_Play(PLAYER_PIP);
#endif

#ifndef TEST_FOR_ENCODER
	printf("play BACK\n");
	memset(&PlaySource, 0x00, sizeof(PlaySource));
	WSD_AL_PLAYER_ProbeFile((D_UINT8*)argv[3], &(PlaySource.MediaParam));
	PlaySource.PlayMode = FILE_STREAM;
	strncpy((char*)PlaySource.SourceInfo.File.FilePath, argv[3], URL_LEN-1);
	WSD_AL_PLAYER_SetSource(PLAYER_BACK, &PlaySource);
	WSD_AL_PLAYER_SetPlayWindow(PLAYER_BACK, 960, 0, 640, 480);
	WSD_AL_PLAYER_ShowPlayWindow(PLAYER_BACK, 1);
	WSD_AL_PLAYER_SetParam(PLAYER_BACK, &sParam);
	WSD_AL_PLAYER_Play(PLAYER_BACK);
#endif

#ifndef TEST_FOR_ENCODER
	printf("show camera .....\n");
//	getchar();

	ret = WSD_AL_CLOUD_TestServerAvailble();
	printf("WSD_AL_CLOUD_TestServerAvailble return %d\n", ret);

	printf("play Camera\n");
	if (WSD_AL_CAM_StartPlayAndEncoder(
		"/dev/video0",
		640,
		480,
		30,
		1,
		CameraYUV422Callbak,
		CameraARGB32Callbak,
		0) == D_SUCCESS)
	{
		int min, max, value, defvalue;

		int autoWB;
		ret = WSD_AL_CAM_GetAutoWhiteBalance(
			&autoWB,
			&defvalue);
		printf("whitebalance: auto=%d, def=%d, ret=0x%08x\n",
			autoWB, defvalue, ret);
		ret = WSD_AL_CAM_GetWhiteBalance(
			&min,
			&max,
			&value,
			&defvalue);
		printf("whitebalance: min=%d, max=%d, value=%d, def=%d, ret=0x%08x\n",
			min, max, value, defvalue, ret);
		WSD_AL_CAM_SetAutoWhiteBalance(0);
		WSD_AL_CAM_SetWhiteBalance(defvalue);

		int autoFocus;
		ret = WSD_AL_CAM_GetAutoFocus(
			&autoFocus,
			&defvalue);
		printf("focus: auto=%d, def=%d, ret=0x%08x\n",
			autoFocus, defvalue, ret);
		ret = WSD_AL_CAM_GetFocus(
			&min,
			&max,
			&value,
			&defvalue);
		printf("focus: min=%d, max=%d, value=%d, def=%d, ret=0x%08x\n",
			min, max, value, defvalue, ret);
		WSD_AL_CAM_SetAutoFocus(0);
		WSD_AL_CAM_SetFocus(defvalue);

		int autoExp;
		ret = WSD_AL_CAM_GetAutoExposure(
			&autoExp,
			&defvalue);
		printf("exposure: auto=%d, def=%d, ret=0x%08x\n",
			autoExp, defvalue, ret);
		ret = WSD_AL_CAM_GetExposure(
			&min,
			&max,
			&value,
			&defvalue);
		printf("exposure: min=%d, max=%d, value=%d, def=%d, ret=0x%08x\n",
			min, max, value, defvalue, ret);
		WSD_AL_CAM_SetAutoExposure(0);
		WSD_AL_CAM_SetExposure(defvalue);

		int autoGain;
		ret = WSD_AL_CAM_GetAutoGain(
			&autoGain,
			&defvalue);
		printf("gain: auto=%d, def=%d, ret=0x%08x\n",
			autoGain, defvalue, ret);
		ret = WSD_AL_CAM_GetGain(
			&min,
			&max,
			&value,
			&defvalue);
		printf("gain: min=%d, max=%d, value=%d, def=%d, ret=0x%08x\n",
			min, max, value, defvalue, ret);
		WSD_AL_CAM_SetGain(defvalue);

		D_RECT rcCameraUI = {1440, 0, 1920, 360};//480*360
		WSD_AL_OSD_SetCameraRect(rcCameraUI);
		WSD_AL_OSD_ShowCamera(D_TRUE);

		printf("press key to start color key .....\n");
		getchar();
		afterBasckgroundCut();
		gCameraColorKeyEnable = 0;
		int nCamWidth = 320, nCamHeight = 240;
		WSD_AL_CAM_GetDevResolution(&nCamWidth, &nCamHeight);
		printf("WSD_AL_CAM_InitColorKey %dx%d .....\n", nCamWidth, nCamHeight);
		WSD_AL_CAM_InitColorKey(nCamWidth, nCamHeight);
		WSD_AL_CAM_EnableColorKey(D_TRUE);
		WSD_AL_CAM_ColorKeyLearning(
			60,
			OnCameraColorKeyLearnDoneCallback,
			0);

		while (1)
		{
			if (gCameraColorKeyEnable)
			{
				break;
			}
			printf("wait for camera key .....\n");
			usleep(1000*1000);
		}
	}
#endif

	D_INT32 nUsbAudioDevCount = 0;
	WSD_AL_USB_AUDIO_DEV_T* pUsbAudioDevList = NULL;
	if (WSD_AL_USB_RECORD_FindCard(NULL, &nUsbAudioDevCount) == D_SUCCESS)
	{
		printf("WSD_AL_USB_RECORD_FindCard, nUsbAudioDevCount=%d\n", nUsbAudioDevCount);
		pUsbAudioDevList =(WSD_AL_USB_AUDIO_DEV_T*) malloc(sizeof(WSD_AL_USB_AUDIO_DEV_T)*nUsbAudioDevCount);
		if (pUsbAudioDevList)
		{
			WSD_AL_USB_RECORD_FindCard(pUsbAudioDevList, &nUsbAudioDevCount);
			for (int i = 0; i < nUsbAudioDevCount; i++)
			{
				D_INT32 nUsbAudioDevCaptureCount = 0;
				WSD_AL_USB_AUDIO_PCM_T* pUsbAudioDevCaptureList = NULL;
				if (WSD_AL_USB_RECORD_FindCapturePCM(pUsbAudioDevList[i].card, NULL, &nUsbAudioDevCaptureCount) == D_SUCCESS)
				{
					printf("WSD_AL_USB_RECORD_FindCapturePCM, card=%d, id=%s, name=%s, nUsbAudioDevCaptureCount=%d\n",
						pUsbAudioDevList[i].card,
						pUsbAudioDevList[i].id,
						pUsbAudioDevList[i].name,
						nUsbAudioDevCaptureCount);
					pUsbAudioDevCaptureList =(WSD_AL_USB_AUDIO_PCM_T*) malloc(sizeof(WSD_AL_USB_AUDIO_PCM_T)*nUsbAudioDevCaptureCount);
					if (pUsbAudioDevCaptureList)
					{
						WSD_AL_USB_RECORD_FindCapturePCM(
							pUsbAudioDevList[i].card,
							pUsbAudioDevCaptureList,
							&nUsbAudioDevCaptureCount);
						for (int j = 0; j < nUsbAudioDevCaptureCount; j++)
						{
							printf("CapturePCM, card=%d, captureIndex=%d, name=%s, desc=%s;\n",
								pUsbAudioDevCaptureList[j].card,
								pUsbAudioDevCaptureList[j].index,
								pUsbAudioDevCaptureList[j].name,
								pUsbAudioDevCaptureList[j].desc);
						}
						free(pUsbAudioDevCaptureList);
					}
				}
			}
			free(pUsbAudioDevList);
		}
	}

	WSD_AL_USB_RECORD_HANDLE pUsbRecHandle = NULL;
	WSD_AL_USB_REC_DEV_T usbRecDev;
	usbRecDev.card = 0;
	strcpy(usbRecDev.pcmName, "");
	usbRecDev.channels = 2;
	usbRecDev.sampleRate = 16000;
	WSD_AL_USB_RECORD_Init(&pUsbRecHandle, &usbRecDev);	

    printf("WSD_AL_RECORD_Start\n");
    WSD_AL_RECORD_Start(WSD_AL_REC_LINE_IN_E, D_FALSE, NULL, NULL);

	//	
	WSD_AL_SINGER_INIT_PARAM_T singerParam;
	strcpy(singerParam.CacheDir, "/tmp");
	singerParam.Callback = OnSingerNotificationCallback;
	singerParam.pParam = NULL;
	singerParam.SampleRate =16000;
	singerParam.VoiceChannel = WSD_AL_SINGER_VOICE_ON_STEREO_E;
	singerParam.PitchErrorThreshold = WSD_AL_SINGER_PITCH_ERROR_THRESHOLD;
	singerParam.MicSilentThreshold = 50;
	WSD_AL_SINGER_Init(&singerParam);
	WSD_AL_SINGER_Set_UseUSBScore(D_TRUE);

	WSD_AL_SINGER_SetSource("60021563");
	WSD_AL_SINGER_Prepare();
	WSD_AL_SINGER_Start();
	WSD_AL_USB_RECORD_Start(pUsbRecHandle, D_TRUE, NULL, NULL);

	printf("press key to recorder .....\n");
	getchar();

	WSD_AL_CLOUD_USER_DETAIL_T user={0};
	strcpy(user.name, "test 7"); // nick name
	strcpy(user.phone, "12345678901"); // phone number
	strcpy(user.birthday, "1980-1-1"); // birthday
	user.gender = 'F';// F-Femail, M-mail, N-unknown :)

	// if you didn't have account, register first
	WSD_AL_CLOUD_User_SignUp("Test", "1234", &user);
	
	// then, sign in this account
	WSD_AL_CLOUD_User_SignIn(WSD_AL_CLOUD_USER_SIGNIN_BY_ACCOUNT_E, "Test", "1234", &user);

	WSD_AL_AE_OutputSubwooferCompressorEnable(0);
	WSD_AL_AE_Output5bandPeqEnable(WSD_AE_SUBWOOFER_OUTPUT, 0);
	WSD_AL_AE_OutputSetup(WSD_AE_SUBWOOFER_OUTPUT, 0x7f, 0xff, 0xff);
	WSD_AL_AE_MixerMixSetup(WSD_AE_SUBWOOFER_OUTPUT, 0xff, 0xff, 0xff, 0x7f);

#if 0
    printf("WSD_AL_AUDIOENC_Start\n");
	WSD_AL_AUDIOENC_Start(
		OnUploadProgressCallback,
		NULL,
		1,
		"测试歌曲",
		"测试歌星");
#else
    printf("WSD_AL_VIDEOENC_Start\n");
	D_RECT rcCamera = {160, 0, 1120, 720};
    D_RECT rcOsdFrom = {960, 0, 1920, 540};
    D_RECT rcOsdTo = {640, 0, 1280, 360};
	WSD_AL_VIDEOENC_Start(
		ENCODER_QUALITYTYPE_NORMAL,
		1280,
		720,
		30,
		1,
		PLAYER_MAIN,
		0,
		rcCamera,
		0,
		rcOsdFrom,
		rcOsdTo,
		1,// enable upload
		OnUploadProgressCallback,
		NULL,
		0,// no transcode
		"测试歌曲",
		"测试歌星",
		0,// enable live broadcast
		NULL,
		NULL,
		"flv",
		"rtmp://send1.douyu.com/live/2759744rO1G6wexV?wsSecret=bf7215e3b3d09a7087223f463e10b6d5&wsTime=59e82516&wsSeek=off");
		//"rtmp://video-center.alivecdn.com/kyo/1?vhost=push-sh1.whiteskycn.com"
		//"rtmp://send1.douyu.com/live/2759744rO1MMmMIK?wsSecret=dbab546b2071e3267bda77da49ec9dce&wsTime=598c1c79&wsSeek=off"
#endif

    printf("please input enter key to exit.....\n");
	getchar();

//	WSD_AL_SINGER_Stop();

//	WSD_AL_SINGER_DeInit();

	int nMediaID=0;
    char cCloudURL[1024]={0};
    char cVodURL[1024]={0};
	WSD_AL_VIDEOENC_Stop(&nMediaID);
	if (nMediaID > 0)
	{
		usleep(1000*1000);
		WSD_AL_CLOUD_GetMediaUrl(nMediaID, 0, cCloudURL, 1024, cVodURL, 1024);
		printf("video cloud ID=%d, url is : %s, vodurl=%s\n", nMediaID, cCloudURL, cVodURL);
	}

	nMediaID = 0;
	WSD_AL_AUDIOENC_Stop(&nMediaID);
	if (nMediaID > 0)
	{
		usleep(1000*1000);
		WSD_AL_CLOUD_GetMediaUrl(nMediaID, 1, cCloudURL, 1024, cVodURL, 1024);
		printf("audio cloud ID=%d, url is : %s\n", nMediaID, cCloudURL);
	}

	WSD_AL_CAM_StopPlayAndEncoder();
	WSD_AL_CAM_DeInitColorKey();

	WSD_AL_CLOUD_User_SignOut();
	WSD_AL_CLOUD_DeInit();

    return 0;
}


/***********************************************************************
*                                局部函数实现
***********************************************************************/


