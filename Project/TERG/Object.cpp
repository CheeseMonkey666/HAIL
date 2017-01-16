#include "Object.h"
#include "GLWindow.h"
#include "Mob.h"

Object::Object(int x, int y, int width, int height, GLWindow *window, GLuint vao, GLuint vbo, const Shader *sh, unsigned int vtxCount, GLfloat depth, GLuint ebo, GLenum drawMode)
	:x(x), y(y), width(width), height(height),
	originalX(x), originalY(y),
	VAO(vao), VBO(vbo), EBO(ebo),
	shader(sh),
	vertCount(vtxCount),
	window(window),
	drawMode(drawMode), depth(depth),
	collidable(true), isStatic(true), occluded(false)
{
	collider[0] = vector2f(0, 0);
	collider[1] = vector2f(width, height);
}

Sprite::Sprite(int x, int y, int width, int height, GLWindow *window, GLuint vao, GLuint vbo, GLuint tex, const Shader *sh, unsigned int vtxCount, GLfloat depth, GLuint ebo)
	:Object(x, y, width, height, window, vao, vbo, sh, vtxCount, depth, ebo),
	texture(tex), animationEnabled(false), flipped(false)
{
}

void Object::update() {

}

void Object::translate(vector2f vector) {
	vector = resolveAxesBlock(vector);
	blockedAxes.clear();
	double delta = window->frameDelta, integerX, integerY;
	lastTransform = glm::vec3(glm::vec3((2 * vector.x / window->width) * delta, (2 * vector.y / window->height) * delta, 0));
	transform = glm::translate(transform, lastTransform);
	errorX += modf((double)vector.x * delta, &integerX);
	errorY -= modf((double)vector.y * delta, &integerY);
	x += (int)integerX;
	y -= (int)integerY;
	if (abs(errorX) > 1) {
		errorX = modf(errorX, &integerX);
		x += (int)integerX;
	}
	if (abs(errorY) > 1) {
		errorY = modf(errorY, &integerY);
		y += (int)integerY;
	}
}

void Object::translateAbs(vector2f vector) {
	vector = resolveAxesBlock(vector);
	double integerX, integerY;
	lastTransform = glm::vec3(glm::vec3((2 * vector.x / window->width), (2 * vector.y / window->height), 0));
	transform = glm::translate(transform, lastTransform);
	errorX += modf((double)vector.x, &integerX);
	errorY -= modf((double)vector.y, &integerY);
	x += (int)integerX;
	y -= (int)integerY;
	if (abs(errorX) > 1) {
		errorX = modf(errorX, &integerX);
		x += (int)integerX;
	}
	if (abs(errorY) > 1) {
		errorY = modf(errorY, &integerY);
		y += (int)integerY;
	}
}

void Object::undoTransform(Axis a) {
	switch (a) {
	case X:
		x -= lastTransform.x * window->width / 2;
		transform = glm::translate(transform, glm::vec3(-lastTransform.x, 0, 0));
		break;
	case Y:
		y += lastTransform.y * window->height / 2;
		transform = glm::translate(transform, glm::vec3(0, -lastTransform.y, 0));
		break;
	case Z:
		transform = glm::translate(transform, glm::vec3(0, 0, -lastTransform.z));
		break;
	}
}

void Object::blockAxis(Axis a) {
	blockedAxes.push_back(a);
}

vector2f Object::resolveAxesBlock(vector2f vector) {
	if (vectorContains(blockedAxes, X))
		vector.x = 0;
	if (vectorContains(blockedAxes, POS_X) && vector.x > 0)
		vector.x = 0;
	if (vectorContains(blockedAxes, NEG_X) && vector.x < 0)
		vector.x = 0;
	if (vectorContains(blockedAxes, Y))
		vector.y = 0;
	if (vectorContains(blockedAxes, POS_Y) && vector.y > 0)
		vector.y = 0;
	if (vectorContains(blockedAxes, NEG_Y) && vector.y < 0)
		vector.y = 0;
	return vector;
}

//SET TRANSFORM FUNCTION (UPDATES X & Y AS WELL AS MAT4)
void Object::setTransformValue(glm::mat4 t, int windowWidth, int windowHeight) {
	transform = t;
	glm::vec4 newCoord = transform * glm::vec4(((float)originalX / (float)windowWidth) * 2 - 1, ((float)originalY / (float)windowHeight) * 2 - 1, 0, 1);
	newCoord.x = ((newCoord.x + 1) / 2) * windowWidth;
	newCoord.y = ((newCoord.y + 1) / 2) * windowHeight;
	double newX = 0, newY = 0;
	errorX = modf(newCoord.x, &newX);
	errorY = modf(newCoord.y, &newY);
	x = (int)newX;
	y = (int)newY;
}

