#version 460

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
//uniform sampler2D texture1;

uniform vec4 ambientLightColor = vec4(.1, .1, .2, 1.0);

out vec4 FinalColor;

void main()
{
	vec4 occultionMap = texture(texture0, fragTexCoord);

	//vec4 texColor = texture(texture0, fragTexCoord);
	//vec4 lightmapColor = texture(texture1, fragTexCoord);

	//vec4 lightColor = texColor * lightmapColor;
	//vec4 ambientColor = texColor * ambientLightColor;
	//FinalColor = texColor + lightColor + ambientColor;
	//FinalColor.a = min(FinalColor.a, 1.0);
}