#include "Scene.h"
#include <algorithm>

Scene::Scene(const std::string& name)
    : name(name), loaded(false), onLoadCallback(nullptr), onUnloadCallback(nullptr) {
}

Scene::~Scene() {
    if (loaded) {
        onUnload();
    }
}

void Scene::onLoad() {
    if (loaded) return;

    loaded = true;

    // Load all root objects
    for (auto& obj : rootObjects) {
        if (obj) {
            obj->onLoad();
        }
    }

    // Call custom callback
    if (onLoadCallback) {
        onLoadCallback();
    }
}

void Scene::onUnload() {
    if (!loaded) return;

    // Unload all root objects
    for (auto& obj : rootObjects) {
        if (obj) {
            obj->onUnload();
        }
    }

    // Call custom callback
    if (onUnloadCallback) {
        onUnloadCallback();
    }

    loaded = false;
}

void Scene::update(float deltaTime) {
    if (!loaded) return;

    for (auto& obj : rootObjects) {
        if (obj && obj->isActive()) {
            obj->update(deltaTime);
        }
    }
}

void Scene::render() {
    if (!loaded) return;

    for (auto& obj : rootObjects) {
        if (obj && obj->isActive()) {
            obj->render();
        }
    }
}

void Scene::addRootObject(std::shared_ptr<GameObject> obj) {
    if (obj) {
        rootObjects.push_back(obj);
        if (loaded) {
            obj->onLoad();
        }
    }
}

void Scene::removeRootObject(std::shared_ptr<GameObject> obj) {
    auto it = std::find(rootObjects.begin(), rootObjects.end(), obj);
    if (it != rootObjects.end()) {
        (*it)->onUnload();
        rootObjects.erase(it);
    }
}

std::shared_ptr<GameObject> Scene::findObject(const std::string& name) {
    for (auto& obj : rootObjects) {
        if (obj && obj->getName() == name) {
            return obj;
        }
        auto child = obj->findChild(name);
        if (child) {
            return child;
        }
    }
    return nullptr;
}

std::shared_ptr<GameObject> Scene::findObjectWithTag(const std::string& tag) {
    for (auto& obj : rootObjects) {
        if (obj && obj->getTag() == tag) {
            return obj;
        }
        auto child = obj->findChildWithTag(tag);
        if (child) {
            return child;
        }
    }
    return nullptr;
}
