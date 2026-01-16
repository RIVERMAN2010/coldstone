#pragma once
#include <string>
#include <vector>
#include <functional>
#include "RGResource.h"
#include "RGBuilder.h"
#include "ICommandBuffer.h"
#include "RGRegistry.h"
#include "RenderPass.h"
#include "GPUResource.h"

class RenderGraph {
	friend class RGBuilder;
	friend class RGRegistry;
private:
	std::vector<RenderPass> passes;
	std::vector<RGResource> resources;

	std::vector<GPUResource*> physicalResources; // alligns with resources by index, i.e. physicalResources[i] is the actual GPU resource for resources[i]
	//the ordering of physicalResources is determined during compile() and the resources are acquired from the resource pool then released during destruction of the RenderGraph
	
public:
	template<typename T>
	void addPass(const std::string& name,
		std::function<void(RGBuilder&, T&)> setupFunc,
		std::function<void(T&, RGRegistry&, ICommandBuffer*)> executeFunc);

	void compile();
	void execute(ICommandBuffer* cmdBuffer);

};

