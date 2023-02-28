#version 430 core

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

out vec4 finalColor;

void main()
{
	vec4 texColor = texture(texture0, fragTexCoord);
	finalColor = texColor * fragColor;
}
