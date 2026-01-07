#include "AssetLoader.h"
#include "Utils.h"
#include "MainDevIL.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>
#include <filesystem>
#include <format>

#pragma comment(lib, "assimp-vc143-mt.lib")

std::vector<std::string> AssetLoader::assetDirs;

void Mesh::Setup() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
}

void Mesh::Draw() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
}

void AssetLoader::AddAssetDir(const std::string& path) {
    assetDirs.push_back(path);
}

std::string AssetLoader::FindFile(const std::string& filename) {
    if (std::filesystem::exists(filename)) return filename;
    for (const auto& dir : assetDirs) {
        std::string full = dir + "/" + filename;
        if (std::filesystem::exists(full)) return full;
    }
    return "";
}

Mesh AssetLoader::LoadMesh(const std::string& filename) {
    Mesh mesh;
    std::string path = FindFile(filename);
    if (path.empty()) {
        Log(std::format("Asset not found: {}", filename));
        return mesh;
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        Log(std::format("Assimp Error: {}", importer.GetErrorString()));
        return mesh;
    }

    if (scene->mNumMeshes > 0) {
        for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
            aiMesh* aMesh = scene->mMeshes[m];
            unsigned int vertexOffset = (unsigned int)mesh.vertices.size();

            for (unsigned int i = 0; i < aMesh->mNumVertices; i++) {
                Vertex vertex;
                vertex.Position = { aMesh->mVertices[i].x, aMesh->mVertices[i].y, aMesh->mVertices[i].z };
                if (aMesh->HasNormals())
                    vertex.Normal = { aMesh->mNormals[i].x, aMesh->mNormals[i].y, aMesh->mNormals[i].z };
                if (aMesh->mTextureCoords[0])
                    vertex.TexCoords = { aMesh->mTextureCoords[0][i].x, aMesh->mTextureCoords[0][i].y };
                else
                    vertex.TexCoords = { 0.0f, 0.0f };
                mesh.vertices.push_back(vertex);
            }
            for (unsigned int i = 0; i < aMesh->mNumFaces; i++) {
                aiFace face = aMesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    mesh.indices.push_back(face.mIndices[j] + vertexOffset);
            }
        }

        if (scene->mMeshes[0]->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[scene->mMeshes[0]->mMaterialIndex];
            if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                aiString str;
                material->GetTexture(aiTextureType_DIFFUSE, 0, &str);

                std::string texPath = str.C_Str();
                Log(std::format("Processing Texture Path: {}", texPath));

                if (texPath.length() > 0 && texPath[0] == '*') {
                    unsigned int texIndex = (unsigned int)atoi(&texPath[1]);
                    if (texIndex < scene->mNumTextures) {
                        aiTexture* embeddedTex = scene->mTextures[texIndex];
                        if (embeddedTex->mHeight == 0) {
                            Log(std::format("Found embedded compressed texture at index {}", texIndex));
                            mesh.textureID = MainDevIL::LoadTextureFromMemory(embeddedTex->pcData, embeddedTex->mWidth);
                        }
                        else {
                            Log("Embedded raw texture data. skip");
                        }
                    }
                }
                else {
                    std::filesystem::path p(texPath);
                    std::string filenameOnly = p.filename().string();
                    std::string fullPath = FindFile(filenameOnly);

                    if (!fullPath.empty()) {
                        mesh.textureID = MainDevIL::LoadTexture(fullPath);
                    }
                    else {
                        Log(std::format("Texture not found on disk: {}", filenameOnly));
                    }
                }
            }
        }
    }

    mesh.Setup();
    Log(std::format("Mesh loaded: {}", filename));
    return mesh;
}
