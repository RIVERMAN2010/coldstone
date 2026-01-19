#include "RenderGraph.h"
#include <iostream>
#include <format>

RenderGraph::RenderGraph(IResourcePool* pool) : resourcePool(pool) {
}

RenderGraph::~RenderGraph() {
    for (auto* res : physicalResources) {
        if (res && res->type == GPUResourceType::TEXTURE) {
            resourcePool->releaseTexture(static_cast<GPUTexture*>(res));
        }
    }
}

void RenderGraph::compile() {
    physicalResources.resize(resources.size(), nullptr);

    for (size_t i = 0; i < resources.size(); ++i) {
        const auto& logicalRes = resources[i];

        if (logicalRes.type == RGResource::Type::TEXTURE) {
            physicalResources[i] = resourcePool->allocateTexture(logicalRes.textureDesc);
        }
    }
}

void RenderGraph::execute(ICommandBuffer* cmdBuffer) {
    for (auto& pass : passes) {
        RGRegistry registry(*this, pass);
        pass.execute(registry, cmdBuffer);
    }
}

void RenderGraph::reset() {
    for (auto* res : physicalResources) {
        if (res) {
            resourcePool->releaseTexture(static_cast<GPUTexture*>(res));
        }
    }

    passes.clear();
    resources.clear();
    physicalResources.clear();
}