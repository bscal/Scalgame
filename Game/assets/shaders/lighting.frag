#version 460

in vec2 FragTexCoord;
in vec4 FragColor;

uniform sampler2D texture0;

uniform float BrightPassThreshold = 0.8;

out vec4 FinalColor;

void main()
{
	vec3 luminanceVector = vec3(0.2125, 0.7154, 0.0721);
    vec4 c = texture2D(texture0, FragTexCoord.st) * FragColor;

    float luminance = dot(luminanceVector, c.xyz);
    luminance = max(0.0, luminance - BrightPassThreshold);
    c.xyz *= sign(luminance);
    c.a = 1.0;

    FinalColor = c;
}