#pragma once
#include "GPUResource.h"
#include <vector>
#include <unordered_map> 

class IResourcePool {
public:
	virtual ~IResourcePool() = default;
	virtual GPUTexture* allocateTexture(const TextureDesc& desc) = 0;
	virtual void releaseTexture(GPUTexture* texture) = 0;

private:
	std::vector<GPUResource*> allocatedResources;
};