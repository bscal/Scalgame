#version 430 core

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D texture1;

uniform vec4 ambientLightColor = vec4(.1, .1, .2, 1.0);

out vec4 finalColor;

void main()
{
	vec4 texColor = texture(texture0, fragTexCoord);
	vec4 lightmapColor = texture(texture1, fragTexCoord);

	vec3 lightColor = texColor.rgb * lightmapColor.rgb;
	vec3 ambientColor = texColor.rgb * ambientLightColor.rgb;
	finalColor = vec4(lightColor + ambientColor, 1.0);
}