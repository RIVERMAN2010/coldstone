#include "GLFramebuffer.h"
#include <iostream>
#include <format>

GLFramebuffer::GLFramebuffer(int w, int h) : width(w), height(h) {
    glGenFramebuffers(1, &fboID);
}

GLFramebuffer::~GLFramebuffer() {
    glDeleteFramebuffers(1, &fboID);
}

void GLFramebuffer::Resize(int w, int h) {
    width = w;
    height = h;
}

void GLFramebuffer::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, fboID);
    glViewport(0, 0, width, height);
}

void GLFramebuffer::Unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLFramebuffer::AttachTexture(GLTexture* texture, GLenum attachment) {
    Bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture->getGLHandle(), 0);

    if (attachment != GL_DEPTH_STENCIL_ATTACHMENT && attachment != GL_DEPTH_ATTACHMENT) {
        drawBuffers.push_back(attachment);
    }
}

void GLFramebuffer::Finalize() {
    Bind();
    if (drawBuffers.empty()) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    else {
        glDrawBuffers((GLsizei)drawBuffers.size(), drawBuffers.data());
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    drawBuffers.clear();
}