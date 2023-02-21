#version 460 core

in vec3 vertexPosition;
in vec4 vertexColor;

uniform mat4 mvp;

out vec4 fragColor;

void main()
{
	gl_Position = mvp * vec4(vertexPosition, 1.0);
	fragColor = vertexColor;
}