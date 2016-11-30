#pragma once
#include <GL/glew.h>
#include <GL/GLU.h>
#include <GLFW/glfw3.h>
class texLoader
{
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int width, height;
	unsigned int imageSize;
	unsigned char * data;

public:
	texLoader();
	~texLoader();
	GLuint loadBMP(const char * path);

};

