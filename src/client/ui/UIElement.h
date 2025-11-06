#pragma once

#include "GameObject.h"
#include <windows.h>
#include <GL/gl.h>
#include <functional>

// Color struct
struct Color {
    float r, g, b, a;
    Color() : r(1), g(1), b(1), a(1) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
};

// Base UI Element class
class UIElement : public GameObject {
public:
    UIElement(const std::string& name = "UIElement");
    virtual ~UIElement() = default;

    // Rendering
    virtual void render() override;

    // Size
    void setSize(float w, float h) { width = w; height = h; }
    float getWidth() const { return width; }
    float getHeight() const { return height; }

    // Color
    void setColor(const Color& col) { color = col; }
    const Color& getColor() const { return color; }

    // Visibility
    void setVisible(bool vis) { visible = vis; }
    bool isVisible() const { return visible; }

    // Mouse interaction
    virtual bool containsPoint(float x, float y) const;
    virtual void onMouseEnter() {}
    virtual void onMouseExit() {}
    virtual void onClick() {}

    // Hover state
    void setHovered(bool hov) { hovered = hov; }
    bool isHovered() const { return hovered; }

protected:
    // Helper rendering functions
    void drawBox(float x, float y, float w, float h, bool filled = false);
    void setGLColor(const Color& col);

    float width, height;
    Color color;
    bool visible;
    bool hovered;
};
