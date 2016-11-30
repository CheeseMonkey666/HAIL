#include <windows.h>

#include "Physics.h"
#include "Object.h"
#include "Mob.h"


Physics::Physics(GLWindow *window, int quadTreeWidth, int quadTreeHeight, int objectsPerQuad)
	:window(window)
{
	if(quadTreeWidth < 0 || quadTreeHeight < 0)
		quadTree = new QuadTree(&window->objects, objectsPerQuad, window, 0, 0, window->width, window->height, true);
	else
		quadTree = new QuadTree(&window->objects, objectsPerQuad, window, 0, 0, quadTreeWidth, quadTreeHeight, true);
	updateQuadTrees();
}

QuadTree::QuadTree(vector<Object*> *const objects, int threshold, GLWindow *window, int x, int y, int width, int height, bool masterQuad, int minSize)
	:objects(*objects),
	threshold(threshold), minSize(minSize),
	x(x), y(y), width(width), height(height),
	window(window),
	master(masterQuad),
	//object(window->addObject(x, y, width, height, new vector3f(0, 1, 0), -1, GL_STATIC_DRAW, GL_LINES))
	object(window->addEmpty(x, y, width, height))
{
	if (minSize <= 0)
		if (width < height)
			this->minSize = height / 16;
		else
			this->minSize = width / 16;
	object->collidable = false;
	objectCount = objects->size();
}

QuadTree::QuadTree(vector<Object*> *const objects, QuadTree *parent, int xParentOffset, int yParentOffset)
	:objects(*objects),
	threshold(parent->threshold), minSize(parent->minSize),
	x(parent->x + xParentOffset), y(parent->y + yParentOffset),
	width(parent->width / 2), height(parent->height / 2),
	window(parent->window),
	master(false),
	parent(parent)
{
	//object = window->addObject(x, y, width, height, new vector3f(0, 1, 0), -1, GL_STATIC_DRAW, GL_LINES);
	object = window->addEmpty(x, y, width, height);
	object->collidable = false;
	QuadTree *quad = parent;
	while (quad != NULL && !quad->master)
		quad = quad->parent;
	if (vectorContains(quad->objects, object))
		quad->objects.erase(quad->objects.begin() + vectorContainsIndex);
}

void QuadTree::update() {
	//Check if objects need to be moved to a different quad
	bool objectsMoved = false;
	for (int i = 0; i < objects.size(); i++) {
		if (objects[i]->x + objects[i]->width / 2 > x + width || objects[i]->x + objects[i]->width / 2 < x
			|| objects[i]->y + objects[i]->height / 2 > y + height || objects[i]->y + objects[i]->height / 2 < y) {
			QuadTree *quad = objects[i]->parentQuad;
			while (!quad->master)
				quad = quad->parent;
			quad->objects.push_back(objects[i]);
			quad->update();
			objects[i] = NULL;
			objectsMoved = true;
		}
	}
	//Pass objects down to child quads
	if (children.size() > 0 && objects.size() > 0) {
		for (int i = 0; i < objects.size(); i++) {
			objectsMoved = true;
			if ((objects[i]->x - x) + (objects[i]->width / 2) < width / 2) {
				if ((objects[i]->y - y) + (objects[i]->height / 2) < height / 2) {
					children[0]->objects.push_back(objects[i]);
						objects[i] = NULL;
				}
				else {
					children[3]->objects.push_back(objects[i]);
						objects[i] = NULL;
				}
			}
			else {
				if ((objects[i]->y - y) + (objects[i]->height / 2) < height / 2) {
					children[1]->objects.push_back(objects[i]);
						objects[i] = NULL;
				}
				else {
					children[2]->objects.push_back(objects[i]);
						objects[i] = NULL;
				}
			}
		}
	}
	//Check if quad needs to split
	else if (objects.size() > threshold && (width / 2 >= minSize && height / 2 >= minSize)) {
		vector<Object*> tl, tr, bl, br;
		for(int i = 0; i < objects.size(); i++){
			if ((objects[i]->x - x) + (objects[i]->width / 2) < width / 2) {
				if ((objects[i]->y - y) + (objects[i]->height / 2) < height / 2) {
					tl.push_back(objects[i]);
				}
				else {
					bl.push_back(objects[i]);
				}
			}
			else {
				if ((objects[i]->y - y) + (objects[i]->height / 2) < height / 2) {
					tr.push_back(objects[i]);
				}
				else {
					br.push_back(objects[i]);
				}
			}
		}
		objects.clear();
		children.push_back(new QuadTree(&tl, this, 0, 0));
		for (int i = 0; i < tl.size(); i++)
			tl[i]->parentQuad = children[0];
		children.push_back(new QuadTree(&tr, this, width / 2, 0));
		for (int i = 0; i < tr.size(); i++)
			tr[i]->parentQuad = children[1];
		children.push_back(new QuadTree(&br, this, width / 2, height / 2));
		for (int i = 0; i < br.size(); i++)
			br[i]->parentQuad = children[2];
		children.push_back(new QuadTree(&bl, this, 0, height / 2));
		for (int i = 0; i < bl.size(); i++)
			bl[i]->parentQuad = children[3];
		tl.clear();
		tr.clear();
		br.clear();
		bl.clear();
	}
	//Erase moved objects
	int it = 0;
	if (!objectsMoved)
		it = objects.size();
	while (it < objects.size()) {
		if (objects[it] == NULL)
			objects.erase(objects.begin() + it);
		else
			it++;
	}
	//Check if child quads should be merged
	if (children.size() > 0 && !(children[0]->children.size() > 0 || children[1]->children.size() > 0
			|| children[2]->children.size() > 0 || children[3]->children.size() > 0)) {
		int childObjCount = children[0]->objects.size();
		childObjCount += children[1]->objects.size();
		childObjCount += children[2]->objects.size();
		childObjCount += children[3]->objects.size();
		if (childObjCount <= threshold) {
			objects.insert(objects.end(), children[0]->objects.begin(), children[0]->objects.end());
			window->removeObject(children[0]->object);
			delete children[0];
			objects.insert(objects.end(), children[1]->objects.begin(), children[1]->objects.end());
			window->removeObject(children[1]->object);
			delete children[1];
			objects.insert(objects.end(), children[2]->objects.begin(), children[2]->objects.end());
			window->removeObject(children[2]->object);
			delete children[2];
			objects.insert(objects.end(), children[3]->objects.begin(), children[3]->objects.end());
			window->removeObject(children[3]->object);
			delete children[3];
			children.clear();
		}
	}
	for (int i = 0; i < objects.size(); i++) {
		objects[i]->parentQuad = this;
	}
	objectCount = objects.size();
}

