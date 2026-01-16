#version 330 core
#include "fractal_noise.glsl"

layout(location = 0) out vec4 FragColor;

in vec2 v_TexCoord;
in float v_TexIndex;

uniform sampler2D u_Textures[32];

void main()
{
    int index = int(v_TexIndex);
    vec4 texColor = texture(u_Textures[index], v_TexCoord);
    
    float noise = fbm(v_TexCoord);
    FragColor = texColor * noise;
}