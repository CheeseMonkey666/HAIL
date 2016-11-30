#version 330 core

in vec3 vertexColor;
in vec2 texCoords;

out vec4 color;

uniform sampler2D tex;

void main()
{
 vec4 texColor = texture(tex, texCoords) * vec4(vertexColor, 1);
 if(texColor.a < 0.1)
	discard;
color = texColor;
}