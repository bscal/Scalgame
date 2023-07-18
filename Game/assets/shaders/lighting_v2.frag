#version 430 core

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0; //tileColorTexture
uniform sampler2D tileDataTexture;

uniform vec3 sunlightColor;
uniform vec3 losColor;

out vec4 finalColor;

void main()
{
	vec4 tileColor = texture(texture0, fragTexCoord);
	vec4 tileData = texture(tileDataTexture, fragTexCoord);

	float hasLos = float(tileData.r > 0.0);
	float hasCeilingFactor = float(tileData.g > 0.0);
	float intensity = fragColor.a;

	// TODO
	// make sunlght los vec4, update uis
	// new uniform for intensity?
	// seperate alpha for fragColor (ambientLight)
	// los fade out?
	// los dithering?

	vec4 light = tileColor * intensity;
	vec4 ambientLight = vec4(fragColor.rgb, 1.0);
	vec4 sunLight = vec4(sunlightColor * hasCeilingFactor, 0.0);
	//vec4 losLight = vec4(losColor * hasLos, 1.0);

	finalColor = (light + ambientLight + sunLight) * hasLos;
	//finalColor = tileData;
}