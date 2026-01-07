#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;
    unsigned int textureID = 0;
    void Setup();
    void Draw();
};

class AssetLoader {
public:
    static void AddAssetDir(const std::string& path);
    static Mesh LoadMesh(const std::string& filename);
    static std::string FindFile(const std::string& filename);
private:
    static std::vector<std::string> assetDirs;
};