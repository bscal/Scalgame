#version 460 core

// Input vertex attributes
in vec3 vertexPosition;
//in vec2 vertexTexCoord;
//in vec3 vertexNormal;
//in vec4 vertexColor;

uniform mat4 mvp;

out vec2 FragTexCoord;
//out vec4 FragColor;

void main()
{
	FragTexCoord = vertexPosition.xy;
	//FragColor = vertexColor;

	gl_Position = mvp * vec4(vertexPosition, 1.0);
}