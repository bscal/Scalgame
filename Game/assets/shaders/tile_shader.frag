#version 460

in vec2 FragTexCoord;
in vec4 FragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

#define LIGHT_MAP_CAPACITY 48 * 32
uniform int LightMapCount = LIGHT_MAP_CAPACITY;
uniform int LightMap[LIGHT_MAP_CAPACITY];

out vec4 FinalColor;


void main()
{
	vec4 texColor = texture(texture0, FragTexCoord);

	FinalColor = texColor * colDiffuse * FragColor;
}
