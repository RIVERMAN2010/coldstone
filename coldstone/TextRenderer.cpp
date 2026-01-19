#include "TextRenderer.h"
#include <glm/gtc/matrix_transform.hpp> 
#include <vector>
#include <cstring>

const char* DIGIT_VS = R"(
#version 460 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
uniform vec2 offset;
uniform float scale;
void main() {
    vec2 pos = aPos * scale + offset;
    gl_Position = projection * vec4(pos, 0.0, 1.0);
}
)";

const char* DIGIT_FS = R"(
#version 460 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
}
)";

const std::vector<std::vector<int>> NUMBERS = {
    {0,1,2,3,4,5},   // 0
    {1,2},           // 1
    {0,1,6,4,3},     // 2
    {0,1,6,2,3},     // 3
    {5,6,1,2},       // 4
    {0,5,6,2,3},     // 5
    {0,5,4,3,2,6},   // 6
    {0,1,2},         // 7
    {0,1,2,3,4,5,6}, // 8
    {0,5,6,1,2,3}    // 9
};

void DigitRenderer::Init() {
    shader = new Shader(DIGIT_VS, DIGIT_FS, true);

    float segments[] = {
        0,1, 1,1, // 0 Top
        1,1, 1,0.5f, // 1 TopRight
        1,0.5f, 1,0, // 2 BotRight
        0,0, 1,0, // 3 Bot
        0,0, 0,0.5f, // 4 BotLeft
        0,0.5f, 0,1, // 5 TopLeft
        0,0.5f, 1,0.5f // 6 Mid
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(segments), segments, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
}

void DigitRenderer::RenderString(GLCommandBuffer* cmd, const std::string& text, float x, float y, float scale, int screenW, int screenH) {
    if (!shader) return;

    cmd->bindShader(shader);
    glm::mat4 proj = glm::ortho(0.0f, (float)screenW, 0.0f, (float)screenH);
    cmd->setUniformMat4(shader, "projection", proj);
    cmd->setUniformVec3(shader, "color", { 1.0f, 1.0f, 0.0f });
    cmd->setUniformFloat(shader, "scale", scale * 20.0f);

    glBindVertexArray(vao);

    float cursorX = x;
    for (char c : text) {
        if (c >= '0' && c <= '9') {
            int num = c - '0';
            cmd->setUniformVec2(shader, "offset", glm::vec2(cursorX, y));

            const auto& segs = NUMBERS[num];
            for (int segIndex : segs) {
                glDrawArrays(GL_LINES, segIndex * 2, 2);
            }
        }
        cursorX += scale * 25.0f;
    }
}