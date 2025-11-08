#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

// UI Transform component for position, rotation, scale (2D)
struct UITransform {
    float x, y, z;
    float rotation;
    float scaleX, scaleY;

    UITransform() : x(0), y(0), z(0), rotation(0), scaleX(1), scaleY(1) {}
    UITransform(float px, float py) : x(px), y(py), z(0), rotation(0), scaleX(1), scaleY(1) {}
};

// Base GameObject class with hierarchy support
class GameObject {
public:
    GameObject(const std::string& name = "GameObject");
    virtual ~GameObject();

    // Hierarchy management
    void addChild(std::shared_ptr<GameObject> child);
    void removeChild(std::shared_ptr<GameObject> child);
    void setParent(GameObject* parent);
    GameObject* getParent() const { return parent; }
    const std::vector<std::shared_ptr<GameObject>>& getChildren() const { return children; }

    // Lifecycle methods
    virtual void onLoad() {}
    virtual void onUnload() {}
    virtual void update(float deltaTime);
    virtual void render();

    // Transform
    UITransform& getTransform() { return transform; }
    const UITransform& getTransform() const { return transform; }
    UITransform getWorldTransform() const;

    // Properties
    void setActive(bool active) { this->active = active; }
    bool isActive() const { return active; }
    void setName(const std::string& name) { this->name = name; }
    const std::string& getName() const { return name; }

    // Tags for searching
    void setTag(const std::string& tag) { this->tag = tag; }
    const std::string& getTag() const { return tag; }

    // Find children by name or tag
    std::shared_ptr<GameObject> findChild(const std::string& name);
    std::shared_ptr<GameObject> findChildWithTag(const std::string& tag);
    void findChildrenWithTag(const std::string& tag, std::vector<std::shared_ptr<GameObject>>& result);

protected:
    std::string name;
    std::string tag;
    bool active;
    UITransform transform;
    GameObject* parent;
    std::vector<std::shared_ptr<GameObject>> children;
};
