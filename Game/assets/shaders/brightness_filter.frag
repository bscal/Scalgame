#version 430 core

#define USE_CUTOFF 0

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform float brightnessThreshold = 0.8;

out vec4 finalColor;

void main()
{
	vec4 texColor = texture(texture0, fragTexCoord);
	float brightness = dot(texColor.rgb, vec3(0.2126, 0.7152, 0.0722));
#if USE_CUTOFF
	if (brightness > brightnessThreshold)
		finalColor = vec4(texColor.rgb, 1.0);
	else
		finalColor = vec4(0.0, 0.0, 0.0, 1.0);
#else
	finalColor = texColor * brightness;
#endif
}
