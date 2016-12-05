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
#include "Mob.h"
#include "Tile.h"
#include "GLWindow.h"

int width = 1280, height = 720;
const int tileSize = 64;
const int playerAcceleration = 40, playerDeceleration = 60, playerMaxRunSpeed = 600, playerJumpSpeed = 50, maxFallSpeed = 400, gravity = 10;
bool leftPressed = false, rightPressed = false, spacePressed = false, grounded = false;
GLWindow *window;
Sprite *level;
Mob *player;
vector<Object*> floorTiles(width / tileSize);

GLuint TEX_TITLE;
Atlas TILES, PLAYER_ANIM;

void keyCallback(GLFWwindow* fwwindow, int key, int scancode, int action, int mode) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_LEFT:
			leftPressed = true;
			break;
		case GLFW_KEY_RIGHT:
			rightPressed = true;
			break;
		case GLFW_KEY_SPACE:
			spacePressed = true;
			break;
		case GLFW_KEY_ESCAPE:
			window->stop();
		}
	}
	else if (action == GLFW_RELEASE) {
		switch (key) {
		case GLFW_KEY_LEFT:
			leftPressed = false;
			break;
		case GLFW_KEY_RIGHT:
			rightPressed = false;
			break;
		case GLFW_KEY_SPACE:
			spacePressed = false;
			break;
		}
	}
}

void background() {
	
}

void update() {
	player->accelerate(vector2i(0, gravity), vector2i(0, maxFallSpeed));
	if (leftPressed)
		player->accelerate(vector2i(-playerAcceleration, 0), vector2i(-playerMaxRunSpeed, 0));
	if (rightPressed)
		player->accelerate(vector2i(playerAcceleration, 0), vector2i(playerMaxRunSpeed, 0));
	else if (!leftPressed && abs(player->getSpeed().x) < playerDeceleration)
		player->setSpeed(vector2f(0, player->getSpeed().y));
	else if(!leftPressed && abs(player->getSpeed().x) > 0)
		player->accelerate(vector2i(playerDeceleration * -(abs(player->getSpeed().x) / player->getSpeed().x), 0), vector2i(0, 0));

	if (player->simpleCollision(floorTiles) && !grounded)
		grounded = true;

	if (spacePressed) {
		player->accelerate(vector2i(0, -playerJumpSpeed), vector2i(0, -500));
		grounded = false;
	}
}

int main(void) {
	//Setup OpenGL
	GLFWwindow *glwin = NULL;
	Object *camera;
	window = new GLWindow(&glwin, "HAIL", width, height);
	glfwSetWindowPos(window->window, 5, 32);
	glfwGetWindowSize(window->window, &width, &height);
	window->setKeyCallback(keyCallback);
	window->setLoopCall(update);
	window->clearColor = vector3f(0.1f, 0.1f, 0.1f);

	TILES = Atlas(window, "assets/Tiles.png", vector2i(16, 16));
	PLAYER_ANIM = Atlas(window, "assets/player.png", vector2i(32, 32), 1);
	TEX_TITLE = window->createTexture("assets/Title.png");

	player = Mob::createMob(window, width / 2, height / 2, 64, 64, &PLAYER_ANIM, 0);
	window->enableUpdate(player);
	for (int i = 0; i < width / tileSize; i++) {
		floorTiles[i] = window->addSprite(i * tileSize, 600, tileSize, tileSize, &TILES, 0, 0.1f);
		window->addSprite(i * tileSize, 600 + tileSize, tileSize, tileSize, &TILES, 2, 0.1f);
	}
	level = window->addSprite(0, 0, width, height, TEX_TITLE);
	
	camera = window->addEmpty(0, 0, 0, 0);
	window->setCamera(camera);
	//system("pause");
	window->start();

	return 0;
}

