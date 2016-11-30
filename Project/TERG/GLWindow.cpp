#include "GLWindow.h"

#include <GL/glew.h>
#include <GL/GLU.h>
#include <GLFW/glfw3.h>
#include <windows.h>
#include <ctime>
#include <thread>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL.h>

#include "GVector.h"
#include "Shader.h"
#include "Object.h"
#include "Tile.h"

void glfwError(int error, const char *description) {
	std::cout << "GLFW error '" << error << "', " << description << "\n";
}

//################## Atlas Section ##################

Atlas::Atlas(GLWindow *window, const char* texPath, vector2i tileSize)
	:width(0), height(0)
{
	texID = window->createTexture(texPath, &width, &height);
	tileSizeX = tileSize.x;
	tileSizeY = tileSize.y;
}

Atlas::Atlas(GLuint texID, vector2i tileSize)
	:width(0), height(0), texID(texID)
{
	tileSizeX = tileSize.x;
	tileSizeY = tileSize.y;
}

vector<GLfloat> Atlas::getTexCoords(int tileNum) {
	vector<GLfloat> result;
	vector<GLfloat> delta { 0.001f, 0.001f, 0.999f, 0.001f, 0.999f, 0.999f, 0.001f, 0.999f }; //offset by 0.001f to stop texture bleeding caused by float inaccuracy
	int tilesX = floor(width / tileSizeX);
	int tilesY = floor(height / tileSizeY);
	for (int i = 0; i < 4; i++) {
		float x = tileNum;
		float y = floor(tileNum / tilesX);
		while (x >= tilesX)
			x -= tilesX;
		x *= tileSizeX;
		y *= tileSizeY;
		result.push_back((x + delta[i * 2] * tileSizeX) / width);
		result.push_back((y + delta[i * 2 + 1] * tileSizeY) / height);
	}
	return result;
}

vector<GLfloat> Atlas::getTexCoords(vector2i tileCoords) {
	vector<GLfloat> result;
	vector<int> delta{ 0, 0, 1, 0, 1, 1, 0, 1 };
	for (int i = 0; i < 4; i++) {
		result.push_back((tileCoords.x + delta[i * 2] * tileSizeX) / width);
		result.push_back((tileCoords.y + delta[i * 2 + 1] * tileSizeY) / height);
	}
	return result;
}

//################## TileMap Section ##################
const vector<GLuint> TileMap::defaultIndices({ 0, 1, 2,  2, 3, 0 });

TileMap::TileMap(Sprite *sprite, int width, int height, int tileSize)
	:sprite(sprite),
	width(width), height(height), tileSize(tileSize), x(sprite->x), y(sprite->y)
{
	tiles = vector<Tile*>(width * height);
	verts = vector<GLfloat>(width * height * vertFloatsPerTile);
	indices = vector<GLuint>(width * height * indCountPerTile, -1);

	glBindVertexArray(sprite->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, sprite->VBO);
	glBufferData(GL_ARRAY_BUFFER, width * height * vertFloatsPerTile * sizeof(GLfloat), &verts[0], GL_STREAM_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprite->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, width * height * indCountPerTile * sizeof(GLuint), &indices[0], GL_STREAM_DRAW);
	glBindVertexArray(0);
}

Tile *TileMap::addTile(int x, int y, Atlas *atlas, int tileNum) {
	Tile *tile = new Tile(x + this->x, y + this->y, tileSize, sprite->window, atlas, tileNum);
	looseTiles.push_back(tile);
	tiles[(y * width) + x] = tile;

	return tile;
}

void TileMap::removeTile(int x, int y) {
	int index = (y * width) + x;
	if (tileSize > 0 && tiles[index] != NULL) {
		for (int i = 0; i < vertFloatsPerTile; i++) {
			verts[index * vertFloatsPerTile + i] = 0;
		}
		for (int i = 0; i < indCountPerTile; i++) {
			indices[index * indCountPerTile + i] = -1;
		}
		delete tiles[index];
		tiles[index] = NULL;
	}
}

