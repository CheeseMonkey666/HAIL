#include "Tile.h"

#include <GL/glew.h>
#include <GL/GLU.h>
#include "GLWindow.h"


Tile::Tile(int x, int y, int size, GLWindow *window, Atlas *atlas, int atlasTileNum)
	:x(x), y(y), size(size)
{
	verts = window->getQuadVerts(x * size, y * size, size, size, new vector3f(1, 1, 1), atlas, atlasTileNum, 0);
}


Tile::~Tile()
{
}
