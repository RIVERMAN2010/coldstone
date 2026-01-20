#pragma once

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
layout (location = 2) out vec4 gMaterial; 

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
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

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

struct PointLight {
    vec3 position;
    vec3 color;
    float radius;
};
#define MAX_LIGHTS 16
uniform PointLight pointLights[MAX_LIGHTS];
uniform int numPointLights;

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

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    float Depth = texture(gDepth, TexCoords).r;
    if (Depth == 1.0) {
        FragColor = vec4(0.05, 0.05, 0.08, 1.0);
        BrightColor = vec4(0,0,0,1);
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
        vec3 glow = Albedo * max(1.0, Emission);
        FragColor = vec4(glow, 1.0);
        float brightness = dot(glow, vec3(0.2126, 0.7152, 0.0722));
        if(brightness > 1.0) BrightColor = vec4(glow, 1.0);
        else BrightColor = vec4(0,0,0,1);
        return;
    }

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, Albedo, Metallic);
    vec3 Lo = vec3(0.0);

    {
        vec3 L = normalize(sunDir);
        vec3 H = normalize(L + ViewDir);
        float NdotL = max(dot(Normal, L), 0.0);
        
        if (NdotL > 0.0) {
            vec4 fragPosLightSpace = lightSpaceMatrix * vec4(WorldPos, 1.0);
            float shadow = ShadowCalculation(fragPosLightSpace, Normal, L);
            vec3 lightColor = vec3(1.0, 0.95, 0.8) * 1.5;
            
            vec3 F = fresnelSchlick(max(dot(H, ViewDir), 0.0), F0);
            float NdotH = max(dot(Normal, H), 0.0);
            float spec = pow(NdotH, 64.0 * (1.0 - Roughness));
            vec3 specular = spec * lightColor * F; 
            vec3 kD = (vec3(1.0) - F) * (1.0 - Metallic);
            vec3 diffuse = kD * Albedo * lightColor * NdotL;

            Lo += (1.0 - shadow) * (diffuse + specular);
        }
    }

    for(int i = 0; i < numPointLights; ++i) {
        vec3 L = normalize(pointLights[i].position - WorldPos);
        float NdotL = max(dot(Normal, L), 0.0);

        if (NdotL > 0.0) {
            vec3 H = normalize(L + ViewDir);
            float distance = length(pointLights[i].position - WorldPos);
            float attenuation = 1.0 / (1.0 + 0.1 * distance + 0.05 * distance * distance);
            vec3 radiance = pointLights[i].color * attenuation;

            vec3 F = fresnelSchlick(max(dot(H, ViewDir), 0.0), F0);
            float NdotH = max(dot(Normal, H), 0.0);
            float spec = pow(NdotH, 32.0 * (1.0 - Roughness));
            vec3 specular = spec * radiance * F;

            vec3 kD = (vec3(1.0) - F) * (1.0 - Metallic);
            vec3 diffuse = kD * Albedo * radiance * NdotL;

            Lo += (diffuse + specular);
        }
    }

    vec3 R = reflect(-ViewDir, Normal);
    vec3 skyColor = vec3(0.2, 0.2, 0.3);
    vec3 groundColor = vec3(0.05, 0.05, 0.05);
    float mixFactor = 0.5 * (R.y + 1.0);
    vec3 envColor = mix(groundColor, skyColor, mixFactor);
    vec3 ambientReflection = envColor * F0 * (1.0 - Roughness);
    vec3 ambientDiffuse = vec3(0.03) * Albedo * (1.0 - Metallic);
    vec3 ambient = ambientDiffuse + ambientReflection;

    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
    
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.8) BrightColor = vec4(color, 1.0);
    else BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
)";

inline const char* BLUR_VS = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTex;
out vec2 TexCoords;
void main() {
    TexCoords = aTex;
    gl_Position = vec4(aPos, 1.0);
}
)";

inline const char* BLUR_FS = R"(
#version 460 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D image;
uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {             
    vec2 tex_offset = 1.0 / textureSize(image, 0); 
    vec3 result = texture(image, TexCoords).rgb * weight[0];
    if(horizontal) {
        for(int i = 1; i < 5; ++i) {
            result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    } else {
        for(int i = 1; i < 5; ++i) {
            result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}
)";

inline const char* COMPOSITE_VS = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTex;
out vec2 TexCoords;
void main() {
    TexCoords = aTex;
    gl_Position = vec4(aPos, 1.0);
}
)";

inline const char* COMPOSITE_FS = R"(
#version 460 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D scene;
uniform sampler2D bloomBlur;

void main() {             
    vec3 hdrColor = texture(scene, TexCoords).rgb;      
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    hdrColor += bloomColor; 
    FragColor = vec4(hdrColor, 1.0);
}
)";

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
uniform float roughness;
uniform vec3 viewPos;
uniform vec3 sunDir;

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 L = normalize(sunDir);
    vec3 H = normalize(L + viewDir);

    vec3 R = reflect(-viewDir, norm);
    vec3 skyColor = vec3(0.5, 0.6, 0.8);
    vec3 groundColor = vec3(0.1, 0.1, 0.1);
    float mixFactor = 0.5 * (R.y + 1.0);
    vec3 envColor = mix(groundColor, skyColor, mixFactor);
    
    float fresnel = pow(1.0 - max(dot(norm, viewDir), 0.0), 3.0);
    vec3 reflection = envColor * fresnel * (1.0 - roughness);

    float diff = max(dot(norm, L), 0.0);
    vec3 diffuse = diff * color * 0.5;
    
    float spec = pow(max(dot(norm, H), 0.0), 64.0);
    vec3 specular = vec3(1.0) * spec;

    vec3 result = diffuse + specular + reflection;
    float finalAlpha = alpha + fresnel * 0.5;
    
    FragColor = vec4(result, clamp(finalAlpha, 0.0, 1.0));
}
)";