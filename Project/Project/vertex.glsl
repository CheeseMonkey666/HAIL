#version 450

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_colour;

layout (std140) uniform Matrices {
    mat4 projModelViewMatrix;
};

out vec3 colour;
out vec4 gl_Position;

void main () {
  colour = vertex_colour;
  gl_Position = projModelViewMatrix * vec4(vertex_position, 1.0);
}