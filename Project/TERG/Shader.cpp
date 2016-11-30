#include "Shader.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

const GLchar *getShaderCode(const char *path) {
	char *code;
	FILE * file;
	fopen_s(&file, path, "rb");
	if (file == NULL) {
		printf("No file found\n");
		return false;
	}
	fseek(file, 0, SEEK_END);
	GLint fsize = ftell(file);
	fseek(file, 0, SEEK_SET);
	code = (char *)malloc(fsize + 1);
	fread(code, fsize, 1, file);
	fclose(file);
	code[fsize] = NULL;

	return code;
}

void printShaderInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n", infoLog);
		free(infoLog);
	}
}

void printProgramInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n", infoLog);
		free(infoLog);
	}
}

Shader::Shader(const char *vertexPath, const char *fragmentPath)
{
	GLuint vertex, fragment;
	const GLchar *vCode = getShaderCode(vertexPath), *fCode = getShaderCode(fragmentPath);

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vCode, NULL);
	glCompileShader(vertex);
	printShaderInfoLog(vertex);

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fCode, NULL);
	glCompileShader(fragment);
	printShaderInfoLog(fragment);

	program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	printProgramInfoLog(program);

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::use() const{
	glUseProgram(program);
}