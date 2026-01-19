#pragma once
#include "ICommandBuffer.h"
#include "GLTexture.h"
#include "Shader.h"
#include "Mesh.h"
#include <GL/glew.h>

class GLCommandBuffer : public ICommandBuffer {
public:
    void drawIndexed(unsigned int count) override;
    void bindTexture(int slot, ResourceHandle<Texture> handle) override;
    void setRegistry(class RGRegistry* reg) { currentRegistry = reg; }

    void bindVertexArray(GLuint vao);
    void setViewport(int width, int height);
    void clear(float r, float g, float b, float a);

    void bindShader(Shader* shader);
    void setUniformMat4(Shader* shader, const std::string& name, const glm::mat4& mat);
    void setUniformVec3(Shader* shader, const std::string& name, const glm::vec3& vec);
    void setUniformVec2(Shader* shader, const std::string& name, const glm::vec2& vec);
    void setUniformFloat(Shader* shader, const std::string& name, float val);
    void setUniformInt(Shader* shader, const std::string& name, int val);

private:
    class RGRegistry* currentRegistry = nullptr;
};