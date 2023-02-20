#version 460 core

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;

uniform mat4 mvp;

uniform float TargetWidth;
uniform int IsHorizontal;
out float Width;
out int IsH;
out vec2 FragTexCoord;
//const int HalfBlurWidth = 5;
//out vec2 BlurTextureCoords[11];

void main()
{
	gl_Position = mvp * vec4(vertexPosition, 1.0);
	Width = TargetWidth;
	IsH = IsHorizontal;
	FragTexCoord = vertexTexCoord;

}