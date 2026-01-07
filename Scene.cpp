#include "Scene.h"
#include "Shaders.h"
#include "MainDevIL.h"
#include "Utils.h"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

unsigned int Scene::shaderProgram = 0;
unsigned int Scene::doubleSidedProgram = 0;
unsigned int Scene::skyProgram = 0;
Mesh Scene::floorMesh;
Mesh Scene::characterMesh;
Mesh Scene::cubeMesh;
unsigned int Scene::hdriTex = 0;
unsigned int Scene::whiteTex = 0;
std::vector<TestObject> Scene::testObjects;

void Scene::Initialize() {
    SetupShaders();

    std::string hdriPath = AssetLoader::FindFile("TestHDRI.jpg");
    hdriTex = MainDevIL::LoadTexture(hdriPath);
    whiteTex = MainDevIL::CreateWhiteTexture();
    if (hdriTex == 0) hdriTex = whiteTex;

    float v[] = { -0.5f,-0.5f,-0.5f,0,0,-1,0,0,  0.5f,-0.5f,-0.5f,0,0,-1,1,0,  0.5f,0.5f,-0.5f,0,0,-1,1,1,
                  0.5f,0.5f,-0.5f,0,0,-1,1,1,   -0.5f,0.5f,-0.5f,0,0,-1,0,1,   -0.5f,-0.5f,-0.5f,0,0,-1,0,0,
                  -0.5f,-0.5f,0.5f,0,0,1,0,0,   0.5f,-0.5f,0.5f,0,0,1,1,0,    0.5f,0.5f,0.5f,0,0,1,1,1,
                  0.5f,0.5f,0.5f,0,0,1,1,1,    -0.5f,0.5f,0.5f,0,0,1,0,1,    -0.5f,-0.5f,0.5f,0,0,1,0,0,
                  -0.5f,0.5f,0.5f,-1,0,0,1,0,  -0.5f,0.5f,-0.5f,-1,0,0,1,1,  -0.5f,-0.5f,-0.5f,-1,0,0,0,1,
                  -0.5f,-0.5f,-0.5f,-1,0,0,0,1, -0.5f,-0.5f,0.5f,-1,0,0,0,0,  -0.5f,0.5f,0.5f,-1,0,0,1,0,
                  0.5f,0.5f,0.5f,1,0,0,1,0,    0.5f,0.5f,-0.5f,1,0,0,1,1,    0.5f,-0.5f,-0.5f,1,0,0,0,1,
                  0.5f,-0.5f,-0.5f,1,0,0,0,1,   0.5f,-0.5f,0.5f,1,0,0,0,0,    0.5f,0.5f,0.5f,1,0,0,1,0,
                  -0.5f,-0.5f,-0.5f,0,-1,0,0,1, 0.5f,-0.5f,-0.5f,0,-1,0,1,1,  0.5f,-0.5f,0.5f,0,-1,0,1,0,
                  0.5f,-0.5f,0.5f,0,-1,0,1,0,   -0.5f,-0.5f,0.5f,0,-1,0,0,0,  -0.5f,-0.5f,-0.5f,0,-1,0,0,1,
                  -0.5f,0.5f,-0.5f,0,1,0,0,1,   0.5f,0.5f,-0.5f,0,1,0,1,1,    0.5f,0.5f,0.5f,0,1,0,1,0,
                  0.5f,0.5f,0.5f,0,1,0,1,0,    -0.5f,0.5f,0.5f,0,1,0,0,0,    -0.5f,0.5f,-0.5f,0,1,0,0,1 };
    for (int i = 0; i < 36; i++) {
        Vertex vert;
        int off = i * 8;
        vert.Position = { v[off],v[off + 1],v[off + 2] }; vert.Normal = { v[off + 3],v[off + 4],v[off + 5] }; vert.TexCoords = { v[off + 6],v[off + 7] };
        cubeMesh.vertices.push_back(vert); cubeMesh.indices.push_back(i);
    }
    cubeMesh.Setup();

    floorMesh.vertices = { {{-10,-1,10},{0,1,0},{0,0}}, {{10,-1,10},{0,1,0},{1,0}}, {{10,-1,-10},{0,1,0},{1,1}}, {{-10,-1,-10},{0,1,0},{0,1}} };
    floorMesh.indices = { 0,1,2, 0,2,3 };
    floorMesh.Setup();

    characterMesh = AssetLoader::LoadMesh("ExportChar.fbx");

    testObjects.push_back({ {-4, 1, 0}, {0,0,0}, {1,1,1}, Material::Standard({0,0,1}, 0.0f, 0.5f) });
    testObjects.push_back({ {-2, 1, 0}, {0,45,0}, {1,1,1}, Material::Standard({0,1,0}, 0.0f, 0.5f) });
    testObjects.push_back({ { 0, 1, 0}, {0,0,0}, {1,1,1}, Material::Standard({1,0,0}, 0.0f, 0.5f) });
    testObjects.push_back({ { 2, 1, 0}, {45,45,0}, {0.8f,0.8f,0.8f}, Material::Standard({1,0.5f,0}, 0.0f, 0.5f) });
    testObjects.push_back({ { 4, 1, 0}, {0,0,0}, {1,1,1}, Material::Emissive({1,1,1}, 5.0f) });

    testObjects.push_back({ {-4, 3, 0}, {0,0,0}, {1,1,1}, Material::Standard({0.9f,0.9f,0.9f}, 1.0f, 0.05f) }); // Full Metal Shiny
    testObjects.push_back({ {-2, 3, 0}, {0,0,0}, {1,1,1}, Material::Standard({0.9f,0.9f,0.9f}, 1.0f, 0.5f) }); // Metal Rough
    testObjects.push_back({ { 0, 3, 0}, {0,0,0}, {1,1,1}, Material::Standard({0.8f,0.8f,0.8f}, 0.0f, 1.0f) }); // Rough Plastic
    testObjects.push_back({ { 2, 3, 0}, {0,0,0}, {1,1,1}, Material::Glass({0.6f, 0.8f, 1.0f}) });
    testObjects.push_back({ { 4, 3, 0}, {0,0,0}, {1,1,1}, Material::Uranium({0.2f, 1.0f, 0.2f}) });
}

