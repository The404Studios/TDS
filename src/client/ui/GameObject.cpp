#include "GameObject.h"
#include <algorithm>

GameObject::GameObject(const std::string& name)
    : name(name), tag(""), active(true), parent(nullptr) {
}

GameObject::~GameObject() {
    onUnload();
}

void GameObject::addChild(std::shared_ptr<GameObject> child) {
    if (child) {
        child->setParent(this);
        children.push_back(child);
        if (active) {
            child->onLoad();
        }
    }
}

void GameObject::removeChild(std::shared_ptr<GameObject> child) {
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        (*it)->setParent(nullptr);
        (*it)->onUnload();
        children.erase(it);
    }
}

void GameObject::setParent(GameObject* newParent) {
    parent = newParent;
}

UITransform GameObject::getWorldTransform() const {
    UITransform world = transform;
    if (parent) {
        UITransform parentWorld = parent->getWorldTransform();
        world.x += parentWorld.x;
        world.y += parentWorld.y;
        world.z += parentWorld.z;
        world.rotation += parentWorld.rotation;
        world.scaleX *= parentWorld.scaleX;
        world.scaleY *= parentWorld.scaleY;
    }
    return world;
}

void GameObject::update(float deltaTime) {
    if (!active) return;

    // Update all children
    for (auto& child : children) {
        if (child && child->isActive()) {
            child->update(deltaTime);
        }
    }
}

void GameObject::render() {
    if (!active) return;

    // Render all children
    for (auto& child : children) {
        if (child && child->isActive()) {
            child->render();
        }
    }
}

std::shared_ptr<GameObject> GameObject::findChild(const std::string& name) {
    for (auto& child : children) {
        if (child && child->getName() == name) {
            return child;
        }
    }
    return nullptr;
}

std::shared_ptr<GameObject> GameObject::findChildWithTag(const std::string& tag) {
    for (auto& child : children) {
        if (child && child->getTag() == tag) {
            return child;
        }
    }
    return nullptr;
}

void GameObject::findChildrenWithTag(const std::string& tag, std::vector<std::shared_ptr<GameObject>>& result) {
    for (auto& child : children) {
        if (child && child->getTag() == tag) {
            result.push_back(child);
        }
        child->findChildrenWithTag(tag, result);
    }
}
