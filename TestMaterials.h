#pragma once
#include "Materials.h"
#include "AssetLoader.h"
#include <vector>
#include <glm/glm.hpp>

struct TestObject {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    Material material;
};

class TestMaterials {
public:
    static void Initialize();
    static void AddCube(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, Material mat);
    static void Draw(unsigned int shaderProgram);
    static void DrawCubeMesh();
    static void Cleanup();
private:
    static std::vector<TestObject> objects;
    static Mesh cubeMesh;
};