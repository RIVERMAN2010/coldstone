#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "ModelLoader.h"

struct Material {
    glm::vec3 albedo;
    float metallic;
    float roughness;
    glm::vec3 emission;
    float transparency;

    // Presets
    static Material Gold() { return { {1.0f, 0.76f, 0.33f}, 1.0f, 0.05f, {0,0,0}, 0.0f }; }
    static Material Silver() { return { {0.95f, 0.95f, 0.95f}, 1.0f, 0.1f, {0,0,0}, 0.0f }; }
    static Material RoughIron() { return { {0.5f, 0.5f, 0.5f}, 0.8f, 0.7f, {0,0,0}, 0.0f }; }

    static Material PlasticRed() { return { {1.0f, 0.1f, 0.1f}, 0.0f, 0.5f, {0,0,0}, 0.0f }; }
    static Material RubberBlue() { return { {0.1f, 0.1f, 1.0f}, 0.0f, 0.95f, {0,0,0}, 0.0f }; }

    // Emissive
    static Material EmissiveGreen() { return { {0,0,0}, 0.0f, 1.0f, {0.0f, 5.0f, 0.0f}, 0.0f }; } // Strong Green Light
    static Material EmissiveWhite() { return { {0,0,0}, 0.0f, 1.0f, {10.0f, 10.0f, 10.0f}, 0.0f }; } // Blinding White

    // Transparent
    static Material GlassClear() { return { {0.9f, 0.95f, 1.0f}, 0.0f, 0.0f, {0,0,0}, 0.8f }; }
    static Material GlassFrosted() { return { {1.0f, 0.8f, 0.8f}, 0.0f, 0.4f, {0,0,0}, 0.6f }; }
};

struct GameObject {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    Material material;
    Mesh* mesh;
};

class Scene {
public:
    static std::vector<GameObject> objects;
    static Mesh cubeMesh;
    static Mesh floorMesh;
    static Mesh quadMesh;

    static void Initialize() {
        cubeMesh = ModelLoader::CreateCube();
        quadMesh = ModelLoader::CreateQuad();
        floorMesh = ModelLoader::CreateCube();

        objects.clear();

        // 1. Floor (Dark Concrete)
        objects.push_back({ {0, -1.0f, 0}, {0,0,0}, {50.0f, 0.2f, 50.0f}, { {0.2f, 0.2f, 0.2f}, 0.0f, 0.8f, {0,0,0}, 0.0f }, &floorMesh });

        // 2. Metals (Left)
        objects.push_back({ {-6, 1, 0}, {0, 30, 0}, {1.5f, 1.5f, 1.5f}, Material::Gold(), &cubeMesh });
        objects.push_back({ {-3, 1, 0}, {0, 0, 0}, {1.5f, 1.5f, 1.5f}, Material::Silver(), &cubeMesh });
        objects.push_back({ { 0, 1, 0}, {0, 0, 0}, {1.5f, 1.5f, 1.5f}, Material::RoughIron(), &cubeMesh });

        // 3. Emissive (Right)
        objects.push_back({ { 3, 1, 2}, {0,0,0}, {1,1,1}, Material::EmissiveGreen(), &cubeMesh });
        objects.push_back({ { 6, 1, -2}, {0,0,0}, {1,1,1}, Material::EmissiveWhite(), &cubeMesh });

        // 4. Glass (Front)
        objects.push_back({ {-2, 1, 4}, {45, 45, 0}, {1.5f, 1.5f, 1.5f}, Material::GlassClear(), &cubeMesh });
        objects.push_back({ { 2, 1, 4}, {0, 20, 0}, {1.5f, 1.5f, 1.5f}, Material::GlassFrosted(), &cubeMesh });
    }
};
std::vector<GameObject> Scene::objects;
Mesh Scene::cubeMesh;
Mesh Scene::floorMesh;
Mesh Scene::quadMesh;