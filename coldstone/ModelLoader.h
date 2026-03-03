#pragma once
#include "Mesh.h"
#include <string>

class ModelLoader {
public:
    static Mesh LoadMesh(const std::string& path);
    static Mesh CreateCube();
    static Mesh CreateQuad();
    static void UpdateAnimation(Mesh* mesh, float timeInSeconds, std::vector<glm::mat4>& transforms);
};