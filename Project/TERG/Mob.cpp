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
	translate(vector2f(speed.x, -speed.y));
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

void Mob::accelerate(vector2i force, vector2i max) {
	if (abs(speed.x + force.x) > abs(max.x) && max.x != 0)
		speed.x = max.x;
	else
		speed.x += force.x;
	if (speed.y + force.y > max.y && max.y != 0)
		speed.y = max.y;
	else
		speed.y += force.y;

	speed = resolveAxesBlock(speed);
}

vector2f Mob::getNextPos() {
	return vector2f(x + speed.x, y - speed.y);
}