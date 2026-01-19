#include "RGRegistry.h"
#include "RenderGraph.h"

RGRegistry::RGRegistry(RenderGraph& _graph, RenderPass& _pass)
    : graph(_graph), pass(_pass) {
}

RGRegistry::~RGRegistry() {
}

GPUTexture* RGRegistry::getTexture(TextureHandle handle) {
    if (handle.id < graph.physicalResources.size()) {
        return static_cast<GPUTexture*>(graph.physicalResources[handle.id]);
    }
    return nullptr;
}