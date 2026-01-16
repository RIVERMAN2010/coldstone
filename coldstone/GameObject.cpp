#include "GameObject.h"


template<typename T>
T* GameObject::getComponent() {
    static_assert(std::is_base_of<Component, T>::value,
        "T must derive from Component");

    for (auto& c : components) {
        if (auto casted = dynamic_cast<T*>(c.get())) {
            return casted;
        }
    }
    return nullptr;
}

template<typename T, typename... Args>
T* GameObject::addComponent(Args&&... args) {
    static_assert(std::is_base_of<Component, T>::value,
        "T must derive from Component");
    
    auto component = std::make_unique<T>(std::forward<Args>(args)...);
    component->owner = this;
	component->start();

    T* ptr = component.get();
    components.emplace_back(std::move(component));

    return ptr;
}

void GameObject::removeComponent(Component* component) {
    components.erase(std::remove_if(components.begin(), components.end(),
        [component](const std::unique_ptr<Component>& c) {
            return c.get() == component;
        }), components.end());
}