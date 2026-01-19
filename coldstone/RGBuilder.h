#pragma once

#include "RGResource.h"
#include "RenderPass.h"
#include <string>

class RenderGraph;

class RGBuilder {
public:
	RGBuilder(RenderGraph&, RenderPass&);

	TextureHandle read(TextureHandle handle);
	TextureHandle write(TextureHandle handle);
	TextureHandle create(const TextureDesc& desc, const std::string& name);

	BufferHandle read(BufferHandle handle);
	BufferHandle write(BufferHandle handle);
	BufferHandle create(const BufferDesc& desc, const std::string& name);
private:
	RenderGraph& graph;
	RenderPass& pass;
};