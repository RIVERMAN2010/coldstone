#include "RenderGraph.h"

template<typename T>
void RenderGraph::addPass(const std::string& name,
	std::function<void(RGBuilder&, T&)> setup,
	std::function<void(T&, RGRegistry&, ICommandBuffer*)> execute) {

	RenderPass pass;
	pass.name = name;

	T data;
	RGBuilder builder(*this, pass);
	setup(builder);

	pass.execute = [=](RGRegistry& registry, ICommandBuffer* cmdBuffer) {
		execute(data, registry, cmdBuffer);
	};
	passes.push_back(pass);
}