void TileMap::batch() {
	vector<GLfloat> v;

	for (int i = 0; i < looseTiles.size(); i++) {
		if (looseTiles[i] == NULL)
			continue;
		v = looseTiles[i]->verts;
		for (int j = 0; j < vertFloatsPerTile; j++)
			verts[(((looseTiles[i]->y - y) * width) + looseTiles[i]->x - x) * vertFloatsPerTile + j] = v[j];
		for (int j = 0; j < defaultIndices.size(); j++)
			indices[(((looseTiles[i]->y - y) * width) + looseTiles[i]->x - x) * indCountPerTile + j] = (((looseTiles[i]->y - y) * width) + looseTiles[i]->x - x) * 4 + defaultIndices[j];
		//tileCount++;
	}
	looseTiles.clear();
	if (!vectorContains(sprite->window->mapsToBatch, this))
		sprite->window->mapsToBatch.push_back(this);
}

//################## GLWindow Section ##################

const vector<GLuint> GLWindow::defaultIndices({ 0, 1, 2,  2, 3, 0 });
const vector<GLuint> GLWindow::lineIndices({ 0, 1,  1, 2,  2, 3,  3, 0 });

GLWindow::GLWindow(GLFWwindow **win, const char *title, int w, int h)
	:width(w), height(h),
	frameDelta(0.016f),
	physics(NULL),
	debugMode(false), syncWorkerThread(false),
	camera(NULL),
	title(title)
{
	glfwSetErrorCallback(glfwError);
	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	window = glfwCreateWindow(w, h, title, NULL, NULL);
	if(win != NULL)
		*win = window;

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
		exit(EXIT_FAILURE);
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	clearColor = vector3f(0, 0, 0);
	setLoopCall(&dummyLoopCall);
	setWorkerLoopCall(NULL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	objectSh = new Shader("shapeVertex.glsl", "shapeFragment.glsl");
	spriteSh = new Shader("spriteVertex.glsl", "spriteFragment.glsl");
}

void GLWindow::setLoopCall(void(*function)()) {
	loopCall = function;
}

void GLWindow::setWorkerLoopCall(void(*function)()) {
	workerCall = function;
}

void GLWindow::loop(int iterations) {
	double start = glfwGetTime(), end = glfwGetTime(), total = 0;
	double timerStart, timerEnd, averageRenderTime = 0;
	int it = 0, frameCount = 0, textureChangePerFrame = 0, cameraUpdatePerFrame = 0;
	Sprite *tileSprite;

	//Pass transforms of all objects (including tiles and static)
	for (int i = 0; i < objects.size(); i++) {
		const Shader *sh = objects[i]->shader;
		sh->use();
		GLuint transformLoc = glGetUniformLocation(sh->program, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(objects[i]->getTransformValue()));
		glUseProgram(0);
	}

	while (!glfwWindowShouldClose(window) && !stopLoop && (it < iterations || iterations < 0)) {
		//Sleep((1000 / fps) - (end - start));
		int tileMapSpriteObjectIndex = -1;
		loopDone = false;
		frameDelta = end - start;
		start = glfwGetTime();
		total += frameDelta;
		if (total >= 1) {
			printf("'%s' rend time: %f, frames: %d, texChPerFr: %d, camPerFr: %d\n\n", title, averageRenderTime / frameCount, frameCount, textureChangePerFrame / frameCount, cameraUpdatePerFrame / frameCount);
			averageRenderTime = 0;
			total = 0;
			frameCount = 0;
			textureChangePerFrame = 0;
			cameraUpdatePerFrame = 0;
		}

		/*if(camera != NULL)
			occludeObjects();*/
		glfwMakeContextCurrent(window);
		timerStart = glfwGetTime();
		
		(*loopCall)();

		for (unsigned int i = 0; i < activeObjects.size(); i++) {
			if (!activeObjects[i]->occluded)
				activeObjects[i]->update();
		}

		if (physics != NULL) {
			physics->collideObjects();
		}

		glClearColor(clearColor.x, clearColor.y, clearColor.z, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		const Shader *sh = NULL;
		GLuint transformLoc, cameraLoc, texture = MAXUINT;
		//ADD TILES TO OBJECTS
		for (int t = 0; t < mapsToBatch.size(); t++) {
			if (mapsToBatch[t]->verts.size() > 0) {
				glBindVertexArray(mapsToBatch[t]->sprite->VAO);
				glBindBuffer(GL_ARRAY_BUFFER, mapsToBatch[t]->sprite->VBO);
				glBufferSubData(GL_ARRAY_BUFFER, 0, mapsToBatch[t]->verts.size() * sizeof(GLfloat), &mapsToBatch[t]->verts[0]);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mapsToBatch[t]->sprite->EBO);
				glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mapsToBatch[t]->indices.size() * sizeof(GLuint), &mapsToBatch[t]->indices[0]);
				glBindVertexArray(0);
			}
		}
		mapsToBatch.clear();
		for(int t = 0; t < tileMaps.size(); t++){
			if(tileMapSpriteObjectIndex < 0)
				tileMapSpriteObjectIndex = objects.size();
			objects.push_back(tileMaps[t]->sprite);
			tileMaps[t]->sprite->vertCount = tileMaps[t]->indices.size();
		}

		//DRAW OBJECTS
		for (unsigned int i = 0; i < objects.size(); i++) {
			if (objects[i]->occluded)
				continue;
			if (objects[i]->shader != sh) {
				sh = objects[i]->shader;
				sh->use();
				cameraLoc = glGetUniformLocation(sh->program, "cameraTransform");
				if (camera != NULL) {
					glUniformMatrix4fv(cameraLoc, 1, GL_FALSE, glm::value_ptr(glm::inverse(camera->getTransformValue())));
					cameraUpdatePerFrame++;
				}
			}
			sh->use();
			if (!objects[i]->isStatic) {
				transformLoc = glGetUniformLocation(sh->program, "transform");
				glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(objects[i]->getTransformValue()));
			}

			Sprite *spr = dynamic_cast<Sprite*>(objects[i]);
			if (spr != NULL && texture != spr->texture) {
				texture = spr->texture;
				glBindTexture(GL_TEXTURE_2D, texture);
				textureChangePerFrame++;
			}
			else if (spr == NULL) {
				texture = 0;
				glBindTexture(GL_TEXTURE_2D, 0);
				textureChangePerFrame++;
			}
			glBindVertexArray(objects[i]->VAO);
			if (objects[i]->EBO > 0) {
				GLint vao = 0, vbo = 0, ebo = 0;
				glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
				glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &ebo);
				glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &vbo);
				glDrawElements(objects[i]->drawMode, objects[i]->vertCount, GL_UNSIGNED_INT, 0);
				if (debugMode) {
					glPointSize(4);
					glBindTexture(GL_TEXTURE_2D, 0);
					glDrawElements(GL_LINES, objects[i]->vertCount, GL_UNSIGNED_INT, 0);
					glDrawElements(GL_POINTS, objects[i]->vertCount, GL_UNSIGNED_INT, 0);
					glBindTexture(GL_TEXTURE_2D, texture);
				}
			}
			else {
				glDrawArrays(objects[i]->drawMode, 0, objects[i]->vertCount);
				if (debugMode) {
					glPointSize(4);
					glBindTexture(GL_TEXTURE_2D, 0);
					glDrawElements(GL_LINES, objects[i]->vertCount, GL_UNSIGNED_INT, 0);
					glDrawElements(GL_POINTS, objects[i]->vertCount, GL_UNSIGNED_INT, 0);
					glBindTexture(GL_TEXTURE_2D, texture);
				}
			}
			glBindVertexArray(0);
			if (!objects[i]->isStatic) {
				transformLoc = glGetUniformLocation(sh->program, "transform");
				glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
			}
			glUseProgram(0);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();

		averageRenderTime += glfwGetTime() - timerStart;

		if(tileMapSpriteObjectIndex >= 0)
			objects.erase(objects.begin() + tileMapSpriteObjectIndex, objects.end()); //Take tileMapSprite back out

		end = glfwGetTime();
		frameCount++;
		it++;
		loopDone = true;
		while (!workerDone && syncWorkerThread) {}
	}
}

