#version 460

in vec2 FragTexCoord;
//in vec4 FragColor;

//uniform sampler2D texture0;
//uniform vec4 colDiffuse;
uniform vec3 Light;
//uniform vec2 World;

#define MAXRADI_SIZE 64.0
#define PI 3.141592

const float RAYTEXT_SIZE = 32.0;
const vec3 texData = vec3(RAYTEXT_SIZE, RAYTEXT_SIZE * 0.5, 
	1.0 / RAYTEXT_SIZE);
const vec2 texCenter = vec2((RAYTEXT_SIZE * 0.5) + 0.5);
const float PI2 = 2.0 * PI;

out vec4 FinalColor;


void main()
{
    vec3 diff = vec3(FragTexCoord - Light.xy, Light.z);
	float strength = 1.0 / (length(diff) - Light.z);
	FinalColor = vec4(strength);
}

