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

	float hasCeilingFactor = 1.0 - clamp(tileData.z * 255.0, 0.0, 1.0);
	float losFactor = tileData.w;

	vec3 color = lightColor.rgb * fragColor.a;
	color += fragColor.rgb;
	color += sunlightColor * hasCeilingFactor;

	if (losFactor > 0)
		finalColor = vec4(color, 1.0);
	else
		finalColor = vec4(losColor, 1.0);
}