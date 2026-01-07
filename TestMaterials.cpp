#include "TestMaterials.h"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

std::vector<TestObject> TestMaterials::objects;
Mesh TestMaterials::cubeMesh;

void TestMaterials::Initialize() {
    objects.clear();

    float v[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    cubeMesh.vertices.clear();
    cubeMesh.indices.clear();
    for (int i = 0; i < 36; i++) {
        Vertex vert;
        int off = i * 8;
        vert.Position = { v[off], v[off + 1], v[off + 2] };
        vert.Normal = { v[off + 3], v[off + 4], v[off + 5] };
        vert.TexCoords = { v[off + 6], v[off + 7] };
        cubeMesh.vertices.push_back(vert);
        cubeMesh.indices.push_back(i);
    }
    cubeMesh.Setup();

    AddCube({ -4, 1, 0 }, { 0, 0, 0 }, { 1, 1, 1 }, Material::Standard({ 0,0,1 }, 0.0f, 0.5f));
    AddCube({ -2, 1, 0 }, { 0, 45, 0 }, { 1, 1, 1 }, Material::Standard({ 0,1,0 }, 0.0f, 0.5f));
    AddCube({ 0, 1, 0 }, { 0, 0, 0 }, { 1, 1, 1 }, Material::Standard({ 1,0,0 }, 0.0f, 0.5f));
    AddCube({ 2, 1, 0 }, { 45, 45, 0 }, { 0.8f, 0.8f, 0.8f }, Material::Standard({ 1,0.5f,0 }, 0.0f, 0.5f));
    AddCube({ 4, 1, 0 }, { 0, 0, 0 }, { 1, 1, 1 }, Material::Emissive({ 1,1,1 }, 5.0f));

    AddCube({ -4, 3, 0 }, { 0, 0, 0 }, { 1, 1, 1 }, Material::Standard({ 0.8f,0.8f,0.8f }, 1.0f, 0.1f));
    AddCube({ -2, 3, 0 }, { 0, 0, 0 }, { 1, 1, 1 }, Material::Standard({ 0.8f,0.8f,0.8f }, 0.5f, 0.5f));
    AddCube({ 0, 3, 0 }, { 0, 0, 0 }, { 1, 1, 1 }, Material::Standard({ 0.8f,0.8f,0.8f }, 0.0f, 1.0f));
    AddCube({ 2, 3, 0 }, { 0, 0, 0 }, { 1, 1, 1 }, Material::Glass({ 0.6f, 0.8f, 1.0f }));
    AddCube({ 4, 3, 0 }, { 0, 0, 0 }, { 1, 1, 1 }, Material::Uranium({ 0.2f, 1.0f, 0.2f }));
}

void TestMaterials::AddCube(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, Material mat) {
    objects.push_back({ pos, rot, scale, mat });
}

void TestMaterials::Draw(unsigned int shaderProgram) {
    glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);

    for (const auto& obj : objects) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, obj.position);
        model = glm::rotate(model, glm::radians(obj.rotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(obj.rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(obj.rotation.z), glm::vec3(0, 0, 1));
        model = glm::scale(model, obj.scale);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);

        const Material& m = obj.material;
        glUniform3fv(glGetUniformLocation(shaderProgram, "material.albedo"), 1, &m.albedo[0]);
        glUniform1f(glGetUniformLocation(shaderProgram, "material.metallic"), m.metallic);
        glUniform1f(glGetUniformLocation(shaderProgram, "material.roughness"), m.roughness);
        glUniform3fv(glGetUniformLocation(shaderProgram, "material.emission"), 1, &m.emission[0]);
        glUniform1f(glGetUniformLocation(shaderProgram, "material.alpha"), m.alpha);

        if (m.alpha < 1.0f) glDepthMask(GL_FALSE);
        cubeMesh.Draw();
        if (m.alpha < 1.0f) glDepthMask(GL_TRUE);
    }
}

void TestMaterials::DrawCubeMesh() {
    cubeMesh.Draw();
}

void TestMaterials::Cleanup() {
    objects.clear();
}