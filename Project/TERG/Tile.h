#ifndef _H_TILE
#define _H_TILE
#include <GL/glew.h>
#include <GL/GLU.h>

#include "GLWindow.h"

class GLWindow;
class Atlas;

class Tile
{


public:

	int x, y, size;
	vector<GLfloat> verts;

	Tile(int x, int y, int size, GLWindow *window, Atlas *atlas, int atlasTileNum);
	~Tile();

};

#endif