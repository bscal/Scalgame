#version 430 core

in vec2 fragTexCoord;

uniform sampler2D texture0; // WorldMap
uniform sampler2D texture1; // LightMap/Ray

uniform vec3 lightPos;
uniform vec3 lightColor;

uniform vec2 lightCenter;
uniform float rayTexSize;

const float MAXRADIUS = 65535.0;
const float TAU = 6.2831853071795864769252867665590;

out vec4 finalColor;

// Custom tone map function, adjust as you please, keep in range 0 to 1.
float ToneMapFunc(float d, float m) {
	return clamp(1. - (d/m), 0., 1.);
}

void main()
{
	vec2 worldTexSize = textureSize(texture0, 0);
	vec2 lightTexSize = textureSize(texture1, 0);
	// Gets the current pixel's texture XY coordinate from it's texture UV coordinate.
	vec2 coord = fragTexCoord * lightTexSize;
		// Gets the lengthdir_xy of the current pixel in reference to the light position.
	vec2 delta = coord - lightCenter;
	// Gets the ray count as equal to the light's circumference.
	float rayCount = TAU * lightPos.z;
		// Gets the index of the closest ray pointing towards this pixel within the ray texture.
	float rayIndex = floor((rayCount * fract(atan(-delta.y, delta.x)/TAU)) + 0.5);
	// Gets the position of the closest ray pointing towards this pixel within the ray texture.
	vec2 rayPos = vec2(mod(rayIndex, rayTexSize), rayIndex / rayTexSize) * (1./rayTexSize);
		// Gets the closest ray associated with this pixel.
	vec2 texRay = texture2D(texture1, rayPos).rg;
	// Gets the distance from the current pixel to the light center.
	float dist = distance(coord, lightCenter);
		// Reads out the length fo the ray itself.
	float rayLength = clamp(texRay.r + (texRay.g / 255.0), 0.0, 1.0) * lightPos.z;
		// Returns a bool whether or not this pixel is within the ray.
	float rayVisible = sign(rayLength - dist) * (1. - texture2D(texture0, (lightPos.xy + delta) * worldTexSize).a);
		// Gets the gradient/tone map based on distance from the pixel to the light.
	float toneMap = ToneMapFunc(dist, lightPos.z);
	
	// Draw the final pixel output with the source and destination color lerp'd together, then apply the gradient/tonemap.
	//finalColor = vec4(mix(in_ColorD, in_ColorS, vec3(toneMap)) * toneMap, toneMap * rayVisible);
	finalColor = vec4(lightColor * toneMap, toneMap * rayVisible);
}
