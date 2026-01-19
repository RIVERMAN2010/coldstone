#pragma once
#include "Shader.h"
#include "GLCommandBuffer.h"
#include <string>
#include <vector>

class DigitRenderer {
public:
    void Init();
    void RenderString(GLCommandBuffer* cmd, const std::string& text, float x, float y, float scale, int screenW, int screenH);
private:
    GLuint vao, vbo;
    Shader* shader = nullptr;
};