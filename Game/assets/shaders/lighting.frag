#version 460 core

in vec4 fragColor;

uniform float lightIntensity = 1.0;
uniform vec4 sunLightColor = vec4(0.0);

out vec4 finalColor;

void main()
{
	// TODO maybe use light alpha to add sunlight, //indoor, outdoor,
	vec4 color = (fragColor * lightIntensity) + sunLightColor;
	finalColor = color;
}