Object *Object::simpleCollision(vector<Object*> objects) {
	Object *collided = NULL;
	Mob *m = dynamic_cast<Mob*>(this);
	for (int j = 0; j < objects.size(); j++) {
		if (this == objects[j])
			continue;
		int xover, yover;
		if (collideWith(this, objects[j], &xover, &yover)) {
			collided = objects[j];
			if (abs(xover) > 0 && abs(xover) < abs(yover)) {
				if (m != NULL)
					m->setSpeed(0, X);
				this->translateAbs(vector2f(xover, 0));
			}
			else if (abs(yover) > 0 && abs(yover) < abs(xover)) {
				if (m != NULL)
					m->setSpeed(0, Y);
				this->translateAbs(vector2f(0, -yover));
			}
		}
		else if (!this->isStatic && m != NULL) {
			Object *probe = window->addEmpty(m->x + m->collider[0].x, m->y + m->collider[0].y + 2, m->collider[1].x, m->collider[1].y);
			if (collideWith(probe, objects[j], &xover, &yover)) {
				if (abs(yover) > 0 && abs(yover) < abs(xover)) {
					this->blockAxis(NEG_Y);
				}
			}
			delete probe;
			probe = window->addEmpty(m->x + m->collider[0].x + 2, m->y + m->collider[0].y, m->collider[1].x, m->collider[1].y);
			if (collideWith(probe, objects[j], &xover, &yover)) {
				if (abs(xover) > 0 && abs(xover) < abs(yover)) {
					this->blockAxis(POS_X);
				}
			}
			delete probe;
			probe = window->addEmpty(m->x + m->collider[0].x - 2, m->y + m->collider[0].y, m->collider[1].x, m->collider[1].y);
			if (collideWith(probe, objects[j], &xover, &yover)) {
				if (abs(xover) > 0 && abs(xover) < abs(yover)) {
					this->blockAxis(NEG_X);
				}
			}
			delete probe;
			probe = window->addEmpty(m->x + m->collider[0].x, m->y + m->collider[0].y - 2, m->collider[1].x, m->collider[1].y);
			if (collideWith(probe, objects[j], &xover, &yover)) {
				if (abs(yover) > 0 && abs(yover) < abs(xover)) {
					this->blockAxis(POS_Y);
				}
			}
			delete probe;
		}
	}
	return collided;
}

bool Object::collideWith(Object *object, Object* others, int *xoverlap, int *yoverlap, bool ignoreCollidable) {
	if (!object->collidable && !ignoreCollidable)
		return false;
	if (!others->collidable && !ignoreCollidable)
		return false;
	Object* other = others;
	bool overlapx = object->x + object->collider[0].x < other->x + other->collider[1].x && object->x + object->collider[1].x > other->x + other->collider[0].x;
	bool overlapy = object->y + object->collider[0].y < other->y + other->collider[1].y && object->y + object->collider[1].y > other->y + other->collider[0].y;
	if (xoverlap != NULL) {
		if (overlapx) {
			if (object->collider[0].x > other->collider[0].x) {
				*xoverlap = other->x + other->collider[1].x - (object->x + object->collider[0].x);
			}
			else {
				*xoverlap = -(object->x + object->collider[1].x - (other->x + other->collider[0].x));
			}
		}
		else
			*xoverlap = 0;
	}
	if (yoverlap != NULL) {
		if (overlapy) {
			if (object->collider[0].y > other->collider[0].y) {
				*yoverlap = other->y + other->collider[1].y - (object->y + object->collider[0].y);
			}
			else {
				*yoverlap = -(object->y + object->collider[1].y - (other->y + other->collider[0].y));
			}
		}
		else
			*yoverlap = 0;
	}
	if (overlapx && overlapy)
		return true;
	return false;
}

void Sprite::setAnimation(Atlas *spriteSheet, vector<unsigned int> tiles, unsigned int fps) {
	if (!animationEnabled)
		return;
	animationSheet = spriteSheet;
	animation = tiles;
	animationFPS = fps;
	animationFrame = 0;
	animationDelta = glfwGetTime();
	window->updateSpriteTexture(this, spriteSheet, tiles[0]);
}

void Sprite::animate() {
	if (!animationEnabled)
		return;
	if (glfwGetTime() - animationDelta >= (double)1.0f / animationFPS) {
		animationFrame++;
		if (animationFrame >= animation.size())
			animationFrame = 0;
		window->updateSpriteTexture(this, animationSheet, animation[animationFrame]);
		animationDelta = glfwGetTime();
	}
}

void Sprite::update() {
	//printf("Sprite updating\n");
}