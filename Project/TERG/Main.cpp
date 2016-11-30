#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include <GL/glew.h>
#include <GL/GLU.h>
#include <GLFW/glfw3.h>
#include <windows.h>
#include <ctime>
#include <math.h>
#include <iostream>
#include <SOIL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Object.h"
#include "Tile.h"
#include "GLWindow.h"

int width = 1280, height = 720, tileSize = 16;
GLWindow *window;
Sprite *level;
TileMap *tileMap;

GLuint TEX_TITLE;
Atlas TILES;

void keyCallback(GLFWwindow* fwwindow, int key, int scancode, int action, int mode) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_A:
			
			break;
		case GLFW_KEY_D:
			
			break;
		case GLFW_KEY_SPACE:
			
			break;
		case GLFW_KEY_ESCAPE:
			window->stop();
		}
	}
	else if (action == GLFW_RELEASE) {
		switch (key) {
		case GLFW_KEY_A:
			
			break;
		case GLFW_KEY_D:
			
			break;
		case GLFW_KEY_SPACE:
			
			break;
		}
	}
}

void background() {
	
}

void update() {
	
}

int main(void) {
	//Setup OpenGL
	GLFWwindow *glwin = NULL;
	window = new GLWindow(&glwin, "HAIL", width, height);
	glfwSetWindowPos(window->window, 5, 32);
	glfwGetWindowSize(window->window, &width, &height);
	window->setKeyCallback(keyCallback);
	window->clearColor = vector3f(0.1f, 0.1f, 0.1f);

	TILES = Atlas(window, "assets/Tiles.png", vector2i(tileSize, tileSize));
	TEX_TITLE = window->createTexture("assets/Title.png");

	tileMap = window->addTileMap(0, 0, width, height, tileSize, &TILES);
	for (int i = 0; i < width / tileSize; i++) {
		tileMap->addTile(i, 40, &TILES, 0);
	}
	level = window->addSprite(0, 0, width, height, TEX_TITLE, 0.1f);
	while (true) {}
	window->start();

	return 0;
}

