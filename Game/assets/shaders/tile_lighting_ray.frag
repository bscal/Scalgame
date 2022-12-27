#version 460

in vec2 FragTexCoord;

uniform sampler2D Tiles;
uniform vec3 Light;
uniform vec2 Size;

out vec4 FinalColor;

const float MAXRADIUS = 65535.0;
const float TAU = 6.2831853071795864769252867665590;

void main()
{
	vec2 in_RayTexSize = vec2(Size.x);

	// Converts the current pixel's coordinate from UV to XY space.
	vec2 Coord = FragTexCoord * in_RayTexSize,
		// Takes the pixel's XY position, converts it to a vec2(1D-array index, ray count).
		xyRay = vec2((Coord.x * in_RayTexSize.x) + Coord.y, TAU * Light.z);
	// Takes the index/ray_count and converts it to an angle in range of: 0 to 2pi = 0 to ray_count.
	float Theta = TAU * (xyRay.x / xyRay.y);
	// Gets the lengthdir_xy polar cooridinate around the light's center.
	vec2 Delta = vec2(cos(Theta), -sin(Theta));
	// "Step" gets checks whether the current ray index < ray count, if not the ray is not traced (for-loop breaks).
	for(float v = step(xyRay.x,xyRay.y), d = 0.; d < MAXRADIUS * v; d++) {
		/*
			"in_Light.z < d" Check if the current ray distance(length) "d" is > light radius (if so, then break).
			"d + in_Light.z * texture2D(...)" If collision in the world map at distance "d" is found, the ray ends
			(add light radius to d to make it greater than the light radius to break out of the for-loop.
		*/
		if (Light.z < d + Light.z * texture2D(Tiles, (Light.xy + (xyRay = Delta * d)) * (Size / 16.)).a) break;
	}
	// Converts the ray length to polar UV coordinates ray_length / light_radius.
	float rayLength = length(xyRay) / Light.z;
	// Takes the length of the current ray and splits it into two bytes and stores it in the texture.
	FinalColor = vec4(vec2(floor(rayLength * 255.0) / 255.0, fract(rayLength * 255.0)), 0.0, 1.0);
}

