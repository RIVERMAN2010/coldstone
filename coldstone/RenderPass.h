#pragma once
#include <string>
#include <vector>
#include <functional>
#include "RGResource.h"

class RGBuilder;
class RGRegistry;
class ICommandBuffer;

struct RenderPass {
	std::string name;
	enum class PassType {
		GRAPHICS,
		COMPUTE
	} type;

	std::vector<ResourceIdentifier> reads;
	std::vector<ResourceIdentifier> writes;

	std::function<void(RGBuilder&)> setup;
	std::function<void(RGRegistry&, ICommandBuffer*)> execute;
};