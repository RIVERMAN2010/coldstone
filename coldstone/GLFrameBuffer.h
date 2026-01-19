#pragma once
#include <GL/glew.h>
#include <vector>
#include "GLTexture.h"

class GLFramebuffer {
public:
    GLFramebuffer(int width, int height);
    ~GLFramebuffer();

    void Resize(int width, int height);
    void Bind();
    void Unbind();

    void AttachTexture(GLTexture* texture, GLenum attachment);
    void Finalize();

    GLuint GetID() const { return fboID; }

private:
    GLuint fboID = 0;
    int width, height;
    std::vector<GLenum> drawBuffers;
};