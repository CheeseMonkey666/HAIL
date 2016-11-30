#ifndef SHADER_INCLUDE
#define SHADER_INCLUDE

#include <GL/glew.h>

class Shader
{
public:
	GLuint program;

	Shader(const char *vertexPath, const char *fragmentPath);

	void use() const;
};

#endif