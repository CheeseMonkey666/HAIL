#pragma once


#include "GLWindow.h"

class GLWindow;
class Object;

struct QuadTree
{
	vector<Object*> objects;
	vector<QuadTree*> children;
	const int threshold, x, y, width, height;
	int objectCount, minSize;
	const bool master;
	Object *object;
	GLWindow *window;
	QuadTree *parent;

	QuadTree(vector<Object*> *const objects, int threshold, GLWindow *window, int x, int y, int width, int height, bool masterQuad = false, int minSize = 0);
	QuadTree(vector<Object*> *const objects, QuadTree *parent, int xParentOffset, int yParentOffset);

	void update();
};

class Physics
{
public:
	GLWindow *window;
	QuadTree *quadTree;

	Physics(GLWindow *window, int quadTreeWidth = 0, int quadTreeHeight = 0, int objectsPerQuad = 16);
	~Physics();

	void collideObjects();
	static bool collideWith(Object *object, vector<Object*> others, int *xoverlap = NULL, int *yoverlap = NULL, bool ignoreCollidable = false);
	static bool collideWith(Object *object, Object *other, int *xoverlap = NULL, int *yoverlap = NULL, bool ignoreCollidable = false);
	//Check if object is FULLY contained by object
	static bool objectContains(Object *container, Object *child);
	void updateQuadTrees();
};

