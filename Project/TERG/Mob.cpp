#include "Mob.h"
#include "Object.h"


Mob::Mob(int x, int y, int width, int height, Sprite *sprite)
	:Sprite(*sprite)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	spriteIndex = -1;
	speed = vector2f(0, 0);
}


Mob::~Mob()
{
}

Mob *Mob::createMob(GLWindow *window, int x, int y, int width, int height, const char *texturePath) {
	vector<GLfloat> v = window->getQuadVerts(x, y, width, height, new vector3f(1, 1, 1), true);
	vector<GLuint> in = { 0, 1, 2, 2, 3, 0 };
	Sprite *spr = window->addSprite(x, y, width, height, texturePath);
	spr->isStatic = false;
	Mob *m = new Mob(x, y, width, height, spr);
	vectorContains(window->objects, static_cast<Object*>(spr));
	window->objects[vectorContainsIndex] = m;
	m->spriteIndex = vectorContainsIndex;
	return m;
}

Mob *Mob::createMob(GLWindow *window, int x, int y, int width, int height, Atlas *atlas, int tileNum) {
	vector<GLfloat> v = window->getQuadVerts(x, y, width, height, new vector3f(1, 1, 1), true);
	vector<GLuint> in = { 0, 1, 2, 2, 3, 0 };
	Sprite *spr = window->addSprite(x, y, width, height, atlas, tileNum);
	spr->isStatic = false;
	Mob *m = new Mob(x, y, width, height, spr);
	vectorContains(window->objects, static_cast<Object*>(spr));
	window->objects[vectorContainsIndex] = m;
	m->spriteIndex = vectorContainsIndex;
	return m;
}

void Mob::update() {
	translate(vector2f(speed.x, speed.y));
}

vector2f Mob::getSpeed() {
	return speed;
}

void Mob::setSpeed(vector2f spd) {
	speed = resolveAxesBlock(spd);
}

void Mob::setSpeed(int spd, Axis a) {
	if (a == X) {
		speed.x = spd;
	}
	else if (a == Y){
		speed.y = spd;
	}
	speed = resolveAxesBlock(speed);
}

void Mob::accelerate(vector2i force, double frameDelta, vector2i max) {
	float sx = speed.x + (float)force.x * frameDelta, sy = speed.y + (float)force.y * frameDelta;
	if (sx < 0)
		if (max.x >= 0 || sx >= max.x)
			speed.x = sx;
		else
			speed.x = max.x;
	else
		if (max.x <= 0 || sx <= max.x)
			speed.x = sx;
		else
			speed.x = max.x;

	if (sy < 0)
		if (max.y >= 0 || sy >= max.y)
			speed.y = sy;
		else
			speed.y = max.y;
	else
		if (max.y <= 0 || sy <= max.y)
			speed.y = sy;
		else
			speed.y = max.y;

	speed = resolveAxesBlock(speed);
}

vector2f Mob::getNextPos() {
	return vector2f(x + speed.x, y + speed.y);
}