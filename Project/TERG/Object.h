#ifndef H_OBJECT
#define H_OBJECT
#include <GL/glew.h>
#include <GL/GLU.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLWindow.h"

enum Axis {X, Y, Z, W, POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z, POS_W, NEG_W};

class GLWindow;
struct QuadTree;
struct Atlas;

class Object
{
	glm::mat4 transform;
	

public:
	int x, y;
	int width, height;
	const int originalX, originalY;
	double errorX, errorY;
	GLuint VAO, VBO, EBO;
	GLfloat depth;
	GLenum drawMode;
	const Shader *const shader;
	unsigned int vertCount;
	GLWindow *const window;
	bool collidable, isStatic, occluded;
	QuadTree *parentQuad;
	glm::vec3 lastTransform;
	vector<Axis> blockedAxes;
	vector2f collider[2];

	Object(int x, int y, int width, int height, GLWindow *window, GLuint vao, GLuint vbo, const Shader *shader, unsigned int vertCount, GLfloat depth = 0, GLuint ebo = 0, GLenum drawMode = GL_TRIANGLES);

	virtual void update();
	//Translate with respect to the frame delta time
	virtual void translate(vector2f vector);
	//Translate without respect to delta time
	virtual void translateAbs(vector2f vector);

	virtual void undoTransform(Axis a);

	glm::mat4 getTransformValue() const { return transform; }
	void setTransformValue(glm::mat4 t, int windowWidth, int windowHeight);
	void blockAxis(Axis a);
	Object *simpleCollision(vector<Object*> objects);
	bool collideWith(Object *object, Object* others, int *xoverlap = NULL, int *yoverlap = NULL, bool ignoreCollidable = true);
	vector2f resolveAxesBlock(vector2f vector);

	bool operator == (const Object& o) const {
		return x == o.x && y == o.y && width == o.width && height == o.height && VAO == o.VAO;
	}
};

class Sprite : public Object
{
	unsigned int animationFrame, animationFPS, minAnimationLoops;
	double animationDelta;
	vector<unsigned int> animation;
	Atlas *animationSheet;

public:
	GLuint texture;
	bool animationEnabled, flipped;
	unsigned int animationLoopCount;

	Sprite(int x, int y, int width, int height, GLWindow *window, GLuint vao, GLuint vbo, GLuint tex, const Shader *shader, unsigned int vertCount, GLfloat depth = 0, GLuint ebo = 0);

	Sprite(Sprite *sprite)
		:Object(sprite->x, sprite->y, sprite->width, sprite->height, sprite->window, sprite->VAO, sprite->VBO, sprite->shader, sprite->vertCount, sprite->depth, sprite->EBO, sprite->drawMode),
		texture(sprite->texture)
	{

	};

	void enableAnimation(bool enable = true) { animationEnabled = enable; }
	void setAnimation(Atlas *spriteSheet, vector<unsigned int> tiles, unsigned int fps, unsigned int minLoopCount = 0);
	void animate();

	void update();
};

#endif
