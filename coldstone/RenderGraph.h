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
#include "IResourcePool.h"

class RenderGraph {
	friend class RGBuilder;
	friend class RGRegistry;
private:
	std::vector<RenderPass> passes;
	std::vector<RGResource> resources;

	std::vector<GPUResource*> physicalResources;
	IResourcePool* resourcePool = nullptr;

public:
	RenderGraph(IResourcePool* pool);
	~RenderGraph();

	template<typename T>
	void addPass(const std::string& name,
		std::function<void(RGBuilder&, T&)> setupFunc,
		std::function<void(T&, RGRegistry&, ICommandBuffer*)> executeFunc) {

		RenderPass pass;
		pass.name = name;

		T data;
		RGBuilder builder(*this, pass);
		setupFunc(builder, data);

		pass.execute = [=](RGRegistry& registry, ICommandBuffer* cmdBuffer) {
			T localData = data;
			executeFunc(localData, registry, cmdBuffer);
			};
		passes.push_back(pass);
	}

	void compile();
	void execute(ICommandBuffer* cmdBuffer);
	void reset();
};