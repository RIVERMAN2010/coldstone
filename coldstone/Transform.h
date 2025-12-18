#pragma once
#include <glm/glm.hpp>
#include "Component.h"

class Transform : public Component {
public:
	glm::vec3 position;
	glm::vec2 size;
};