Physics::~Physics()
{
}

void Physics::collideObjects() {
	double start = glfwGetTime(), end;
	updateQuadTrees();
	end = glfwGetTime();
	//printf("QuadTree update took %f secs\n", end - start);
	start = glfwGetTime();
	vector<Object*> objects = window->objects;
	for (int i = 0; i < objects.size(); i++) {
		Mob *m = dynamic_cast<Mob*>(objects[i]);
		if (objects[i]->collidable && !objects[i]->isStatic && objects[i]->parentQuad != NULL) {
			//Look first at all objects in the smallest quad that fully contains objects, then narrow it down to those close enough to collide
			vector<Object*> maxLocals, locals; 
			QuadTree *biggestCollidingQuad = objects[i]->parentQuad;
			vector<QuadTree*> unsearched;
			bool foundAllChildren = false;
 			while (!biggestCollidingQuad->master && !objectContains(biggestCollidingQuad->object, objects[i])) {
				biggestCollidingQuad = biggestCollidingQuad->parent;
			}
			maxLocals.insert(maxLocals.end(), biggestCollidingQuad->objects.begin(), biggestCollidingQuad->objects.end());
			unsearched.insert(unsearched.end(), biggestCollidingQuad->children.begin(), biggestCollidingQuad->children.end());
			while (unsearched.size() > 0) {
				maxLocals.insert(maxLocals.end(), unsearched[0]->objects.begin(), unsearched[0]->objects.end());
				unsearched.insert(unsearched.end(), unsearched[0]->children.begin(), unsearched[0]->children.end());
				unsearched.erase(unsearched.begin());
			}
			for (int j = 0; j < maxLocals.size(); j++) {
				if (maxLocals[j]->x < objects[i]->x + objects[i]->width * 2 && maxLocals[j]->x > objects[i]->x - objects[i]->width
					&& maxLocals[j]->y < objects[i]->y + objects[i]->height * 2 && maxLocals[j]->y > objects[i]->y - objects[i]->height)
					locals.push_back(maxLocals[j]);
			}
			maxLocals.clear();

			bool collided = false;
			for (int j = 0; j < locals.size(); j++) {
				if (objects[i] == locals[j])
					continue;
				int xover, yover;
				if (collideWith(objects[i], locals[j], &xover, &yover)) {
					//printf("Collision detected\n");
					collided = true;
					if (abs(xover) > 0 && abs(xover) < abs(yover)) {
						//objects[i]->undoTransform(X);
						if (m != NULL)
							m->setSpeed(0, X);
						objects[i]->translateAbs(vector2f(xover, 0));
					}
					else if (abs(yover) > 0 && abs(yover) < abs(xover)) {
						//objects[i]->undoTransform(Y);
						if (m != NULL)
							m->setSpeed(0, Y);
						objects[i]->translateAbs(vector2f(0, -yover));
					}
				}
				else if (!objects[i]->isStatic && m != NULL) {
					Object *probe = window->addEmpty(m->x, m->y + 1, m->width, m->height);
					if (collideWith(probe, locals[j], &xover, &yover)) {
						if (abs(yover) > 0 && abs(yover) < abs(xover)) {
							objects[i]->blockAxis(NEG_Y);
						}
					}
					delete probe;
					probe = window->addEmpty(m->x + 1, m->y, m->width, m->height);
					if (collideWith(probe, locals[j], &xover, &yover)) {
						if (abs(xover) > 0 && abs(xover) < abs(yover)) {
							objects[i]->blockAxis(POS_X);
						}
					}
					delete probe;
					probe = window->addEmpty(m->x - 1, m->y, m->width, m->height);
					if (collideWith(probe, locals[j], &xover, &yover)) {
						if (abs(xover) > 0 && abs(xover) < abs(yover)) {
							objects[i]->blockAxis(NEG_X);
						}
					}
					delete probe;
					probe = window->addEmpty(m->x, m->y - 1, m->width, m->height);
					if (collideWith(probe, locals[j], &xover, &yover)) {
						if (abs(yover) > 0 && abs(yover) < abs(xover)) {
							objects[i]->blockAxis(POS_Y);
						}
					}
					delete probe;
				}
			}
		}
	}
	end = glfwGetTime();
	//printf("Collision resolution took %f secs\n", end - start);
}

