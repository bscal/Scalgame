#version 430 core

in vec2 fragTexCoord;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float bloomIntensity = 1.0;

out vec4 finalColor;

void main()
{  
	vec4 lightmapColor = texture(texture0, fragTexCoord);
	vec4 blurColor = texture(texture1, fragTexCoord);

	vec3 result = lightmapColor.rgb + (blurColor.rgb * bloomIntensity);
	finalColor = vec4(result, 1.0);
}