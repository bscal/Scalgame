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
    vec2 coord = floor(FragTexCoord * Size);
    vec2 xyRay = vec2(coord.x + (coord.y * Size.x), TAU * Light.z);
    float theta = TAU * (xyRay.x / xyRay.y);
    vec2 delta = vec2(cos(theta), -sin(theta));

    float Validated = step(xyRay.x, xyRay.y);
    for(float d = 0.; d < MAXRADIUS * Validated; d++)
    {
        if (Light.z < d + Light.z * texture2D(Tiles, (vec2(Light.z, Light.z) + xyRay) * Size).a) break;
        xyRay = floor(delta * d + 0.5);
    }

    float rayLength = length(xyRay) / Light.z;
    FinalColor = vec4(vec2(floor(rayLength * 255.0) / 255.0, fract(rayLength * 255.0)), 0.0, 1.0);
}

