#version 460

in vec2 FragTexCoord;
in vec4 FragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 FinalColor;

void main()
{
	vec4 texColor = texture(texture0, FragTexCoord);
	vec4 col = texColor * colDiffuse * FragColor;
	//FinalColor = texColor * colDiffuse * vec4(FragColor.xyz, min(1.0, FragColor.a));
	FinalColor = col;
}
