#version 460

in vec2 FragTexCoord;

uniform sampler2D LightMap;
uniform sampler2D texture0;

uniform vec3 Light;
uniform vec2 Size;

out vec4 FinalColor;

const float MAXRADIUS = 65535.0f;
const float TAU = 6.2831853071795864769252867665590;

void main()
{
    vec2 in_TexCenter = vec2((Size.xy * 0.5));
    vec2 Coord = FragTexCoord * Size;
    vec2 Delta = Coord - Light.xy;
    float RayCount = TAU * Light.z;
    float RayIndex = floor((RayCount * fract(atan(-Delta.y, Delta.x)/TAU)) + 0.5);
    vec2 RayPos = vec2(mod(RayIndex, Size.x), RayIndex / Size.y) * (1./Size.x);
    vec2 TexRay = texture2D(LightMap, RayPos).rg;
    float Distance = distance(Coord, Light.xy);
    float RayLength = clamp(TexRay.r + (TexRay.g / 255.0), 0.0, 1.0) * Light.z;
    float RayVisible = sign(RayLength - Distance) * (1. - texture2D(texture0, (Light.xy + Delta) * Size).a);
    float ToneMap = 1. - (Distance/Light.z);

    vec4 color = texture2D(texture0, FragTexCoord);
    //vec4 color = vec4(1., 1., 1., 1.);
    FinalColor = vec4(color.xyz * ToneMap, RayVisible);
}

