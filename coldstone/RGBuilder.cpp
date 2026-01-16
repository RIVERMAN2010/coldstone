#include "RGBuilder.h"

RGBuilder::RGBuilder(RenderGraph& _graph, RenderPass& _pass)
	: graph(_graph), pass(_pass) {
}
TextureHandle RGBuilder::read(TextureHandle handle) {
	pass.reads.push_back(handle.id);
	return handle;
}
TextureHandle RGBuilder::write(TextureHandle handle) {
	pass.writes.push_back(handle.id);
	return handle;
}
TextureHandle RGBuilder::create(const TextureDesc& desc, const std::string& name) {
	RGResource resource;
	resource.name = name;
	resource.id = static_cast<ResourceIdentifier>(graph.resources.size());
	resource.type = RGResource::Type::TEXTURE;
	resource.textureDesc = desc;
	graph.resources.push_back(resource);
	TextureHandle handle;
	handle.id = resource.id;
	pass.writes.push_back(handle.id);
	return handle;
}

BufferHandle RGBuilder::read(BufferHandle handle) {
	pass.reads.push_back(handle.id);
	return handle;
}
BufferHandle RGBuilder::write(BufferHandle handle) {
	pass.writes.push_back(handle.id);
	return handle;
}
BufferHandle RGBuilder::create(const BufferDesc& desc, const std::string& name) {
	RGResource resource;
	resource.name = name;
	resource.id = static_cast<ResourceIdentifier>(graph.resources.size());
	resource.type = RGResource::Type::BUFFER;
	resource.bufferDesc = desc;
	graph.resources.push_back(resource);
	BufferHandle handle;
	handle.id = resource.id;
	pass.writes.push_back(handle.id);
	return handle;
}