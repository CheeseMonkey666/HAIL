#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texC;

out vec3 vertexColor;
out vec2 texCoords;

uniform mat4 transform = mat4(1);
uniform mat4 cameraTransform = mat4(1);

void main()
{
	gl_Position = cameraTransform * transform * vec4(pos, 1.0);
	vertexColor = color;
	texCoords = vec2(texC.x, texC.y);
}