#pragma once
#include "Shader.h"
#include <GL/glew.h>

struct GLPipeline {
    Shader* shader = nullptr;

    bool depthTest = true;
    bool depthWrite = true;
    GLenum depthFunc = GL_LESS;

    bool blend = false;
    GLenum blendSrc = GL_SRC_ALPHA;
    GLenum blendDst = GL_ONE_MINUS_SRC_ALPHA;

    bool cullFace = true;
    GLenum cullMode = GL_BACK;

    void Bind() {
        if (shader) shader->use();

        if (depthTest) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(depthFunc);
            glDepthMask(depthWrite ? GL_TRUE : GL_FALSE);
        }
        else {
            glDisable(GL_DEPTH_TEST);
        }

        if (blend) {
            glEnable(GL_BLEND);
            glBlendFunc(blendSrc, blendDst);
        }
        else {
            glDisable(GL_BLEND);
        }

        if (cullFace) {
            glEnable(GL_CULL_FACE);
            glCullFace(cullMode);
        }
        else {
            glDisable(GL_CULL_FACE);
        }
    }
};