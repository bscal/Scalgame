#version 430 core

in vec3 vertexPosition;
in vec2 vertexTexCoord;

uniform mat4 mvp;

//uniform vec2 viewOffset;
//uniform vec2 viewportSize;
//uniform vec2 inverseTileTextureSize;
//uniform float inverseTileSize;

//out vec2 pixelCoord;
out vec2 texCoord;

void main()
{
	gl_Position = mvp * vec4(vertexPosition, 1.0);
	texCoord = vertexTexCoord;
    //pixelCoord = (vertexTexCoord * viewportSize) + viewOffset;
    //texCoord = pixelCoord * inverseTileTextureSize * inverseTileSize;
}