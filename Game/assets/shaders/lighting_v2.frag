#version 430 core

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0; // Color map
uniform sampler2D tileDataMap; // World map

uniform vec3 sunlightColor;
uniform vec3 losColor;

out vec4 finalColor;

void main()
{
	vec4 lightColor = texture(texture0, fragTexCoord);
	vec4 tileData = texture(tileDataMap, fragTexCoord);

	vec3 color = lightColor.rgb * fragColor.a;
	color += fragColor.rgb;
	color += sunlightColor;

	if (tileData.a > 0)
		finalColor.rgb = color;
	else
		finalColor.rgb = losColor;

	finalColor.a = 1.0;
}