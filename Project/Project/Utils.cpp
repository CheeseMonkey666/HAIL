#define _CRT_SECURE_NO_WARNINGS

#include "Utils.h"
#include <GL/glew.h>
#include <GL/GLU.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

using namespace std;

unsigned int lastModelSize = 0;

bool loadOBJ(const char * path, GLuint &vertexBuffer, GLuint &uvBuffer, GLuint &normalBuffer, int &bufferSize) {
	vector<vector3f> tempVerts, vertices, tempNorms, normals;
	vector<vector2f> tempUVs, uvs;
	vector<unsigned int> vertInd, UVInd, normInd;
	FILE * file;
	fopen_s(&file, path, "rb");
	if (file == NULL) {
		printf("No file found\n");
		return false;
	}
	printf("Opening file\n");
	while (true) {
		char header[128];
		int res = fscanf(file, "%s", header);
		if (res == EOF)
			break;

		if (strcmp(header, "v") == 0) {
			vector3f vert;
			fscanf(file, "%f%f%f\n", &vert.x, &vert.y, &vert.z);
			tempVerts.push_back(vert);
		}
		else if (strcmp(header, "vt") == 0) {
			vector2f uv;
			fscanf(file, "%f%f\n", &uv.x, &uv.y);
			tempUVs.push_back(uv);
		}
		else if (strcmp(header, "vn") == 0) {
			vector3f norm;
			fscanf(file, "%f%f%f\n", &norm.x, &norm.y, &norm.z);
			tempNorms.push_back(norm);
		}
		else if (strcmp(header, "f") == 0) {
			string v1, v2, v3;
			unsigned int indexV[3], indexU[3], indexN[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &indexV[0], &indexU[0], &indexN[0], &indexV[1], &indexU[1], &indexN[1], &indexV[2], &indexU[2], &indexN[2]);
			if (matches != 9) {
				return false;
			}
			vertInd.push_back(indexV[0]);
			vertInd.push_back(indexV[1]);
			vertInd.push_back(indexV[2]);
			UVInd.push_back(indexU[0]);
			UVInd.push_back(indexU[1]);
			UVInd.push_back(indexU[2]);
			normInd.push_back(indexN[0]);
			normInd.push_back(indexN[1]);
			normInd.push_back(indexN[2]);
		}
	}
	for (unsigned int i = 0; i < vertInd.size(); i++) {
		unsigned int index = vertInd[i];
		vector3f v = tempVerts[index - 1];
		vertices.push_back(v);
		index = normInd[i];
		vector3f n = tempNorms[index - 1];
		normals.push_back(n);
		index = UVInd[i];
		vector2f u = tempUVs[index - 1];
		uvs.push_back(u);
	}
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vector3f), &vertices[0], GL_STATIC_DRAW);
	printf("Vertices bound to buffer #%d\n", vertexBuffer);
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vector3f), &normals[0], GL_STATIC_DRAW);
	glGenBuffers(1, &uvBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vector2f), &uvs[0], GL_STATIC_DRAW);
	lastModelSize = vertices.size();
	printf("Model succesfully loaded, vertex count: %d\n", vertices.size());
	bufferSize = vertices.size() * sizeof(float);
	return true;
}

/*Generates randomized colour data for the LAST LOADED MODEL*/
void generateColorsf(GLuint & colorBuffer) {
	vector3f v;
	vector<vector3f> temp;
	for (int i = 0; i < lastModelSize; i++) {
		v.x = static_cast <float> (rand()) / (float)RAND_MAX;
		v.y = static_cast <float> (rand()) / (float)RAND_MAX;
		v.z = static_cast <float> (rand()) / (float)RAND_MAX;
		temp.push_back(v);
	}
	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, temp.size() * sizeof(vector3f), &temp[0], GL_STATIC_DRAW);
}

bool createShader(const char * path, GLenum type, GLuint & shader) {
	char *code;
	FILE * file;
	fopen_s(&file, path, "rb");
	if (file == NULL) {
		printf("No file found\n");
		return false;
	}
	fseek(file, 0, SEEK_END);
	GLint fsize = ftell(file);
	fseek(file, 0, SEEK_SET);
	code = (char *)malloc(fsize + 1);
	fread(code, fsize, 1, file);
	fclose(file);
	code[fsize] = NULL;

	shader = glCreateShader(type);
	glShaderSource(shader, 1, &code, NULL);
	glCompileShader(shader);
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	printf("%d\n", success);
	printf("Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	return true;
}

void loadTestCube(GLuint &vertexBuffer, GLuint &colorBuffer) {
	const GLfloat vertBuffer[] = {
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f,-1.0f, 1.0f
	};
	const GLfloat colorArray[] = {
		0.583f,  0.771f,  0.014f, 
		0.609f,  0.115f,  0.436f, 
		0.327f,  0.483f,  0.844f, 
		0.822f,  0.569f,  0.201f, 
		0.310f,  0.747f,  0.185f, 
		0.597f,  0.770f,  0.761f, 
		0.559f,  0.436f,  0.730f, 
		0.359f,  0.583f,  0.152f, 
		0.483f,  0.596f,  0.789f, 
		0.559f,  0.861f,  0.639f, 
		0.195f,  0.548f,  0.859f, 
		0.014f,  0.184f,  0.576f, 
		0.771f,  0.328f,  0.970f, 
		0.406f,  0.615f,  0.116f, 
		0.676f,  0.977f,  0.133f, 
		0.971f,  0.572f,  0.833f, 
		0.140f,  0.616f,  0.489f, 
		0.997f,  0.513f,  0.064f, 
		0.945f,  0.719f,  0.592f, 
		0.543f,  0.021f,  0.978f, 
		0.279f,  0.317f,  0.505f, 
		0.167f,  0.620f,  0.077f, 
		0.347f,  0.857f,  0.137f, 
		0.055f,  0.953f,  0.042f, 
		0.714f,  0.505f,  0.345f, 
		0.783f,  0.290f,  0.734f, 
		0.722f,  0.645f,  0.174f, 
		0.302f,  0.455f,  0.848f, 
		0.225f,  0.587f,  0.040f, 
		0.517f,  0.713f,  0.338f, 
		0.053f,  0.959f,  0.120f, 
		0.393f,  0.621f,  0.362f, 
		0.673f,  0.211f,  0.457f, 
		0.820f,  0.883f,  0.371f, 
		0.982f,  0.099f,  0.879f,
		0.982f,  0.099f,  0.879f
	};
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertBuffer), vertBuffer, GL_STATIC_DRAW);
	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorArray), colorArray, GL_STATIC_DRAW);
}

void printShaderInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n", infoLog);
		free(infoLog);
	}
}

void printProgramInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n", infoLog);
		free(infoLog);
	}
}