bool Physics::collideWith(Object *object, vector<Object*> others, int *xoverlap, int *yoverlap, bool ignoreCollidable){
	if (!object->collidable && !ignoreCollidable)
		return false;
	for (int i = 0; i < others.size(); i++) {
		if (!others[i]->collidable && !ignoreCollidable)
			continue;
		Object* other = others[i];
		bool overlapx = object->x < other->x + other->width && object->x + object->width > other->x;
		bool overlapy = object->y < other->y + other->height && object->y + object->height > other->y;
		if (xoverlap != NULL) {
			if (overlapx) {
				if (object->x > other->x) {
					*xoverlap = other->x + other->width - object->x;
				}
				else {
					*xoverlap = -(object->x + object->width - other->x);
				}
			}
			else
				*xoverlap = 0;
		}
		if (yoverlap != NULL) {
			if (overlapy) {
				if (object->y > other->y) {
					*yoverlap = other->y + other->height - object->y;
				}
				else {
					*yoverlap = -(object->y + object->height - other->y);
				}
			}
			else
				*yoverlap = 0;
		}
		if (overlapx && overlapy)
			return true;
	}
	return false;
}

bool Physics::collideWith(Object *object, Object *other, int *xoverlap, int *yoverlap, bool ignoreCollidable){
	if ((!object->collidable || !other->collidable) && !ignoreCollidable)
		return false;
	bool overlapx = object->x < other->x + other->width && object->x + object->width > other->x;
	bool overlapy = object->y < other->y + other->height && object->y + object->height > other->y;
	if (xoverlap != NULL) {
		if (overlapx) {
			if (object->x > other->x) {
				*xoverlap = other->x + other->width - object->x;
			}
			else {
				*xoverlap = -(object->x + object->width - other->x);
			}
		}
		else
			*xoverlap = 0;
	}
	if (yoverlap != NULL) {
		if (overlapy) {
			if (object->y > other->y) {
				*yoverlap = other->y + other->height - object->y;
			}
			else {
				*yoverlap = -(object->y + object->height - other->y);
			}
		}
		else
			*yoverlap = 0;
	}
	if (overlapx && overlapy)
 		return true;
	return false;
}

void Physics::updateQuadTrees() {
	int it = 0;
	vector<QuadTree*> trees, buff;
	trees.push_back(quadTree);
	while (!trees.empty()) {
		//printf("Iteration of quadupdate: %d\n", it);
		it++;
		trees[0]->objectCount = trees[0]->objects.size();
		trees[0]->update();
		for (int j = 0; j < trees[0]->children.size(); j++)
			trees.push_back(trees[0]->children[j]);
		trees.erase(trees.begin());
		//printf("Number of quads to go: %d\n", trees.size());
	}
}

bool Physics::objectContains(Object *container, Object *child){
	bool xcontained = child->x > container->x && child->x + child->width < container->x + container->width;
	bool ycontained = child->y > container->y && child->y + child->height < container->y + container->height;
	if (xcontained && ycontained)
		return true;
	return false;
}
