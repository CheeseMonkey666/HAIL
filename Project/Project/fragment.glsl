#version 450

in vec3 colour;

out vec4 colorOut;

void main () {
  colorOut = vec4(colour, 1.0);
}