#pragma once

#include "GLResource.h"

class GLCommandBuffer {
public:
	void drawIndexed(unsigned int count);
	void bindVertexArray();
};