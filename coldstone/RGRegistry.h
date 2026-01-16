#pragma once

#include "RGResource.h"
#include "GPUResource.h"
#include "RenderGraph.h"

class RGRegistry {
public:
	RGRegistry(RenderGraph&, RenderPass&);
	~RGRegistry();
	GPUTexture* getTexture(TextureHandle handle); //uses the handle to acquire the descriptor which is then used to get the actual texture from the resource pool
private:
	RenderGraph& graph;
	RenderPass& pass;
};