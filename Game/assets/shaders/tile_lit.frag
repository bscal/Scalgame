#version 430 core

in vec2 fragTexCoord;

uniform sampler2D texture0; // World Map
uniform sampler2D texture1; // Light Map

out vec4 finalColor;

void main()
{
	vec4 texColor = texture(texture0, fragTexCoord);
	vec4 lightmapColor = texture(texture1, fragTexCoord);
	vec3 color = texColor.rgb * lightmapColor.rgb;
	finalColor = vec4(color, 1.0);
}