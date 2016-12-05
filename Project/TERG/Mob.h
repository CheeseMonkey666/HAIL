#pragma once
#include "GLWindow.h"
#include "Object.h"
class Mob :
	public Sprite
{
protected:
	vector2f speed;
public:
	int spriteIndex;
	vector<Axis> blockedAxes;
	vector<Object*> collidable;

	Mob(int x, int y, int width, int height, Sprite *sprite);
	~Mob();

	void update();
	vector2f getSpeed();
	void setSpeed(vector2f spd);
	void setSpeed(int spd, Axis a);
	void accelerate(vector2i force, vector2i max = vector2i(0, 0));
	vector2f getNextPos();
	

	static Mob *createMob(GLWindow *window, int x, int y, int width, int height, const char *texturePath);
	static Mob *createMob(GLWindow *window, int x, int y, int width, int height, Atlas *atlas, int tileNum);
};

