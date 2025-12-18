#pragma once
#include <glm/glm.hpp>
#include "Component.h" 
#include "Texture.h"
#include <memory>

class Sprite : public Component{
public:
	bool visible = true;
	std::shared_ptr<Texture> texture;
};