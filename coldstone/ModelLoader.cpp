#include "ModelLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <format>
#include <IL/il.h>
#include <IL/ilu.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#pragma comment(lib, "assimp-vc143-mt.lib")

Mesh ModelLoader::LoadMesh(const std::string& path) {
    Mesh mesh;
    mesh.importer = std::make_shared<Assimp::Importer>();
    mesh.scene = mesh.importer->ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
    const aiScene* scene = mesh.scene;

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << std::format("Assimp Error: {}\n", mesh.importer->GetErrorString());
        return CreateCube();
    }

    auto& m = scene->mRootNode->mTransformation;
    mesh.globalInverseTransform = glm::inverse(glm::mat4(m.a1, m.b1, m.c1, m.d1, m.a2, m.b2, m.c2, m.d2, m.a3, m.b3, m.c3, m.d3, m.a4, m.b4, m.c4, m.d4));

    for (unsigned int mIdx = 0; mIdx < scene->mNumMeshes; mIdx++) {
        aiMesh* aMesh = scene->mMeshes[mIdx];
        int baseVertex = (int)mesh.vertices.size();

        glm::vec4 matColor(1.0f);
        if (scene->HasMaterials()) {
            aiMaterial* material = scene->mMaterials[aMesh->mMaterialIndex];
            aiColor4D color;
            if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color)) {
                matColor = { color.r, color.g, color.b, color.a };
            }
        }

        for (unsigned int i = 0; i < aMesh->mNumVertices; i++) {
            Vertex vertex;
            vertex.Position = { aMesh->mVertices[i].x, aMesh->mVertices[i].y, aMesh->mVertices[i].z };
            if (aMesh->HasNormals())
                vertex.Normal = { aMesh->mNormals[i].x, aMesh->mNormals[i].y, aMesh->mNormals[i].z };
            if (aMesh->mTextureCoords[0])
                vertex.TexCoords = { aMesh->mTextureCoords[0][i].x, aMesh->mTextureCoords[0][i].y };
            else
                vertex.TexCoords = { 0.0f, 0.0f };

            if (aMesh->HasVertexColors(0)) {
                vertex.Color = { aMesh->mColors[0][i].r, aMesh->mColors[0][i].g, aMesh->mColors[0][i].b, aMesh->mColors[0][i].a };
            }
            else {
                vertex.Color = matColor;
            }

            for (int k = 0; k < 4; k++) { vertex.BoneIDs[k] = -1; vertex.Weights[k] = 0.0f; }
            mesh.vertices.push_back(vertex);
        }
        for (unsigned int i = 0; i < aMesh->mNumFaces; i++) {
            aiFace face = aMesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                mesh.indices.push_back(baseVertex + face.mIndices[j]);
        }

        for (unsigned int i = 0; i < aMesh->mNumBones; i++) {
            aiBone* bone = aMesh->mBones[i];
            std::string boneName = bone->mName.data;
            int boneID = -1;
            if (mesh.boneInfoMap.find(boneName) == mesh.boneInfoMap.end()) {
                boneID = mesh.boneCounter++;
                BoneInfo bi;
                bi.id = boneID;
                auto& bm = bone->mOffsetMatrix;
                bi.offset = glm::mat4(bm.a1, bm.b1, bm.c1, bm.d1, bm.a2, bm.b2, bm.c2, bm.d2, bm.a3, bm.b3, bm.c3, bm.d3, bm.a4, bm.b4, bm.c4, bm.d4);
                mesh.boneInfoMap[boneName] = bi;
            }
            else {
                boneID = mesh.boneInfoMap[boneName].id;
            }

            for (unsigned int j = 0; j < bone->mNumWeights; j++) {
                int vertexID = baseVertex + bone->mWeights[j].mVertexId;
                float weight = bone->mWeights[j].mWeight;
                for (int k = 0; k < 4; ++k) {
                    if (mesh.vertices[vertexID].BoneIDs[k] < 0) {
                        mesh.vertices[vertexID].BoneIDs[k] = boneID;
                        mesh.vertices[vertexID].Weights[k] = weight;
                        break;
                    }
                }
            }
        }
        if (scene->HasTextures()) {
            aiTexture* aiTex = scene->mTextures[0];
            ILuint imgName;
            ilGenImages(1, &imgName);
            ilBindImage(imgName);
            if (aiTex->mHeight == 0) {
                if (ilLoadL(IL_TYPE_UNKNOWN, (const void*)aiTex->pcData, aiTex->mWidth)) {
                    glGenTextures(1, &mesh.textureID);
                    glBindTexture(GL_TEXTURE_2D, mesh.textureID);
                    glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP) == 4 ? GL_RGBA : GL_RGB,
                        ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,
                        ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_TYPE), ilGetData());
                    glGenerateMipmap(GL_TEXTURE_2D);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
                else {
                    std::cerr << std::format("Failed to load embedded texture\n");
                }
            }
            ilDeleteImages(1, &imgName);
        }
    }
    mesh.Setup();
    return mesh;
}

