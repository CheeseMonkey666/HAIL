#include "texLoader.h"
#include <stdio.h>
#include <iostream>



texLoader::texLoader()
{

}


texLoader::~texLoader()
{
}

GLuint texLoader::loadBMP(const char * path) {
	
	FILE * file;
	fopen_s(&file, path, "rb");
	GLuint texID;
	if (!file)
		printf("File could not be opened\n");
	if (fread(header, 1, 54, file) != 54 || header[0] != 'B' || header[1] != 'M')
		printf("Invalid BMP file\n");
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	if (imageSize == 0)    
		imageSize = width*height * 3;
	if (dataPos == 0)     
		dataPos = 54;
	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	return texID;
}
