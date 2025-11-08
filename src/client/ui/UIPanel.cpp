#include "UIPanel.h"

UIPanel::UIPanel(const std::string& name)
    : UIElement(name), borderWidth(2.0f), drawBorder(false),
      hasShadow(false), shadowOffsetX(0.01f), shadowOffsetY(0.01f), shadowAlpha(0.4f),
      isRounded(false), cornerRadius(0.02f) {
    borderColor = Color(0.4f, 0.35f, 0.25f, 1.0f);
}

void UIPanel::render() {
    if (!visible || !active) return;

    UITransform worldTrans = getWorldTransform();
    float x = worldTrans.x;
    float y = worldTrans.y;
    float w = width * worldTrans.scaleX;
    float h = height * worldTrans.scaleY;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw shadow
    if (hasShadow) {
        glColor4f(0.0f, 0.0f, 0.0f, shadowAlpha);
        drawBox(x + shadowOffsetX, y - shadowOffsetY, w, h, true);
    }

    // Draw panel background
    setGLColor(color);
    drawBox(x, y, w, h, true);

    // Draw border
    if (drawBorder) {
        setGLColor(borderColor);
        glLineWidth(borderWidth);
        drawBox(x, y, w, h, false);
        glLineWidth(1.0f);
    }

    // Render children
    UIElement::render();
}
