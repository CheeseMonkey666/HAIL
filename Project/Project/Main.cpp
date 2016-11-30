
#include <GL/glew.h>
#include <GL/GLU.h>
#include <GLFW/glfw3.h>
#include "quaternion.h"
#include <stdio.h>
#include <windows.h>
#include <ctime>

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include "Utils.h"

double x = 0, z = -3, y = 0, xRot = 0, yRot = 0;
bool mforward = false, mback = false, mleft = false, mright = false, mup = false, mdown = false, printedVerts = false;
GLuint vertexBuffer = 0, normalBuffer = 0, colorBuffer = 0, uvBuffer = 0, vao = 0;
int vertexAttribIndex = 0, colorAttribIndex = 1, vertBufferSize = 0, colorBufferSize = 0;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_W) {
			mforward = true;
		} 
		if (key == GLFW_KEY_S) {
			mback = true;
		}
		if (key == GLFW_KEY_A) {
			mleft = true;
		}
		if (key == GLFW_KEY_D) {
			mright = true;
		}
		if (key == GLFW_KEY_SPACE) {
			mup = true;
		}
		if (key == GLFW_KEY_C) {
			mdown = true;
		}
	}
	else if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W) {
			mforward = false;
		}
		if (key == GLFW_KEY_S) {
			mback = false;
		}
		if (key == GLFW_KEY_A) {
			mleft = false;
		}
		if (key == GLFW_KEY_D) {
			mright = false;
		}
		if (key == GLFW_KEY_SPACE) {
			mup = false;
		}
		if (key == GLFW_KEY_C) {
			mdown = false;
		}
	}
}

int main(void) {
	GLFWwindow* window = NULL;

	if (!glfwInit())
		return -1;

	window = glfwCreateWindow(1024, 768, "Hello World", NULL, NULL);

	if (!window) {
		glfwTerminate();
		return -1;
	}

	glClearColor(0.0F, 0.0F, 0.0F, 1);

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK)
		exit(EXIT_FAILURE);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	glfwSetKeyCallback(window, key_callback);


	//glEnable(GL_CULL_FACE);

	
	GLuint vertShader = 0, fragShader = 0, program = 0;
	createShader("vertex.glsl", GL_VERTEX_SHADER, vertShader);
	createShader("fragment.glsl", GL_FRAGMENT_SHADER, fragShader);
	program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	// insert location binding code here

	glLinkProgram(program);

	printShaderInfoLog(vertShader);
	printShaderInfoLog(fragShader);
	printProgramInfoLog(program);

	glReleaseShaderCompiler();

	
	/*if (!loadOBJ("model.obj", vertexBuffer, uvBuffer, normalBuffer, vertBufferSize)) {
		printf("Something went wrong with loading\n");
	}
	
	generateColorsf(colorBuffer);*/
	colorBufferSize = vertBufferSize = 108;
	loadTestCube(vertexBuffer, colorBuffer);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glEnableVertexAttribArray(vertexAttribIndex);
	glVertexAttribPointer(vertexAttribIndex, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glEnableVertexAttribArray(colorAttribIndex);
	glVertexAttribPointer(colorAttribIndex, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	double mouseSensitivity = 0.0015, xCap = M_PI / 2;
	time_t startTime = 0, endTime = 0;
	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		Sleep(1000 / 60 - (endTime - startTime));
		startTime = time(0);
		
		glClearColor(0.0F, 0.0F, 0.0F, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glUseProgram(program);
		glLoadIdentity();

		int width, height;
		double xpos, ypos, dx, dy;
		double moveSpeed = 0.2;
		glfwGetWindowSize(window, &width, &height);
		glfwGetCursorPos(window, &xpos, &ypos);
		glfwSetCursorPos(window, width / 2, height / 2);

		glViewport(0, 0, width, height);
		glOrtho(0, 1, 1, 0, -1, 1);

		glLoadIdentity();

		dx = double((width / 2 - xpos)) * mouseSensitivity;
		dy = double((height / 2 - ypos)) * mouseSensitivity;
		xRot -= dy;
		yRot -= dx;

		if (mforward) {
			x += sin(yRot) * moveSpeed;
			z += cos(yRot) * moveSpeed;
		}
		if (mback) {
			x -= sin(yRot) * moveSpeed;
			z -= cos(yRot) * moveSpeed;
		}
		if (mleft) {
			x -= sin(yRot + M_PI / 2) * moveSpeed;
			z -= cos(yRot + M_PI / 2) * moveSpeed;
		}
		if (mright) {
			x += sin(yRot + M_PI / 2) * moveSpeed;
			z += cos(yRot + M_PI / 2) * moveSpeed;
		}
		if (mup) {
			y -= moveSpeed;
		}
		if (mdown) {
			y += moveSpeed;
		}

		gluPerspective(70.0, width / height, 1.0, 30.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		if (xRot > xCap) {
			xRot = xCap;
		}
		else if (xRot < -xCap) {
			xRot = -xCap;
		}

		vector3d dir, right, up;
		dir.x = cos(xRot) * sin(yRot);
		dir.y = sin(xRot);
		dir.z = cos(xRot) * cos(yRot);
		right.x = sin(yRot - M_PI / 2);
		right.y = 0;
		right.z = cos(yRot - M_PI / 2);
		up.x = dir.y * right.z - dir.z * right.y;
		up.y = dir.z * right.x - dir.x * right.z;
		up.z = dir.x * right.y - dir.y * right.x;

		gluLookAt(x, y, z, x + dir.x, y + dir.y, z + dir.z, up.x, up.y, up.z);

		glBindVertexArray(vao);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glDrawArrays(GL_TRIANGLES, 0, vertBufferSize * 2);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glBindVertexArray(0);

		//http://imgur.com/a/9q0NF
		glFlush();
		glfwSwapBuffers(window);
		glfwPollEvents();

		endTime = time(0);
		//std::cout << "Frame delta: " << endTime - startTime << "\n";
	}

	glfwTerminate();
	return 0;
}
