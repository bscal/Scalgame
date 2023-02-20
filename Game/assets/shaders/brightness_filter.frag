#version 430 core

#define USE_CUTOFF 1

in vec2 FragTexCoord;
in vec4 FragColor;

uniform sampler2D texture0;

uniform float BrightnessThreshold = 0.9;

out vec4 FinalColor;

void main()
{
	vec4 texColor = texture(texture0, FragTexCoord);
	float brightness = dot(texColor.rgb, vec3(0.2126, 0.7152, 0.0722));
#if USE_CUTOFF
	if (brightness > BrightnessThreshold)
		FinalColor = texColor;
	else
		FinalColor = vec4(0.);
#else
	FinalColor = texColor * brightness;
#endif
}
