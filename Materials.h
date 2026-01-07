#pragma once
#include <glm/glm.hpp>
#include <string>

struct Material {
    std::string name;
    glm::vec3 albedo;
    float metallic;
    float roughness;
    glm::vec3 emission;
    float alpha;

    static Material Standard(glm::vec3 color, float met = 0.0f, float rough = 0.5f) {
        return { "Std", color, met, rough, {0,0,0}, 1.0f };
    }
    static Material Emissive(glm::vec3 color, float strength) {
        return { "Glow", color, 0.0f, 1.0f, color * strength, 1.0f };
    }
    static Material Glass(glm::vec3 color) {
        return { "Glass", color, 0.0f, 0.1f, {0,0,0}, 0.3f };
    }
    static Material Uranium(glm::vec3 color) {
        return { "Uranium", color, 0.0f, 0.2f, color * 2.0f, 0.5f };
    }
};