#pragma once
#include "RenderGraph.h"
#include "GLCommandBuffer.h"
#include "Shader.h"
#include <vector>
#include <memory>

const char* GRID_VS = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
}
)";

const char* GRID_FS = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.6, 0.6, 0.6, 1.0); // Grey grid
}
)";

struct TestSceneResources {
    GLuint gridVAO = 0;
    GLuint gridVBO = 0;
    GLuint gridEBO = 0;
    unsigned int indexCount = 0;
    std::unique_ptr<Shader> gridShader;
    bool initialized = false;

    void init() {
        if (initialized) return;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        int slices = 10;
        float size = 10.0f;
        float step = size / slices;

        float v[] = {
             -5.0f, 0.0f,  5.0f, // BL
              5.0f, 0.0f,  5.0f, // BR
              5.0f, 0.0f, -5.0f, // TR
             -5.0f, 0.0f, -5.0f  // TL
        };
        unsigned int i[] = {
            0, 1, 2,
            2, 3, 0
        };

        glGenVertexArrays(1, &gridVAO);
        glGenBuffers(1, &gridVBO);
        glGenBuffers(1, &gridEBO);

        glBindVertexArray(gridVAO);

        glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(i), i, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        indexCount = 6;
        initialized = true;

    }
};

static TestSceneResources g_TestResources;

struct TestSceneData {
    TextureHandle output;
};

inline void SetupTestScene(RenderGraph& rg, int width, int height) {
    g_TestResources.init();

    rg.addPass<TestSceneData>("Grid Pass",
        [&](RGBuilder& builder, TestSceneData& data) {
            TextureDesc desc = { (uint32_t)width, (uint32_t)height, TextureFormat::RGBA16F, false };
            data.output = builder.create(desc, "SceneColor");
        },
        [&](TestSceneData& data, RGRegistry& registry, ICommandBuffer* cmd) {
            GLCommandBuffer* glCmd = static_cast<GLCommandBuffer*>(cmd);
            glCmd->setRegistry(&registry);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glCmd->setViewport(width, height);
            glCmd->clear(0.1f, 0.1f, 0.1f, 1.0f);

            glCmd->bindVertexArray(g_TestResources.gridVAO);
            glCmd->drawIndexed(g_TestResources.indexCount);
        }
    );
}