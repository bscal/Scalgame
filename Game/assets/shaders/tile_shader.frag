#version 460

in vec2 FragTexCoord;
in vec4 FragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 FinalColor;

void main()
{
	vec4 texColor = texture(texture0, FragTexCoord);
	FinalColor = texColor * colDiffuse * FragColor;
}
