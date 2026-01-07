#pragma once

const char* vsSrc = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;
out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    TexCoords = aTex;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* fsSrc = R"(
#version 460 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    vec3 emission;
    float alpha;
};
uniform Material material;
uniform sampler2D tex1; 
uniform sampler2D skybox; 
uniform vec3 viewPos;
uniform int useTexture; 

const float PI = 3.14159265359;

vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, 0.3183); 
    uv += 0.5;
    return uv;
}

void main() {
    vec3 norm = normalize(Normal);
    if (!gl_FrontFacing) {
        norm = -norm;
    }
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 R = reflect(-viewDir, norm); 

    vec3 baseColor = material.albedo;
    if (useTexture == 1) {
        baseColor *= texture(tex1, TexCoords).rgb;
    }

    vec2 envUV = SampleSphericalMap(normalize(R));
    vec3 envColor = texture(skybox, envUV).rgb;
    
    // Simple Fresnel
    float NdotV = max(dot(norm, viewDir), 0.0);
    vec3 F0 = mix(vec3(0.04), baseColor, material.metallic);
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - NdotV, 5.0);

    vec3 kD = (1.0 - F) * (1.0 - material.metallic);
    
    vec3 diffuse = kD * baseColor; 
    vec3 specular = envColor * F * (1.0 - material.roughness); 

    vec3 finalColor = diffuse * 0.5 + specular + material.emission; 
    FragColor = vec4(finalColor, material.alpha);
}
)";

const char* vsSky = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
out vec3 WorldPos;
uniform mat4 view;
uniform mat4 projection;
void main() {
    WorldPos = aPos;
    mat4 rotView = mat4(mat3(view)); // Remove translation
    vec4 clipPos = projection * rotView * vec4(WorldPos, 1.0);
    gl_Position = clipPos.xyww; // Force z to 1.0
}
)";

const char* fsSky = R"(
#version 460 core
out vec4 FragColor;
in vec3 WorldPos;
uniform sampler2D skybox;
vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, 0.3183);
    uv += 0.5;
    return uv;
}
void main() {
    vec2 uv = SampleSphericalMap(normalize(WorldPos));
    FragColor = texture(skybox, uv);
}
)";