#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;

out vec3 vertexColor;

uniform mat4 transform = mat4(1.0);
uniform mat4 cameraTransform = mat4(1.0);

void main()
{
	gl_Position = cameraTransform * transform * vec4(pos, 1.0);
	vertexColor = color;
}