void GLWindow::worker() {	
	double end = glfwGetTime(), start = glfwGetTime(), total = 0, vStart = start, vTime = 0, iTime = 0;
	int frameCount = 0;
	while (!stopLoop) {
		total += end - start;
		if (total >= 1) {
			printf("Worker thread frames: %d, vTime: %f, iTime: %f\n", frameCount, vTime / frameCount, iTime / frameCount);
			total = 0;
			frameCount = 0;
			vTime = 0;
			iTime = 0;
		}
		start = glfwGetTime();
		workerDone = false;
		if(workerCall != NULL)
			(*workerCall)();

		/*if (camera != NULL && tileSize > 0 && tileMap.size() > 0 && tileMapSprite != NULL) {
			while (loopAccessTiles) {}
			workerAccessTiles = true;
			int tileCount = 0;
			vector<GLfloat> v;
			tileMapVerts.clear();
			tileMapInd.clear();
			for (int x = floor(camera->x / tileSize) - 1; x < ceil((camera->x + width) / tileSize) + 1; x++) {
				for (int y = floor(camera->y / tileSize) - 1; y < ceil((camera->y + height) / tileSize) + 1; y++) {
					if (x < 0 || x > tileMapWidth - 1 || y < 0 || y > tileMapHeight - 1 || tileMap[(y * tileMapWidth) + x] == NULL)
						continue;
					vStart = glfwGetTime();
					v = tileMap[(y * tileMapWidth) + x]->verts;
					tileMapVerts.insert(tileMapVerts.end(), v.begin(), v.end());
					vTime += glfwGetTime() - vStart;
					vStart = glfwGetTime();
					for (int i = 0; i < defaultIndices.size(); i++)
						tileMapInd.push_back(defaultIndices[i] + tileCount * 4);
					iTime += glfwGetTime() - vStart;
					tileCount++;
				}
			}
		}
		workerAccessTiles = false;*/

		workerDone = true;
		while (!loopDone && syncWorkerThread) {}
		frameCount++;
		end = glfwGetTime();
	}
}

