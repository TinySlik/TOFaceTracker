#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <GL/glew.h>
//#include "glad.h"
#include <SOIL.h>
#include <iostream>

class TextureHelper
{
public:
	/*
	/* �ɹ�����2D�����򷵻��������Id ���򷵻�0                                                                
	*/
	static  GLint load2DTexture(const char* filename, GLint internalFormat = GL_RGB,
		GLenum picFormat = GL_RGB, int loadChannels = SOIL_LOAD_RGB)
	{
		// Step1 ���������������
		GLuint textureId = 0;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		// Step2 �趨wrap����
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Step3 �趨filter����
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
			GL_LINEAR_MIPMAP_LINEAR); // ΪMipMap�趨filter����
		// Step4 ��������
		GLubyte *imageData = NULL;
		int picWidth, picHeight;
		imageData = SOIL_load_image(filename, &picWidth, &picHeight, 0, loadChannels);
		if (imageData == NULL)
		{
			std::cerr << "Error::Texture could not load texture file:" << filename << std::endl;
			return 0;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, picWidth, picHeight, 
			0, picFormat, GL_UNSIGNED_BYTE, imageData);
		glGenerateMipmap(GL_TEXTURE_2D);
		// Step5 �ͷ�����ͼƬ��Դ
		SOIL_free_image_data(imageData);
		glBindTexture(GL_TEXTURE_2D, 0);
		return textureId;
	}
};

#endif