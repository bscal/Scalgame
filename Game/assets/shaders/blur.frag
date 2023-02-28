#version 430 core

out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D texture0;
  
uniform int isHorizontal;

const float weight[5] = float[] (0.175713, 0.121703, 0.065984, 0.028002, 0.0093);
const float weightBase = 0.198596;

void main()
{             
    vec2 tex_offset = 1.0 / textureSize(texture0, 0); // gets size of single texel
    vec3 result = texture(texture0, TexCoords).rgb * weightBase;
    if(isHorizontal == 1)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(texture0, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(texture0, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(texture0, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(texture0, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}