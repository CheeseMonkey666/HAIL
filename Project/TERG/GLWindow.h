#ifndef H_GLWINDOW
#define H_GLWINDOW

#include <GL/glew.h>
#include <GL/GLU.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <thread>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GVector.h"
#include "Shader.h"
#include "Object.h"
#include "Physics.h"
#include "Tile.h"

class Object;
class Sprite;
class Physics;
class Tile;

struct Atlas {
	int width, height, tileSizeX, tileSizeY, border;
	GLuint texID;

	Atlas(GLWindow *window, const char *texPath, vector2i tileSize, int border = 0);
	Atlas(GLuint texID, vector2i tileSize, int border = 0);
	Atlas()
		:width(0), height(0), tileSizeX(0), tileSizeY(0), texID(0) {}

	vector<GLfloat> getTexCoords(int tileNum);
	vector<GLfloat> getTexCoords(vector2i tileCoords);
};

struct TileMap {
	static const vector<GLuint> defaultIndices;
	static const int vertFloatsPerTile = 32, indCountPerTile = 6;
	int width, height, x, y, tileSize;
	vector<Tile*> tiles, looseTiles;
	vector<GLfloat> verts;
	vector<GLuint> indices;
	Sprite* sprite;

	TileMap(Sprite *sprite, int width, int height, int tileSize);

	void batch();
	Tile* addTile(int x, int y, Atlas *atlas, int atlasTileNum); //X and Y are local
	void removeTile(int x, int y); //X and Y are local
};

class GLWindow
{
	bool stopLoop, loopDone, workerDone;
	const Shader *objectSh, *spriteSh;
	std::thread workerThread;
	Object *camera;
	vector<Object*> activeObjects;

public:

	static const vector<GLuint> defaultIndices;
	static const vector<GLuint> lineIndices;

	GLFWwindow *window;
	int width, height;
	std::vector<Object*> objects;
	const char *title;
	vector<TileMap*> tileMaps;
	vector<TileMap*> mapsToBatch;
	vector3f clearColor;
	void(*loopCall)(), (*workerCall)();
	void(*keyCall)(GLFWwindow* const, int, int, int, int);
	double frameDelta;
	Physics *physics;
	bool debugMode, syncWorkerThread;

	GLWindow(GLFWwindow **win, const char *title, int w = 200, int h = 200);
	~GLWindow();

	void stop();
	void start(int iterations = -1);
	void loop(int iterations = -1);
	void endHere();
	void setLoopCall(void(*function)());
	void setWorkerLoopCall(void(*function)());
	void setKeyCallback(void(*function)(GLFWwindow *, int, int, int, int));
	void setPhysicsEnvironment(Physics *const physics);
	TileMap *addTileMap(int x, int y, int width, int height, int tileSize, Atlas *atlas);
	void updateObjectColor(Object *const object, vector3f color);
	void updateSpriteTexture(Sprite *const sprite, GLuint texID);
	void updateSpriteTexture(Sprite *const sprite, Atlas *atlas, unsigned int tile);
	void flipSpriteTexture(Sprite *const sprite);
	void updateObjectTransform(Object *object);
	void enableUpdate(Object *object, bool enable = true);
	static void dummyLoopCall();
	//Converts a pixel coordinate (NOT VALUE) value into an openGL vertex coordinate, given the value and either the width or height of the frame
	static GLfloat pixelToFrame(int value, int dimension);
	static int frameToPixel(GLfloat value, int dimension);
	//Returns {x,y,z,r,g,b,(t),(k)} 2D OpenGL vertices given position and dimensions
	std::vector<GLfloat> getQuadVerts(int x, int y, int quadWidth, int quadHeight, vector3f *color, bool needsTex = false, GLfloat depth = 0) const;
	std::vector<GLfloat> getQuadVerts(int x, int y, int quadWidth, int quadHeight, vector3f *color, Atlas *atlas, int tileNum, GLfloat depth = 0) const;
	GLuint createTexture(const char* path, int *width = NULL, int *height = NULL);
	GLuint createTexture(vector<unsigned char> image, int width, int height);
	//Camera object (preferably empty) should have its tranform updated to the inverse of desired camera movement 
	void setCamera(Object *camera);
	const Shader *getDefaultObjectShader() { return objectSh; }
	const Shader *getDefaultSpriteShader() { return spriteSh; }

	Object *addObject(int x, int y, int width, int height, vector3f *color, GLfloat depth = 0, GLenum usage = GL_STATIC_DRAW, GLenum drawMode = GL_TRIANGLES, GLFWwindow *context = NULL);
	Object *addEmpty(int x, int y, int width, int height);
	Sprite *addSprite(int x, int y, int width, int height, const char *texturePath, GLfloat depth = 0, GLenum usage = GL_STATIC_DRAW, GLFWwindow *context = NULL);
	Sprite *addSprite(int x, int y, int width, int height, GLuint textureId, GLfloat depth = 0, GLenum usage = GL_STATIC_DRAW, GLFWwindow *context = NULL);
	Sprite *addSprite(int x, int y, int width, int height, Atlas *atlas, int atlasTileNum, GLfloat depth = 0, GLenum usage = GL_STATIC_DRAW, GLFWwindow *context = NULL);
	Sprite *addTile(int x, int y, const char *texturePath, GLenum usage = GL_STATIC_DRAW, GLFWwindow *context = NULL);
	Sprite *addTile(int x, int y, GLuint textureId, GLenum usage = GL_STATIC_DRAW, GLFWwindow *context = NULL);
	Tile *addSingleTile(int x, int y, Atlas *atlas, int atlasTileNum); // Tile will be streamed directly into tilemap, rather than batched with the rest
	void addObject(Object *object, GLFWwindow *context = NULL);
	void removeObject(Object *Object);

	void occludeObjects();
	void occludeTiles();
	void occludeObject(Object *object);
	void unOccludeObject(Object *object);
	void takeFocus();

	void worker();
};

#endif
