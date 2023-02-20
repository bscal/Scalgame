#version 460 core

in vec2 FragTexCoord;
in float Width;
in int IsH;

const int HalfBlurWidth = 5;
vec2 BlurTextureCoords[11];

uniform sampler2D texture0;
 
//uniform float TargetWidth;
//uniform int IsHorizontal;

out vec4 FinalColor;

void main()
{  
	vec2 centerTexCoord = FragTexCoord * 0.5 + 0.5;

	float pixelSize = 1.0 / Width;
	float fi = -5.;
	for (int i = -HalfBlurWidth; i <= HalfBlurWidth; ++i)
	{
		vec2 offset = (IsH == 0) ? vec2(0., pixelSize * i) : vec2(pixelSize * i, 0.);
		BlurTextureCoords[i + HalfBlurWidth] = FragTexCoord + offset;
		fi += 1.;
	}

	FinalColor = vec4(0.0);
	FinalColor += texture(texture0, BlurTextureCoords[0]) * 0.0093;
    FinalColor += texture(texture0, BlurTextureCoords[1]) * 0.028002;
    FinalColor += texture(texture0, BlurTextureCoords[2]) * 0.065984;
    FinalColor += texture(texture0, BlurTextureCoords[3]) * 0.121703;
    FinalColor += texture(texture0, BlurTextureCoords[4]) * 0.175713;
    FinalColor += texture(texture0, BlurTextureCoords[5]) * 0.198596;
    FinalColor += texture(texture0, BlurTextureCoords[6]) * 0.175713;
    FinalColor += texture(texture0, BlurTextureCoords[7]) * 0.121703;
    FinalColor += texture(texture0, BlurTextureCoords[8]) * 0.065984;
    FinalColor += texture(texture0, BlurTextureCoords[9]) * 0.028002;
    FinalColor += texture(texture0, BlurTextureCoords[10]) * 0.0093;
}