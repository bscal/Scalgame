#version 430 core

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0; // Color map

uniform vec4 sunLightColor = vec4(0.0);
uniform float lightIntensity = 1.0;

out vec4 finalColor;

void main()
{
	vec4 color = texture(texture0, fragTexCoord);
	color *= lightIntensity;
	//color += fragColor;
	color.a = 1.0;
	finalColor = color;
}