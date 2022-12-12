#version 460 core

in vec3 vertexPosition;
in vec2 vertexTexCoord;

uniform mat4 mvp;
uniform vec2 Scale;

out vec2 FragTexCoord;

void main()
{
	FragTexCoord = vertexTexCoord;
	gl_Position = mvp * vec4(vertexPosition.xyz, 1.0);
}