void GLWindow::dummyLoopCall() {
	//do nothing
}

void GLWindow::occludeObjects() {
	vector<Object*> toBuffer, toObjects;
	for (int i = 0; i < objects.size(); i++) {
		Object *obj = objects[i];
		if (!obj->occluded && (obj->x + obj->width < camera->x || obj->x - obj->width > camera->x + width
			|| obj->y + obj->height < camera->y || obj->y - obj->height > camera->y + height)) {
			toBuffer.push_back(obj);
		}
		else if (obj->occluded && obj->x + obj->width > camera->x && obj->x < camera->x + width
			&& obj->y + obj->height > camera->y && obj->y < camera->y + height) {
			toObjects.push_back(obj);
		}

	}
	for (int i = 0; i < toObjects.size(); i++) {
		if(physics != NULL)
			physics->quadTree->objects.push_back(toObjects[i]);
		toObjects[i]->occluded = false;
	}
	for (int i = 0; i < toBuffer.size(); i++) {
		if (physics != NULL && toBuffer[i]->parentQuad != NULL && vectorContains(toBuffer[i]->parentQuad->objects, toBuffer[i]))
			toBuffer[i]->parentQuad->objects.erase(toBuffer[i]->parentQuad->objects.begin() + vectorContainsIndex);
		toBuffer[i]->occluded = true;
	}
}

void GLWindow::occludeTiles() {
	if (camera == NULL)
		return;
	
}

void GLWindow::occludeObject(Object *object) {
	if (!object->occluded) {
		object->occluded = true;
		if (physics != NULL && object->parentQuad != NULL && vectorContains(object->parentQuad->objects, object)) {
   			object->parentQuad->objects.erase(object->parentQuad->objects.begin() + vectorContainsIndex);
			object->parentQuad = NULL;
		}
	}
}

void GLWindow::unOccludeObject(Object *object) {
	if (object->occluded) {
		object->occluded = false;
		if (physics != NULL) {
			physics->quadTree->objects.push_back(object);
			object->parentQuad = physics->quadTree;
		}
	}
}

void GLWindow::addObject(Object *object, GLFWwindow *context) {
	if (context != NULL)
		glfwMakeContextCurrent(context);
	if (object != NULL) {
		objects.push_back(object);
		if (physics != NULL) {
			physics->quadTree->objects.push_back(object);
			object->parentQuad = physics->quadTree;
		}
	}
}

