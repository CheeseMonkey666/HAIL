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

enum PlayerAnimationState {PLAYER_IDLE, PLAYER_RUN, PLAYER_JUMP, PLAYER_FALL, PLAYER_ATTACK};

int width = 1280, height = 720;
const int tileSize = 32;
const int playerAcceleration = 2000, playerDeceleration = 1600, playerMaxRunSpeed = 600, playerJumpSpeed = 800, maxFallSpeed = -1600, gravity = -1000;
bool leftPressed = false, rightPressed = false, spacePressed = false, attackPressed = false, grounded = false, playerFacing = true; //playerFacing true = right
GLWindow *window;
Sprite *level;
Mob *player, *attackPlayer;
PlayerAnimationState playerState = PLAYER_IDLE;
vector<Object*> floorTiles(width / tileSize);
vector<vector<unsigned int>> playerAnimations(5);

GLuint TEX_TITLE;
Atlas TILES, PLAYER_ANIM, PLAYER_ATTACK_ANIM;

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
		case GLFW_KEY_Z:
			attackPressed = true;
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
		case GLFW_KEY_Z:
			attackPressed = false;
			break;
		}
	}
}

void background() {
	
}

void update() {
	player->simpleCollision(floorTiles);
	if(!vectorContains(player->blockedAxes, NEG_Y))
		player->accelerate(vector2i(0, gravity), window->frameDelta, vector2i(0, maxFallSpeed));
	if (leftPressed)
		player->accelerate(vector2i(-playerAcceleration, 0), window->frameDelta, vector2i(-playerMaxRunSpeed, 0));
	if (rightPressed)
		player->accelerate(vector2i(playerAcceleration, 0), window->frameDelta, vector2i(playerMaxRunSpeed, 0));
	else if (!leftPressed && abs(player->getSpeed().x) < (float)playerDeceleration * window->frameDelta)
		player->setSpeed(vector2f(0, player->getSpeed().y));
	else if(!leftPressed && abs(player->getSpeed().x) > 0)
		player->accelerate(vector2i(playerDeceleration * -(abs(player->getSpeed().x) / player->getSpeed().x), 0), window->frameDelta, vector2i(0, 0));
	player->simpleCollision(floorTiles);
	if (!grounded)
		grounded = true;
	if (spacePressed && grounded) {
		player->setSpeed(playerJumpSpeed, Y);
		grounded = false;
	}
	if (player->x < 0)
		player->translateAbs(vector2f(-player->x, 0));
	else if (player->x > width - player->width)
		player->translateAbs(vector2f(width - player->x - player->width, 0));
	if (player->y < 0)
		player->translateAbs(vector2f(0, -player->y));

	if (player->getSpeed().y < 0 && playerState != PLAYER_FALL) {
		playerState = PLAYER_FALL;
		player->setAnimation(&PLAYER_ANIM, playerAnimations[playerState], 16);
	}
	else if (player->getSpeed().y > 0 && playerState != PLAYER_JUMP) {
		playerState = PLAYER_JUMP;
		player->setAnimation(&PLAYER_ANIM, playerAnimations[playerState], 20);
	}
	if (player->getSpeed().x != 0 && playerState != PLAYER_RUN && grounded) {
		playerState = PLAYER_RUN;
		player->setAnimation(&PLAYER_ANIM, playerAnimations[playerState], 20);
	}
	else if (player->getSpeed().x == 0 && playerState != PLAYER_IDLE && grounded) {
		playerState = PLAYER_IDLE;
		player->setAnimation(&PLAYER_ANIM, playerAnimations[playerState], 2);
	}

	if (player->getSpeed().x < 0 && playerFacing) {
		playerFacing = false;
		window->flipSpriteTexture(player);
	}
	else if (player->getSpeed().x > 0 && !playerFacing) {
		playerFacing = true;
		window->flipSpriteTexture(player);
	}

	player->animate();
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
	//window->debugMode = true;

	TILES = Atlas(window, "assets/Tiles.png", vector2i(16, 16));
	PLAYER_ANIM = Atlas(window, "assets/player.png", vector2i(32, 32), 1);
	PLAYER_ATTACK_ANIM = Atlas(window, "assets/PlayerAttack.png", vector2i(72, 70), 1);
	TEX_TITLE = window->createTexture("assets/Title.png");

	playerAnimations[PLAYER_IDLE] = { 0, 1, 2 };
	playerAnimations[PLAYER_RUN] = { 8, 9, 10, 11, 12 };
	playerAnimations[PLAYER_JUMP] = { 16, 17, 18, 19, 20 };
	playerAnimations[PLAYER_FALL] = { 3, 4, 5 };
	playerAnimations[PLAYER_ATTACK] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	player = Mob::createMob(window, width / 2, height / 2, 64, 64, &PLAYER_ANIM, 0);
	window->enableUpdate(player);
	player->enableAnimation();
	player->setAnimation(&PLAYER_ANIM, playerAnimations[PLAYER_IDLE], 2);
	for (int i = 0; i < width / tileSize; i++) {
		floorTiles[i] = window->addSprite(i * tileSize, height - tileSize * 3, tileSize, tileSize, &TILES, 0, 0.1f);
		window->addSprite(i * tileSize, height - tileSize * 2, tileSize, tileSize, &TILES, 2, 0.1f);
		window->addSprite(i * tileSize, height - tileSize, tileSize, tileSize, &TILES, 2, 0.1f);
	}
	level = window->addSprite(0, 0, width, height, TEX_TITLE);
	
	camera = window->addEmpty(0, 0, 0, 0);
	window->setCamera(camera);
	//system("pause");
	window->start();

	return 0;
}

