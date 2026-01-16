#pragma once

#include "GPUResource.h"
#include <GL/glew.h>

class GLTexture : public GPUTexture {
public:
	GLuint getGLHandle() const { return textureID; }
private:
	GLuint textureID;
};