//array structure for object vertData is {x, y, z, r, g, b}, position data is sent to shader with index 0, color 1
Object* GLWindow::addObject(int x, int y, int width, int height, vector3f *color, GLfloat depth, GLenum usage, GLenum drawMode, GLFWwindow *context) {
	if (context != NULL)
		glfwMakeContextCurrent(context);
	else
		glfwMakeContextCurrent(window);
	Object *object;
	GLuint vao, vbo, ebo;
	int indicesSize = 0;
	vector<GLfloat> vertData = getQuadVerts(x, y, width, height, color, false, depth);
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertData.size() * sizeof(GLfloat), &vertData[0], usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	if (drawMode == GL_LINES) {
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, lineIndices.size() * sizeof(GLuint), &lineIndices[0], usage);
		indicesSize = lineIndices.size();
	}
	else {
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, defaultIndices.size() * sizeof(GLuint), &defaultIndices[0], usage);
		indicesSize = defaultIndices.size();
	}
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (GLvoid*)(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	object = new Object(x, y, width, height, this, vao, vbo, objectSh, indicesSize, depth, ebo, drawMode);
	objects.push_back(object);
	updateObjectTransform(object);

	if (physics != NULL) {
		physics->quadTree->objects.push_back(object);
		object->parentQuad = physics->quadTree;
	}
	return object;
}

Sprite* GLWindow::addSprite(int x, int y, int width, int height, const char *texturePath, GLfloat depth, GLenum usage, GLFWwindow *context) {
	if (context != NULL)
		glfwMakeContextCurrent(context);
	else
		glfwMakeContextCurrent(window);
	GLuint vao, vbo, ebo = 0, tex;
	Sprite *sprite;
	vector<GLfloat> vertData = getQuadVerts(x, y, width, height, new vector3f(1, 1, 1), true, depth);
	int texWidth = 0, texHeight = 0;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenTextures(1, &tex);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, (vertData).size() * sizeof(GLfloat), &vertData[0], usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, defaultIndices.size() * sizeof(GLfloat), &defaultIndices[0], usage);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	unsigned char* texture = SOIL_load_image(texturePath, &texWidth, &texHeight, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	sprite = new Sprite(x, y, width, height, this, vao, vbo, tex, spriteSh, defaultIndices.size(), depth, ebo);
	objects.push_back(sprite);
	updateObjectTransform(sprite);

	if (physics != NULL) {
		physics->quadTree->objects.push_back(sprite);
		sprite->parentQuad = physics->quadTree;
	}
	return sprite;
}

Sprite *GLWindow::addSprite(int x, int y, int width, int height, GLuint textureId, GLfloat depth, GLenum usage, GLFWwindow *context) {
	if (context != NULL)
		glfwMakeContextCurrent(context);
	else
		glfwMakeContextCurrent(window);
	GLuint vao, vbo, ebo;
	Sprite *sprite;
	vector<GLfloat> vertData = getQuadVerts(x, y, width, height, new vector3f(1, 1, 1), true, depth);
	int texWidth = 0, texHeight = 0;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, (vertData).size() * sizeof(GLfloat), &vertData[0], usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, defaultIndices.size() * sizeof(GLfloat), &defaultIndices[0], usage);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	sprite = new Sprite(x, y, width, height, this, vao, vbo, textureId, spriteSh, defaultIndices.size(), depth, ebo);
	objects.push_back(sprite);
	updateObjectTransform(sprite);

	if (physics != NULL) {
		physics->quadTree->objects.push_back(sprite);
		sprite->parentQuad = physics->quadTree;
	}
	return sprite;
}

Sprite *GLWindow::addSprite(int x, int y, int width, int height, Atlas *atlas, int tileNum, GLfloat depth, GLenum usage, GLFWwindow *context) {
	if (context != NULL)
		glfwMakeContextCurrent(context);
	else
		glfwMakeContextCurrent(window);
	GLuint vao, vbo, ebo;
	Sprite *sprite;
	vector<GLfloat> vertData = getQuadVerts(x, y, width, height, new vector3f(1, 1, 1), atlas, tileNum, depth);
	int texWidth = 0, texHeight = 0;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, (vertData).size() * sizeof(GLfloat), &vertData[0], usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, defaultIndices.size() * sizeof(GLfloat), &defaultIndices[0], usage);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	sprite = new Sprite(x, y, width, height, this, vao, vbo, atlas->texID, spriteSh, defaultIndices.size(), depth, ebo);
	objects.push_back(sprite);
	updateObjectTransform(sprite);

	if (physics != NULL) {
		physics->quadTree->objects.push_back(sprite);
		sprite->parentQuad = physics->quadTree;
	}
	return sprite;
}

