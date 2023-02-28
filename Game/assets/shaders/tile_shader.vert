#version 430 core

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

uniform mat4 mvp;

out vec2 fragTexCoord;
out vec4 fragColor;

void main()
{
	gl_Position = mvp * vec4(vertexPosition, 1.0);
	fragTexCoord = vertexTexCoord;
	fragColor = vertexColor;
}