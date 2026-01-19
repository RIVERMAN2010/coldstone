#pragma once

// --- SHADOW PASS ---
inline const char* SHADOW_VS = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
uniform mat4 lightSpaceMatrix;
uniform mat4 model;
void main() {
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}
)";

inline const char* SHADOW_FS = R"(
#version 460 core
void main() {}
)";

// --- GBUFFER PASS (Opaque) ---
inline const char* GBUFFER_VS = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
out vec3 FragPos;
out vec3 Normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * worldPos;
}
)";

inline const char* GBUFFER_FS = R"(
#version 460 core
layout (location = 0) out vec4 gAlbedo;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gMaterial; // R=Metallic, G=Roughness, B=Emission

in vec3 FragPos;
in vec3 Normal;

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    vec3 emission;
};
uniform Material material;

void main() {
    gAlbedo = vec4(material.albedo, 1.0);
    gNormal = vec4(normalize(Normal), 1.0);
    float emissionStrength = max(material.emission.r, max(material.emission.g, material.emission.b));
    gMaterial = vec4(material.metallic, material.roughness, emissionStrength, 1.0);
    
    if (emissionStrength > 0.0) {
        gAlbedo = vec4(material.emission, 1.0);
    }
}
)";

// --- LIGHTING PASS (Deferred) ---
inline const char* LIGHTING_VS = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTex;
out vec2 TexCoords;
void main() {
    TexCoords = aTex;
    gl_Position = vec4(aPos, 1.0);
}
)";

inline const char* LIGHTING_FS = R"(
#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gMaterial;
uniform sampler2D gDepth;
uniform sampler2D shadowMap;

uniform vec3 viewPos;
uniform mat4 inverseView;
uniform mat4 inverseProj;
uniform mat4 lightSpaceMatrix;
uniform vec3 sunDir;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0) return 0.0;
    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    return shadow / 9.0;
}

vec3 ReconstructPosition(float depth, vec2 uv) {
    float z = depth * 2.0 - 1.0;
    vec4 clipSpacePosition = vec4(uv * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = inverseProj * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = inverseView * viewSpacePosition;
    return worldSpacePosition.xyz;
}

void main() {
    float Depth = texture(gDepth, TexCoords).r;
    if (Depth == 1.0) {
        FragColor = vec4(0.1, 0.1, 0.15, 1.0); // Background Color
        return;
    }

    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec4 MatData = texture(gMaterial, TexCoords);
    float Metallic = MatData.r;
    float Roughness = MatData.g;
    float Emission = MatData.b;

    vec3 WorldPos = ReconstructPosition(Depth, TexCoords);
    vec3 ViewDir = normalize(viewPos - WorldPos);

    if (Emission > 0.0) {
        FragColor = vec4(Albedo * max(1.0, Emission), 1.0);
        return;
    }

    // Hemisphere Ambient (Sky vs Ground)
    vec3 skyColor = vec3(0.3, 0.3, 0.4);
    vec3 groundColor = vec3(0.1, 0.1, 0.1);
    vec3 up = vec3(0,1,0);
    float hemiMix = dot(Normal, up) * 0.5 + 0.5;
    vec3 ambient = mix(groundColor, skyColor, hemiMix) * Albedo;

    // Sun Light
    vec3 L = normalize(sunDir);
    vec3 H = normalize(L + ViewDir);
    float NdotL = max(dot(Normal, L), 0.0);
    
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(WorldPos, 1.0);
    float shadow = ShadowCalculation(fragPosLightSpace, Normal, L);

    vec3 lighting = ambient;

    if (NdotL > 0.0) {
        vec3 lightColor = vec3(1.0, 0.95, 0.8);
        vec3 diffuse = NdotL * lightColor * Albedo;
        
        float NdotH = max(dot(Normal, H), 0.0);
        float spec = pow(NdotH, 64.0 * (1.0 - Roughness));
        vec3 specular = spec * lightColor * mix(vec3(0.04), Albedo, Metallic);

        lighting += (1.0 - shadow) * (diffuse + specular);
    }

    // Tone Mapping
    lighting = lighting / (lighting + vec3(1.0));
    lighting = pow(lighting, vec3(1.0/2.2));

    FragColor = vec4(lighting, 1.0);
}
)";

// --- FORWARD PASS (Transparent) ---
inline const char* FORWARD_VS = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
out vec3 FragPos;
out vec3 Normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

inline const char* FORWARD_FS = R"(
#version 460 core
out vec4 FragColor;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 color;
uniform float alpha;
uniform vec3 viewPos;
uniform vec3 sunDir;

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 L = normalize(sunDir);
    vec3 H = normalize(L + viewDir);

    // Simple Blinn-Phong
    vec3 ambient = 0.3 * color;
    float diff = max(dot(norm, L), 0.0);
    vec3 diffuse = diff * color;
    
    float spec = pow(max(dot(norm, H), 0.0), 32.0);
    vec3 specular = vec3(0.5) * spec;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, alpha);
}
)";