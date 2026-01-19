#pragma once

class GameObject;

class Component {
public:
    virtual ~Component() = default;

    GameObject* owner = nullptr;

    GameObject* getOwner() const { return owner; }

    virtual void start() {}
    virtual void update() {}
    virtual void destroy() {}
};