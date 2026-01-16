#pragma once

#include "IResourcePool.h"
#include "GLTexture.h"
#include <vector>
#include <unordered_map>

class GLResourcePool : public IResourcePool {
public:
	GLResourcePool();
	~GLResourcePool() override;
	GPUTexture* allocateTexture(const TextureDesc& desc) override;
	void releaseTexture(GPUTexture* texture) override;
private:
	
};