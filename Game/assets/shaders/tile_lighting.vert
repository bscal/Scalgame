#version 460 core

in vec3 vertexPosition;
in vec2 vertexTexCoord;

uniform mat4 mvp;

out vec2 FragTexCoord;

void main()
{
	FragTexCoord = vertexTexCoord;
	gl_Position = mvp * vec4(vertexPosition, 1.0);
}