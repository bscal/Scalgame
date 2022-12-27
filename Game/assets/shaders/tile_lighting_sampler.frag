#version 460

in vec2 FragTexCoord;

uniform sampler2D LightMap;
uniform sampler2D texture0;

uniform vec3 Light;
uniform vec2 Size;

out vec4 FinalColor;

const float MAXRADIUS = 65535.0f;
const float TAU = 6.2831853071795864769252867665590;

// Custom tone map function, adjust as you please, keep in range 0 to 1.
float ToneMapFunc(float d, float m) {
	return clamp(1. - (d/m), 0., 1.);
}

void main() {
	// Gets the current pixel's texture XY coordinate from it's texture UV coordinate.
	vec2 Coord = FragTexCoord * Size,
		// Gets the lengthdir_xy of the current pixel in reference to the light position.
		Delta = Coord - Light.xy;
	// Gets the ray count as equal to the light's circumference.
	float RayCount = TAU * Light.z,
		// Gets the index of the closest ray pointing towards this pixel within the ray texture.
		RayIndex = floor((RayCount * fract(atan(-Delta.y, Delta.x)/TAU)) + 0.5);
	// Gets the position of the closest ray pointing towards this pixel within the ray texture.
	vec2 RayPos = vec2(mod(RayIndex, Size.x), RayIndex / Size.x) * (1./Size.x);
		// Gets the closest ray associated with this pixel.
	vec2 TexRay = texture2D(LightMap, RayPos).rg;
	// Gets the distance from the current pixel to the light center.
	float Distance = distance(Coord, Light.xy),
		// Reads out the length fo the ray itself.
		RayLength = clamp(TexRay.r + (TexRay.g / 255.0), 0.0, 1.0) * Light.z,
		// Returns a bool whether or not this pixel is within the ray.
		RayVisible = sign(RayLength - Distance) * (1. - texture2D(texture0, (Light.xy + Delta) * Size).a),
		// Gets the gradient/tone map based on distance from the pixel to the light.
		ToneMap = ToneMapFunc(Distance, Light.z);
	
	vec4 color = texture(texture0, FragTexCoord);
	// Draw the final pixel output with the source and destination color lerp'd together, then apply the gradient/tonemap.
	FinalColor = vec4(color.xyz * ToneMap, ToneMap * RayVisible);
}

