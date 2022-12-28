#version 460

in vec2 FragTexCoord;

uniform float transmit=0.99;// light transmition coeficient <0,1>
uniform int txrsiz=512;     // max texture size [pixels]
uniform sampler2D LightMap; // texture unit for light map
uniform vec3 Light;
uniform vec2 Size;

out vec4 FinalColor;

//uniform vec2 t0;            // texture start point (mouse position) <0,1>
//in smooth vec2 t1;          // texture end point, direction <0,1>

void main()
{
    vec2 dir = FragTexCoord;
    int i;
    vec2 t,dt;
    vec4 c0,c1;
    dt=normalize(dir-Light.xy)/float(Size.x/16.0);
    c0=vec4(1.0,1.0,1.0,1.0);   // light ray strength
    t=Light.xy;
    if (dot(dir-t,dt)>0.0)
     for (i=0;i<(Size.x/16.0);i++)
        {
        c1=texture2D(LightMap,t);
        c0.rgb*=((c1.a)*(c1.rgb))+((1.0f-c1.a)*transmit);
        if (c1.a < 1.) break;
        if (dot(dir-t,dt)<=0.000f) break;
        if (c0.r+c0.g+c0.b<=0.001f) break;
        t+=dt;
        }
    FinalColor=0.90*c0+0.10*texture2D(LightMap,dir);  // render with ambient light
    //FinalColor=c0;                                 // render without ambient light
 }