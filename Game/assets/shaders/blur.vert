#version 430 core

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;

uniform mat4 mvp;

//uniform float targetWidth;
//uniform int isHorizontal;

//out vec2 blurTextureCoords[11];
out vec2 TexCoords;

void main()
{
	gl_Position = mvp * vec4(vertexPosition, 1.0);
	TexCoords = vertexTexCoord;
	//vec2 centerTexCoord = vertexTexCoord * 0.5 + 0.5;



}