/*Sprite* GLWindow::addTile(int x, int y,const char *texturePath, GLenum usage, GLFWwindow *context) {
	if (context != NULL)
		glfwMakeContextCurrent(context);
	else
		glfwMakeContextCurrent(window);
	GLuint vao, vbo, ebo = 0, tex;
	Sprite *sprite;
	vector<GLfloat> vertData = getQuadVerts(x * tileSize, y * tileSize, tileSize, tileSize, new vector3f(1, 1, 1), true, 0);
	int texWidth = 0, texHeight = 0;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenTextures(1, &tex);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, (vertData).size() * sizeof(GLfloat), &vertData[0], usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, defaultIndices.size() * sizeof(GLfloat), &defaultIndices[0], usage);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	unsigned char* texture = SOIL_load_image(texturePath, &texWidth, &texHeight, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	

	sprite = new Sprite(x * tileSize, y * tileSize, tileSize, tileSize, this, vao, vbo, tex, spriteSh, defaultIndices.size(), 0, ebo);
	tileMap[(y * tileMapWidth) + x] = sprite;
	updateObjectTransform(sprite);

	return sprite;
}

Sprite *GLWindow::addTile(int x, int y, GLuint textureId, GLenum usage, GLFWwindow *context) {
	if (context != NULL)
		glfwMakeContextCurrent(context);
	else
		glfwMakeContextCurrent(window);
	GLuint vao, vbo, ebo;
	Sprite *sprite;
	vector<GLfloat> vertData = getQuadVerts(x * tileSize, y * tileSize, tileSize, tileSize, new vector3f(1, 1, 1), true, 0);
	int texWidth = 0, texHeight = 0;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, (vertData).size() * sizeof(GLfloat), &vertData[0], usage);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, defaultIndices.size() * sizeof(GLfloat), &defaultIndices[0], usage);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
	

	sprite = new Sprite(x * tileSize, y * tileSize, tileSize, tileSize, this, vao, vbo, textureId, spriteSh, defaultIndices.size(), 0, ebo);
	tileMap[(y * tileMapWidth) + x] = sprite;
	updateObjectTransform(sprite);

	return sprite;
}*/

Object *GLWindow::addEmpty(int x, int y, int width, int height) {
	return new Object(x, y, width, height, this, 0, 0, objectSh, defaultIndices.size(), 0);
}

