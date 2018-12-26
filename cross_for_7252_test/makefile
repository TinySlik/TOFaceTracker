##################################################################################
#																										#
# MiddleWare Lib Build Makefile 																		#
#																										#
# Author: 																						#	
#																										#			
# Copyright (c) MiddleWare System 2008																#
#																										#	
# NOTE: 																									#
##################################################################################
#########################################################
# Define Project Root Directory												#
#########################################################




PRO_DIR=../..


MAJOR_VERSION := 1
MINOR_VERSION := 00
PATCH_VERSION := 001

export MAJOR_VERSION MINOR_VERSION PATCH_VERSION

NAME=libTO.so
SO_LIB_NAME=$(NAME).$(MAJOR_VERSION)
LIB_NAME=$(SO_LIB_NAME).$(MINOR_VERSION).$(PATCH_VERSION)



ifneq ($(PRO_DIR)/rules,$(wildcard $(PRO_DIR)/rules))
$(error Can't find rules dir. You probably set PRO_DIR incorrectly)
endif


#########################################################
#Define App Build Param													#
#########################################################
BUILD_APP = YES

ifeq ($(BUILD_APP),YES)

APP_NAME = test_cv
ifeq ($(APP_NAME),)
$(error Can't find APP_NAME. You probably not set APP_NAME)
endif

endif


#########################################################
#Define Lib Build Name														#
#########################################################
BUILD_LIB = NO
BUILD_FORCE_STRIP = YES

ifeq ($(BUILD_LIB),YES)

MODE_LIB_NAME=$(LIB_NAME)
ifeq ($(MODE_LIB_NAME),)
$(error Can't find MODE_LIB_NAME. You probably not set MODE_LIB_NAME)
endif

ifeq ($(lib_path),)
MODE_LIB_PATH = 
else
MODE_LIB_PATH = $(lib_path)
endif

endif


#########################################################
#Define Build Subdirectories													#
#########################################################
ifeq ($(drivers),)
SUB_DIRS =  
else
SUB_DIRS = $(drivers)
endif


#########################################################
#Define C Build Source Files							#
#########################################################
GENERAL_C_FILES = 	osd_cube_test.c \
                	esutil.c  \
                	vibe-background-sequential.c


#########################################################
#Define C++ Build Source Files												#
#########################################################
GENERAL_CPP_FILES = damd_main.cpp\
					opencv_test.cpp\
					TODynamiacBackgroundExtraction.cpp\
					TOFaceDecorator.cpp



#########################################################
#Define Exter Include File Directories											#
#########################################################
EXTRA_INCLUDE_DIRS += $(PRO_DIR)/app/include/OPENCV
EXTRA_INCLUDE_DIRS += $(PRO_DIR)/libs/TOFace/include
EXTRA_INCLUDE_DIRS += $(PRO_DIR)/app/libs/ne10/include

#########################################################
#Exter Object File Directories												#
#########################################################
EXTRA_OBJS = 


#########################################################
#Exter Libs File Directories													#
#########################################################
EXTRA_LIBS   = -L$(PRO_DIR_LIB)
EXTRA_LIBS += -lal
EXTRA_LIBS += -L$(PRO_DIR)/libs/nexus
EXTRA_LIBS += -lv3ddriver -lnxpl -lb_os -lb_playback_ip -lnexus -lstdc++ -lpthread -lm -lrt -lb_psip -ltshdrbuilder 
EXTRA_LIBS += -lliveMedia -lgroupsock -lUsageEnvironment -lBasicUsageEnvironment -lblive_ext 
EXTRA_LIBS += -L$(PRO_DIR)/libs/ssl/lib -lssl -lcrypto
EXTRA_LIBS += -L$(PRO_DIR)/libs/TOFace/lib -lTOFace
EXTRA_LIBS += -L$(PRO_DIR)/libs/jpeg/lib -ljpeg
EXTRA_LIBS += -L$(PRO_DIR)/libs/png/lib  -lpng
EXTRA_LIBS += -L$(PRO_DIR)/libs/zlib/lib -lz
EXTRA_LIBS += -L$(PRO_DIR)/libs/cJSON/lib -lcjson
EXTRA_LIBS += -L$(PRO_DIR)/libs/curl/lib -lcurl
EXTRA_LIBS += -L$(PRO_DIR)/libs/ne10/lib -lNE10
EXTRA_LIBS += -L$(PRO_DIR)/libs/librtmp/lib -lrtmp
EXTRA_LIBS += -L$(PRO_DIR)/libs/x264/lib -lx264
EXTRA_LIBS += -L$(PRO_DIR)/libs/ffmpeg/lib -lavcodec  -lavdevice  -lavformat -lavutil -lswresample -lavfilter -lswscale -lpostproc
EXTRA_LIBS += -L$(PRO_DIR)/libs/alsa/lib -lasound
EXTRA_LIBS += -L$(PRO_DIR)/libs/XKey/lib -lXKey
EXTRA_LIBS += -L$(PRO_DIR)/libs/opencv3.1_from_mr_wang_arm_gnu_cross_compline_11.20/lib -lopencv_core -lopencv_imgcodecs -lopencv_ml -lopencv_imgproc -lopencv_videoio -lopencv_videoio -lopencv_highgui  -lopencv_objdetect


#########################################################
# Include File Building Rules							#
#########################################################
include $(PRO_DIR)/rules/BuildMode.mak


#***********************************************************************************
#* $Log:
#* $
#***********************************************************************************
