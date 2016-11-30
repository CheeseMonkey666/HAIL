#pragma once
#include <vector>
#include <GL/glew.h>
#include <GL/GLU.h>
#include <GLFW/glfw3.h>

using namespace std;

struct vector2d {
	double x, y;
	vector2d(double vx = 0, double vy = 0) {
		x = vx;
		y = vy;
	};
};

struct vector3d {
	double x, y, z;
	vector3d(double vx = 0, double vy = 0, double vz = 0) {
		x = vx;
		y = vy;
		z = vz;
	};
};

struct vector2f {
	float x, y;
	vector2f(float vx = 0, float vy = 0) {
		x = vx;
		y = vy;
	};
};

struct vector3f {
	float x, y, z;
	vector3f(float vx = 0, float vy = 0, float vz = 0) {
		x = vx;
		y = vy;
		z = vz;
	};
};

bool loadOBJ(const char * path, GLuint & vertexBuffer, GLuint & uvBuffer, GLuint & normalBuffer, int &bufferSize);

void generateColorsf(GLuint & buffer);

bool createShader(const char * path, GLenum type, GLuint & shader);

void loadTestCube(GLuint &vertexBuffer, GLuint &colorBuffer);

void printShaderInfoLog(GLuint obj);

void printProgramInfoLog(GLuint obj);