Mesh ModelLoader::CreateCube() {
    Mesh mesh;
    float v[] = {
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f, 0.0f, 0.0f,
        // Left face
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        // Right face
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
         // Bottom face
         -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
          0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
          0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
         // Top face
         -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f
    };
    for (int i = 0; i < 36; i++) {
        Vertex vtx;
        int o = i * 8;
        vtx.Position = { v[o], v[o + 1], v[o + 2] };
        vtx.Normal = { v[o + 3], v[o + 4], v[o + 5] };
        vtx.TexCoords = { v[o + 6], v[o + 7] };
        vtx.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
        for (int k = 0; k < 4; k++) { vtx.BoneIDs[k] = -1; vtx.Weights[k] = 0.0f; }
        mesh.vertices.push_back(vtx);
        mesh.indices.push_back(i);
    }
    mesh.Setup();
    return mesh;
}

Mesh ModelLoader::CreateQuad() {
    Mesh mesh;
    mesh.vertices = {
        {{-1, 1, 0}, {0,0,1}, {0,1}, {1,1,1,1}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{-1,-1, 0}, {0,0,1}, {0,0}, {1,1,1,1}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{ 1,-1, 0}, {0,0,1}, {1,0}, {1,1,1,1}, {-1,-1,-1,-1}, {0,0,0,0}},
        {{ 1, 1, 0}, {0,0,1}, {1,1}, {1,1,1,1}, {-1,-1,-1,-1}, {0,0,0,0}}
    };
    mesh.indices = { 0,1,2, 0,2,3 };
    mesh.Setup();
    return mesh;
}

static glm::mat4 InterpolateTranslation(float time, const aiNodeAnim* nodeAnim) {
    if (nodeAnim->mNumPositionKeys == 1) return glm::translate(glm::mat4(1.0f), glm::vec3(nodeAnim->mPositionKeys[0].mValue.x, nodeAnim->mPositionKeys[0].mValue.y, nodeAnim->mPositionKeys[0].mValue.z));
    unsigned int p0 = 0;
    for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys - 1; i++) { if (time < (float)nodeAnim->mPositionKeys[i + 1].mTime) { p0 = i; break; } }
    unsigned int p1 = p0 + 1;
    float factor = (time - (float)nodeAnim->mPositionKeys[p0].mTime) / ((float)nodeAnim->mPositionKeys[p1].mTime - (float)nodeAnim->mPositionKeys[p0].mTime);
    aiVector3D start = nodeAnim->mPositionKeys[p0].mValue;
    aiVector3D end = nodeAnim->mPositionKeys[p1].mValue;
    aiVector3D delta = end - start;
    return glm::translate(glm::mat4(1.0f), glm::vec3(start.x + factor * delta.x, start.y + factor * delta.y, start.z + factor * delta.z));
}

static glm::mat4 InterpolateRotation(float time, const aiNodeAnim* nodeAnim) {
    if (nodeAnim->mNumRotationKeys == 1) {
        aiQuaternion q = nodeAnim->mRotationKeys[0].mValue;
        return glm::mat4_cast(glm::quat(q.w, q.x, q.y, q.z));
    }
    unsigned int p0 = 0;
    for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys - 1; i++) { if (time < (float)nodeAnim->mRotationKeys[i + 1].mTime) { p0 = i; break; } }
    unsigned int p1 = p0 + 1;
    float factor = (time - (float)nodeAnim->mRotationKeys[p0].mTime) / ((float)nodeAnim->mRotationKeys[p1].mTime - (float)nodeAnim->mRotationKeys[p0].mTime);
    aiQuaternion start = nodeAnim->mRotationKeys[p0].mValue;
    aiQuaternion end = nodeAnim->mRotationKeys[p1].mValue;
    aiQuaternion out;
    aiQuaternion::Interpolate(out, start, end, factor);
    out = out.Normalize();
    return glm::mat4_cast(glm::quat(out.w, out.x, out.y, out.z));
}

