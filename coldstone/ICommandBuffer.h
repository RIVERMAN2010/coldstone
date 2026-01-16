#pragma once
#include "RGResource.h"

class ICommandBuffer {
public:
	virtual void drawIndexed(unsigned int count) = 0;
	virtual void bindTexture(int slot, ResourceHandle<Texture> handle) = 0;
	virtual ~ICommandBuffer() = default;

};