GLuint GLWindow::createTexture(const char* path, int *width, int *height) {
	GLuint tex;
	GLint texWidth, texHeight;
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	unsigned char* texture = SOIL_load_image(path, &texWidth, &texHeight, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (width != NULL)
		*width = texWidth;
	if (height != NULL)
		*height = texHeight;
	return tex;
}

GLuint GLWindow::createTexture(vector<unsigned char> image, int width, int height) {
	GLuint tex;
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	return tex;
}

std::vector<GLfloat> GLWindow::getQuadVerts(int x, int y, int quadWidth, int quadHeight, vector3f *color, bool needsTex, GLfloat depth) const {
	GLfloat delta[] = { 0, 0, 1, 0, 1, 1, 0, 1 };
	vector<GLfloat> result;
	for (int i = 0; i < 4; i++) {
		result.push_back(pixelToFrame(x + delta[i * 2] * quadWidth, width));
		result.push_back(0 - pixelToFrame(y + delta[i * 2 + 1] * quadHeight, height));
		result.push_back(depth);
		result.push_back(color->x);
		result.push_back(color->y);
		result.push_back(color->z);
		if (needsTex) {
			result.push_back(delta[i * 2]);
			result.push_back(delta[i * 2 + 1]);
		}
	}
	return result;
}

std::vector<GLfloat> GLWindow::getQuadVerts(int x, int y, int quadWidth, int quadHeight, vector3f *color, Atlas *atlas, int tileNum, GLfloat depth) const {
	GLfloat delta[] = { 0, 0, 1, 0, 1, 1, 0, 1 };
	vector<GLfloat> texCoords = atlas->getTexCoords(tileNum);
	vector<GLfloat> result;
	for (int i = 0; i < 4; i++) {
		result.push_back(pixelToFrame(x + delta[i * 2] * quadWidth, width));
		result.push_back(0 - pixelToFrame(y + delta[i * 2 + 1] * quadHeight, height));
		result.push_back(depth);
		result.push_back(color->x);
		result.push_back(color->y);
		result.push_back(color->z);
		result.push_back(texCoords[i * 2]);
		result.push_back(texCoords[i * 2 + 1]);
	}
	return result;
}

void GLWindow::updateObjectColor(Object *object, vector3f color) {
	Sprite *spr = dynamic_cast<Sprite*>(object);
	vector<GLfloat> verts = getQuadVerts(object->originalX, object->originalY, object->width, object->height, &color, spr != NULL, object->depth);
	glBindVertexArray(object->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, object->VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, verts.size() * sizeof(GLfloat), &verts[0]);
	glBindVertexArray(0);
}

void GLWindow::updateObjectTransform(Object *object) {
	const Shader *sh = object->shader;
	sh->use();
	GLuint transformLoc = glGetUniformLocation(sh->program, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(object->getTransformValue()));
	glUseProgram(0);
}

void GLWindow::enableUpdate(Object *object, bool enable) {
	if (enable && !vectorContains(activeObjects, object))
		activeObjects.push_back(object);
	else if (!enable && vectorContains(activeObjects, object))
		activeObjects.erase(activeObjects.begin() + vectorContainsIndex);
}

void GLWindow::removeObject(Object *object) {
	if (physics != NULL && object->parentQuad != NULL) {
		if(vectorContains(object->parentQuad->objects, object))
			object->parentQuad->objects.erase(object->parentQuad->objects.begin() + vectorContainsIndex);
	}
	if (vectorContains(objects, object)) {
		glDeleteBuffers(1, &object->VBO);
		glDeleteBuffers(1, &object->EBO);
		glDeleteVertexArrays(1, &object->VAO);
		delete objects[vectorContainsIndex];
		objects.erase(objects.begin() + vectorContainsIndex);
	}

}

void GLWindow::start(int iterations) {
	stopLoop = false;
	if(workerCall != NULL)
		workerThread = thread(&GLWindow::worker, this);
	loop(iterations);
}

void GLWindow::stop() {
	stopLoop = true;
	if(workerThread.joinable())
		workerThread.join();
}

void GLWindow::endHere() {
	stopLoop = true;
	if(workerThread.joinable())
		workerThread.join();
	glfwDestroyWindow(window);
	for (int i = 0; i < objects.size(); i++) {
		glDeleteVertexArrays(1, &(*objects[i]).VAO);
		glDeleteBuffers(1, &(*objects[i]).VBO);
		if ((*objects[i]).EBO > 0)
			glDeleteBuffers(1, &(*objects[i]).EBO);
	}
}

// x, y are in pixel space, width & height are in tile space
TileMap *GLWindow::addTileMap(int x, int y, int width, int height, int tileSize, Atlas *atlas) {
	GLuint vao, vbo, ebo;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glBindVertexArray(vao);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GL_FLOAT), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	Sprite *sp = new Sprite(x, y, this->width, this->height, this, vao, vbo, atlas->texID, spriteSh, 0, 0, ebo);
	TileMap *tm = new TileMap(sp, width, height, tileSize);
	tileMaps.push_back(tm);

	return tm;
}

void GLWindow::setCamera(Object *camera) {
	this->camera = camera;
}

void GLWindow::takeFocus() {
	glfwMakeContextCurrent(window);
}

void GLWindow::setKeyCallback(void(*function)(GLFWwindow*, int, int, int, int)) {
	keyCall = function;
	//glfwMakeContextCurrent(workerContext);
	glfwSetKeyCallback(window, keyCall);
}

void GLWindow::setPhysicsEnvironment(Physics *physics) {
	this->physics = physics;
}

GLfloat GLWindow::pixelToFrame(int value, int dimension) {
	return ((GLfloat)value / (GLfloat)dimension) * 2 - 1;
}

int GLWindow::frameToPixel(GLfloat value, int dimension) {
	return ((value + 1) / 2) * dimension;
}

GLWindow::~GLWindow()
{
	//glfwDestroyWindow(window);
}
