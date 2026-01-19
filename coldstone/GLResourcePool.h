#pragma once

#include "IResourcePool.h"
#include "GLTexture.h"
#include <vector>
#include <list>

class GLResourcePool : public IResourcePool {
public:
	GLResourcePool();
	~GLResourcePool() override;
	GPUTexture* allocateTexture(const TextureDesc& desc) override;
	void releaseTexture(GPUTexture* texture) override;
	void destroy();

private:
	std::vector<GLTexture*> pool;
};