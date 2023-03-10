#version 430

in vec2 fragTexCoord;

uniform sampler2D texture0; // The texture of the game's world/collision map.
uniform vec3 light; // X, Y and Z (radius) of the light that is being ray-traced.
uniform vec2 world; // Size of the world/collision texture we're tracing rays against.
uniform float rayTexSize; // Size of the texture that the rays are being stored on.

const float MAXRADIUS = 65535.; // Maximum ray-length of 2 bytes, 2^16-1.
const float TAU = 6.2831853071795864769252867665590; // TAU or 2 * pi (shortcut for radial.circular math).

out vec4 finalColor;

void main()
{
	vec2 rayTexSize = textureSize(texture0, 0);
	// Converts the current pixel's coordinate from UV to XY space.
	vec2 coord = floor(fragTexCoord * rayTexSize);
		// Takes the pixel's XY position, converts it to a vec2(1D-array index, ray count).
	vec2 xyRay = vec2((coord.y * rayTexSize.y) + coord.x, TAU * light.z);
	// Takes the index/ray_count and converts it to an angle in range of: 0 to 2pi = 0 to ray_count.
	float theta = TAU * (xyRay.x / xyRay.y);
	// Gets the lengthdir_xy polar cooridinate around the light's center.
	vec2 delta = vec2(cos(theta), -sin(theta));
	// "Step" gets checks whether the current ray index < ray count, if not the ray is not traced (for-loop breaks).
	for(float v = step(xyRay.x,xyRay.y), d = 0.; d < MAXRADIUS * v; d++)
		/*
			"in_Light.z < d" Check if the current ray distance(length) "d" is > light radius (if so, then break).
			"d + in_Light.z * texture2D(...)" If collision in the world map at distance "d" is found, the ray ends
			(add light radius to d to make it greater than the light radius to break out of the for-loop.
		*/
		if (light.z < d + light.z * texture2D(texture0, (light.xy + (xyRay = delta * d)) * world).a) break;
	// Converts the ray length to polar UV coordinates ray_length / light_radius.
	float rayLength = length(xyRay) / light.z;
	// Takes the length of the current ray and splits it into two bytes and stores it in the texture.
	finalColor = vec4(vec2(floor(rayLength * 255.0) / 255.0, fract(rayLength * 255.0)), 0.0, 1.0);
}

