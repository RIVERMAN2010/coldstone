#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Materials.h"
#include "AssetLoader.h"
#include "TestMaterials.h"

class Scene {
public:
    static void Initialize();
    static void Draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& camPos);
    static void Cleanup();

private:
    static void SetupShaders();
    static void ResetMaterialUniforms(unsigned int shader);

    static unsigned int shaderProgram;
    static unsigned int skyProgram;

    static Mesh floorMesh;
    static Mesh characterMesh;
    static Mesh cubeMesh;

    static unsigned int hdriTex;
    static unsigned int whiteTex;

    static std::vector<TestObject> testObjects;
};