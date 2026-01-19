#include "GLCommandBuffer.h"
#include "RGRegistry.h"

void GLCommandBuffer::drawIndexed(unsigned int count) {
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
}

void GLCommandBuffer::bindTexture(int slot, ResourceHandle<Texture> handle) {
    if (!currentRegistry) return;
    GPUTexture* gpuTex = currentRegistry->getTexture(handle);
    if (gpuTex) {
        GLTexture* glTex = static_cast<GLTexture*>(gpuTex);
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, glTex->getGLHandle());
    }
}

void GLCommandBuffer::bindVertexArray(GLuint vao) {
    glBindVertexArray(vao);
}

void GLCommandBuffer::setViewport(int width, int height) {
    glViewport(0, 0, width, height);
}

void GLCommandBuffer::clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLCommandBuffer::bindShader(Shader* shader) {
    if (shader) shader->use();
}

void GLCommandBuffer::setUniformMat4(Shader* shader, const std::string& name, const glm::mat4& mat) {
    if (shader) shader->setMat4(name, mat);
}
void GLCommandBuffer::setUniformVec3(Shader* shader, const std::string& name, const glm::vec3& vec) {
    if (shader) {
        glUniform3fv(shader->getLocation(name), 1, &vec[0]);
    }
}
void GLCommandBuffer::setUniformVec2(Shader* shader, const std::string& name, const glm::vec2& vec) {
    if (shader) {
        glUniform2fv(shader->getLocation(name), 1, &vec[0]);
    }
}
void GLCommandBuffer::setUniformFloat(Shader* shader, const std::string& name, float val) {
    if (shader) shader->setFloat(name, val);
}
void GLCommandBuffer::setUniformInt(Shader* shader, const std::string& name, int val) {
    if (shader) shader->setInt(name, val);
}