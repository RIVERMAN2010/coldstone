#pragma once

class GameObject;

class Component {
public:
	virtual ~Component() = default;

	GameObject* owner = nullptr;
	GameObject* getOwner() const { return owner; }

	void update() {}
	void start() {}
	void destroy() {}
};

