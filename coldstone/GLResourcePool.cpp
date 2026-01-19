#include "GLResourcePool.h"
#include <iostream>
#include <format>
#include <algorithm>

GLResourcePool::GLResourcePool() {
}

GLResourcePool::~GLResourcePool() {
	destroy();
}

void GLResourcePool::destroy() {
	for (auto* tex : pool) {
		glDeleteTextures(1, &tex->textureID);
		delete tex;
	}
	pool.clear();
}

GPUTexture* GLResourcePool::allocateTexture(const TextureDesc& desc) {
	for (auto* tex : pool) {
		if (tex->free &&
			tex->desc.width == desc.width &&
			tex->desc.height == desc.height &&
			tex->desc.format == desc.format) {

			tex->free = false;
			return tex;
		}
	}

	GLTexture* texture = new GLTexture();
	texture->desc = desc;
	texture->free = false;

	glGenTextures(1, &texture->textureID);
	glBindTexture(GL_TEXTURE_2D, texture->textureID);

	GLint internalFormat = GL_RGBA8;
	GLenum format = GL_RGBA;
	GLenum type = GL_UNSIGNED_BYTE;

	switch (desc.format) {
	case TextureFormat::RGBA8:
		internalFormat = GL_RGBA8; format = GL_RGBA; type = GL_UNSIGNED_BYTE; break;
	case TextureFormat::RGB8:
		internalFormat = GL_RGB8; format = GL_RGB; type = GL_UNSIGNED_BYTE; break;
	case TextureFormat::RGBA16F:
		internalFormat = GL_RGBA16F; format = GL_RGBA; type = GL_FLOAT; break;
	case TextureFormat::RGBA32F:
		internalFormat = GL_RGBA32F; format = GL_RGBA; type = GL_FLOAT; break;
	case TextureFormat::DEPTH24STENCIL8:
		internalFormat = GL_DEPTH24_STENCIL8; format = GL_DEPTH_STENCIL; type = GL_UNSIGNED_INT_24_8; break;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, desc.width, desc.height, 0, format, type, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	pool.push_back(texture);

	return texture;
}

void GLResourcePool::releaseTexture(GPUTexture* texture) {
	if (texture) {
		texture->free = true;
	}
}