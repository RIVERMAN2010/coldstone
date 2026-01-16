#pragma once
#include "RGResource.h"

enum class GPUResourceType {
	TEXTURE,
	BUFFER
};

class GPUResource {
public:
	GPUResourceType type;
	bool free = true;
	bool deleted = false;
	virtual ~GPUResource() = default;
};

class GPUTexture : public GPUResource {
public:
	TextureDesc desc;
};