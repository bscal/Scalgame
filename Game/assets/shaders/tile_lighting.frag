#version 460

in vec2 FragTexCoord;

uniform sampler2D Tiles;
uniform vec3 Light;
uniform vec2 Offset;
uniform int TileCount;
uniform int TilesWidth;

out vec4 FinalColor;

void main()
{
	vec4 c = texture2D(Tiles, gl_FragCoord.xy / vec2(1600., 900.));
	FinalColor = c;
}

