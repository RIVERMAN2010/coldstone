#pragma once

#include "RGResource.h"
#include "GPUResource.h"

class RenderGraph;
struct RenderPass;

class RGRegistry {
public:
	RGRegistry(RenderGraph& graph, RenderPass& pass);
	~RGRegistry();
	GPUTexture* getTexture(TextureHandle handle);
private:
	RenderGraph& graph;
	RenderPass& pass;
};