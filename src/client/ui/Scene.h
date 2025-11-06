#pragma once

#include "GameObject.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>

// Scene - Container for GameObjects with lifecycle management
class Scene {
public:
    Scene(const std::string& name);
    virtual ~Scene();

    // Scene lifecycle
    virtual void onLoad();
    virtual void onUnload();
    virtual void update(float deltaTime);
    virtual void render();

    // GameObject management
    void addRootObject(std::shared_ptr<GameObject> obj);
    void removeRootObject(std::shared_ptr<GameObject> obj);
    std::shared_ptr<GameObject> findObject(const std::string& name);
    std::shared_ptr<GameObject> findObjectWithTag(const std::string& tag);

    // Properties
    const std::string& getName() const { return name; }
    bool isLoaded() const { return loaded; }

    // Async loading support
    void setLoadCallback(std::function<void()> callback) { onLoadCallback = callback; }
    void setUnloadCallback(std::function<void()> callback) { onUnloadCallback = callback; }

protected:
    std::string name;
    bool loaded;
    std::vector<std::shared_ptr<GameObject>> rootObjects;
    std::function<void()> onLoadCallback;
    std::function<void()> onUnloadCallback;
};