static glm::mat4 InterpolateScaling(float time, const aiNodeAnim* nodeAnim) {
    if (nodeAnim->mNumScalingKeys == 1) return glm::scale(glm::mat4(1.0f), glm::vec3(nodeAnim->mScalingKeys[0].mValue.x, nodeAnim->mScalingKeys[0].mValue.y, nodeAnim->mScalingKeys[0].mValue.z));
    unsigned int p0 = 0;
    for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys - 1; i++) { if (time < (float)nodeAnim->mScalingKeys[i + 1].mTime) { p0 = i; break; } }
    unsigned int p1 = p0 + 1;
    float factor = (time - (float)nodeAnim->mScalingKeys[p0].mTime) / ((float)nodeAnim->mScalingKeys[p1].mTime - (float)nodeAnim->mScalingKeys[p0].mTime);
    aiVector3D start = nodeAnim->mScalingKeys[p0].mValue;
    aiVector3D end = nodeAnim->mScalingKeys[p1].mValue;
    aiVector3D delta = end - start;
    return glm::scale(glm::mat4(1.0f), glm::vec3(start.x + factor * delta.x, start.y + factor * delta.y, start.z + factor * delta.z));
}

static void ReadNodeHeirarchy(const aiNode* node, const glm::mat4& parentTransform, float animationTime, const aiAnimation* pAnimation, Mesh& mesh, std::vector<glm::mat4>& transforms) {
    std::string nodeName = node->mName.data;
    auto& m = node->mTransformation;
    glm::mat4 nodeTransform = glm::mat4(m.a1, m.b1, m.c1, m.d1, m.a2, m.b2, m.c2, m.d2, m.a3, m.b3, m.c3, m.d3, m.a4, m.b4, m.c4, m.d4);

    const aiNodeAnim* nodeAnim = nullptr;
    for (unsigned int i = 0; i < pAnimation->mNumChannels; i++) {
        if (std::string(pAnimation->mChannels[i]->mNodeName.data) == nodeName) {
            nodeAnim = pAnimation->mChannels[i];
            break;
        }
    }

    if (nodeAnim) {
        glm::mat4 trans = InterpolateTranslation(animationTime, nodeAnim);
        glm::mat4 rot = InterpolateRotation(animationTime, nodeAnim);
        glm::mat4 scale = InterpolateScaling(animationTime, nodeAnim);
        nodeTransform = trans * rot * scale;
    }

    glm::mat4 globalTransform = parentTransform * nodeTransform;

    if (mesh.boneInfoMap.find(nodeName) != mesh.boneInfoMap.end()) {
        int boneIndex = mesh.boneInfoMap[nodeName].id;
        transforms[boneIndex] = mesh.globalInverseTransform * globalTransform * mesh.boneInfoMap[nodeName].offset;
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ReadNodeHeirarchy(node->mChildren[i], globalTransform, animationTime, pAnimation, mesh, transforms);
    }
}

void ModelLoader::UpdateAnimation(Mesh* mesh, float timeInSeconds, std::vector<glm::mat4>& transforms) {
    if (!mesh || !mesh->scene || mesh->scene->mNumAnimations == 0) return;
    const aiAnimation* animation = mesh->scene->mAnimations[0];
    float ticksPerSecond = animation->mTicksPerSecond != 0 ? (float)animation->mTicksPerSecond : 30.0f;
    float timeInTicks = timeInSeconds * ticksPerSecond;
    float animationTime = fmod(timeInTicks, (float)animation->mDuration);

    transforms.resize(mesh->boneCounter, glm::mat4(1.0f));
    ReadNodeHeirarchy(mesh->scene->mRootNode, glm::mat4(1.0f), animationTime, animation, *mesh, transforms);
}