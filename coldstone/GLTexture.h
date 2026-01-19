#pragma once

#include "GPUResource.h"
#include <GL/glew.h>

class GLTexture : public GPUTexture {
public:
	friend class GLResourcePool;

	GLuint getGLHandle() const { return textureID; }

	GLTexture() : textureID(0) {}

	~GLTexture() override {
		if (textureID != 0) {
		}
	}

private:
	GLuint textureID;
};