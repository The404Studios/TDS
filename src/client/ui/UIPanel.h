#pragma once

#include "UIElement.h"

// UI Panel - A rectangular container for other UI elements
class UIPanel : public UIElement {
public:
    UIPanel(const std::string& name = "Panel");

    void render() override;

    // Border
    void setBorderColor(const Color& col) { borderColor = col; }
    void setBorderWidth(float w) { borderWidth = w; }
    void setDrawBorder(bool draw) { drawBorder = draw; }

    // Shadow
    void setShadow(bool shadow, float offsetX = 0.01f, float offsetY = 0.01f, float alpha = 0.4f) {
        hasShadow = shadow;
        shadowOffsetX = offsetX;
        shadowOffsetY = offsetY;
        shadowAlpha = alpha;
    }

    // Rounded corners
    void setRounded(bool rounded, float radius = 0.02f) {
        isRounded = rounded;
        cornerRadius = radius;
    }

private:
    Color borderColor;
    float borderWidth;
    bool drawBorder;
    bool hasShadow;
    float shadowOffsetX, shadowOffsetY, shadowAlpha;
    bool isRounded;
    float cornerRadius;
};
