#version 430 core

in vec4 fragColor;

uniform float lightIntensity = 1.0;
uniform vec4 sunLightColor = vec4(0.0);

out vec4 finalColor;

void main()
{
	vec3 sunColor = sunLightColor.rgb * fragColor.w;
	finalColor = vec4((fragColor.rgb * lightIntensity) + sunColor, 1.0);
}