#version 460 core

in vec3 vertexPosition;
in vec2 vertexTexCoord;

uniform mat4 mvp;

out vec2 fragTexCoord;

void main()
{
	gl_Position = mvp * vec4(vertexPosition.xyz, 1.0);
	fragTexCoord = vertexTexCoord;
}