void Scene::ResetMaterialUniforms(unsigned int shader) {
    glUniform3f(glGetUniformLocation(shader, "material.albedo"), 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(shader, "material.metallic"), 0.0f);
    glUniform1f(glGetUniformLocation(shader, "material.roughness"), 0.5f);
    glUniform3f(glGetUniformLocation(shader, "material.emission"), 0.0f, 0.0f, 0.0f);
    glUniform1f(glGetUniformLocation(shader, "material.alpha"), 1.0f);
    glUniform1i(glGetUniformLocation(shader, "useTexture"), 0);
}

void Scene::Draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& camPos) {
    glDepthFunc(GL_LEQUAL);
    glUseProgram(skyProgram);
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, hdriTex);
    cubeMesh.Draw();
    glDepthFunc(GL_LESS);

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, &camPos[0]);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, hdriTex);
    glUniform1i(glGetUniformLocation(shaderProgram, "skybox"), 1);

    glUseProgram(doubleSidedProgram);
    glUniformMatrix4fv(glGetUniformLocation(doubleSidedProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(doubleSidedProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3fv(glGetUniformLocation(doubleSidedProgram, "viewPos"), 1, &camPos[0]);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, hdriTex);
    glUniform1i(glGetUniformLocation(doubleSidedProgram, "skybox"), 1);

    ResetMaterialUniforms(doubleSidedProgram);
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(doubleSidedProgram, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform1f(glGetUniformLocation(doubleSidedProgram, "material.roughness"), 0.8f);
    glUniform1i(glGetUniformLocation(doubleSidedProgram, "useTexture"), 1);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, whiteTex);
    glUniform1i(glGetUniformLocation(doubleSidedProgram, "tex1"), 0);
    floorMesh.Draw();

    ResetMaterialUniforms(doubleSidedProgram);
    model = glm::mat4(1.0f);

    for (const auto& obj : testObjects) {
        ResetMaterialUniforms(shaderProgram);

        model = glm::translate(glm::mat4(1.0f), obj.position);
        model = glm::rotate(model, glm::radians(obj.rotation.y), glm::vec3(0, 1, 0));
        model = glm::scale(model, obj.scale);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);

        glUniform3fv(glGetUniformLocation(shaderProgram, "material.albedo"), 1, &obj.material.albedo[0]);
        glUniform1f(glGetUniformLocation(shaderProgram, "material.metallic"), obj.material.metallic);
        glUniform1f(glGetUniformLocation(shaderProgram, "material.roughness"), obj.material.roughness);
        glUniform3fv(glGetUniformLocation(shaderProgram, "material.emission"), 1, &obj.material.emission[0]);
        glUniform1f(glGetUniformLocation(shaderProgram, "material.alpha"), obj.material.alpha);

        if (obj.material.alpha < 1.0f) glDepthMask(GL_FALSE);
        cubeMesh.Draw();
        if (obj.material.alpha < 1.0f) glDepthMask(GL_TRUE);
    }
    ResetMaterialUniforms(shaderProgram); 

    glUseProgram(doubleSidedProgram);
    glUniformMatrix4fv(glGetUniformLocation(doubleSidedProgram, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(doubleSidedProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniform3fv(glGetUniformLocation(doubleSidedProgram, "viewPos"), 1, &camPos[0]);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, hdriTex);
    glUniform1i(glGetUniformLocation(doubleSidedProgram, "skybox"), 1);

    ResetMaterialUniforms(doubleSidedProgram);

    model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(doubleSidedProgram, "model"), 1, GL_FALSE, &model[0][0]);

    glUniform3f(glGetUniformLocation(doubleSidedProgram, "material.albedo"), 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(doubleSidedProgram, "material.metallic"), 0.0f);
    glUniform1f(glGetUniformLocation(doubleSidedProgram, "material.roughness"), 0.8f);
    glUniform3f(glGetUniformLocation(doubleSidedProgram, "material.emission"), 0.0f, 0.0f, 0.0f);
    glUniform1f(glGetUniformLocation(doubleSidedProgram, "material.alpha"), 1.0f);

    glUniform1i(glGetUniformLocation(doubleSidedProgram, "useTexture"), 1);

    unsigned int texToUse = whiteTex;
    if (characterMesh.textureID != 0) {
        texToUse = characterMesh.textureID;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texToUse);
    glUniform1i(glGetUniformLocation(doubleSidedProgram, "tex1"), 0);

    characterMesh.Draw();
}

void Scene::SetupShaders() {
    auto Compile = [](const char* src, GLenum type) {
        unsigned int s = glCreateShader(type);
        glShaderSource(s, 1, &src, 0); glCompileShader(s); return s;
        };
    unsigned int vs = Compile(vsSrc, GL_VERTEX_SHADER);
    unsigned int fs = Compile(fsSrc, GL_FRAGMENT_SHADER);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs); glAttachShader(shaderProgram, fs); glLinkProgram(shaderProgram);

    unsigned int fsDouble = Compile(fsSrcDoubleSided, GL_FRAGMENT_SHADER);
    doubleSidedProgram = glCreateProgram();
    glAttachShader(doubleSidedProgram, vs); glAttachShader(doubleSidedProgram, fsDouble); glLinkProgram(doubleSidedProgram);

    unsigned int vs2 = Compile(vsSky, GL_VERTEX_SHADER);
    unsigned int fs2 = Compile(fsSky, GL_FRAGMENT_SHADER);
    skyProgram = glCreateProgram();
    glAttachShader(skyProgram, vs2); glAttachShader(skyProgram, fs2); glLinkProgram(skyProgram);
}

void Scene::Cleanup() {

}