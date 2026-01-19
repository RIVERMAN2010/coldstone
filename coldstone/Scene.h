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

    static Material Chrome() { return { {0.9f, 0.9f, 0.9f}, 1.0f, 0.05f, {0,0,0} }; }
    static Material Gold() { return { {1.0f, 0.76f, 0.33f}, 1.0f, 0.1f, {0,0,0} }; }
    static Material PlasticRed() { return { {1.0f, 0.1f, 0.1f}, 0.0f, 0.5f, {0,0,0} }; }
    static Material RubberBlue() { return { {0.1f, 0.1f, 1.0f}, 0.0f, 0.9f, {0,0,0} }; }
    static Material EmissiveGreen() { return { {0,0,0}, 0.0f, 1.0f, {0.2f, 1.0f, 0.2f} }; } // Glows
    static Material EmissiveStrong() { return { {0,0,0}, 0.0f, 1.0f, {5.0f, 5.0f, 5.0f} }; } // White Light
    static Material GlassFake() { return { {0.9f, 0.9f, 1.0f}, 0.0f, 0.01f, {0,0,0} }; } // glass surface
    static Material RoughMetal() { return { {0.6f, 0.6f, 0.6f}, 1.0f, 0.8f, {0,0,0} }; }
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

        // 1. Huge Floor to catch shadows
        objects.push_back({ {0, -1.0f, 0}, {0,0,0}, {50.0f, 0.2f, 50.0f}, Material::PlasticRed(), &floorMesh });
        // Override floor material to grey concrete
        objects.back().material = { {0.5f, 0.5f, 0.5f}, 0.0f, 0.9f, {0,0,0} };

        // 2. Test Cubes Row 1
        // Gold
        objects.push_back({ {-4, 1, 0}, {0, 30, 0}, {1.5f, 1.5f, 1.5f}, Material::Gold(), &cubeMesh });

        // Chrome / Mirror
        objects.push_back({ {-1, 1, 0}, {0, 0, 0}, {1.5f, 1.5f, 1.5f}, Material::Chrome(), &cubeMesh });

        // Fake Glass (High Gloss)
        objects.push_back({ { 2, 1, 0}, {45, 45, 0}, {1.5f, 1.5f, 1.5f}, Material::GlassFake(), &cubeMesh });

        // Rough Metal
        objects.push_back({ { 5, 1, 0}, {0, 0, 0}, {1.5f, 1.5f, 1.5f}, Material::RoughMetal(), &cubeMesh });

        // 3. Test Cubes Row 2 (Special)
        // Emissive Green (Radioactive)
        objects.push_back({ {-4, 4, -2}, {0,0,0}, {1,1,1}, Material::EmissiveGreen(), &cubeMesh });

        // Super Bright Light Source (Visual only, point light logic is in shader)
        objects.push_back({ { 0, 5, 2}, {0,0,0}, {0.5f, 0.5f, 0.5f}, Material::EmissiveStrong(), &cubeMesh });

        // Blue Rubber
        objects.push_back({ { 4, 4, -2}, {10, 20, 30}, {1,1,1}, Material::RubberBlue(), &cubeMesh });
    }
};
std::vector<GameObject> Scene::objects;
Mesh Scene::cubeMesh;
Mesh Scene::floorMesh;
Mesh Scene::quadMesh;