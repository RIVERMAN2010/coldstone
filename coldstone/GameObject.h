#pragma once
#include <vector>
#include "Component.h"
#include <memory>

class GameObject {
public:
	// Retrieves a component of type T if it exists, otherwise returns nullptr
    template<typename T>
    T* getComponent();

	// Adds a new component of type T to the GameObject and returns a pointer to it
	//allows for construction with arguments, for example: addComponent<Transform>(x, y, z);
    template<typename T, typename... Args>
    T* addComponent(Args&&... args);

	//passes in a pointer to the component to be removed
	void removeComponent(Component* component);


private:
    std::vector<std::unique_ptr<Component>> components;
};

