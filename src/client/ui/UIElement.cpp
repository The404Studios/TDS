#include "UIElement.h"

UIElement::UIElement(const std::string& name)
    : GameObject(name), width(1.0f), height(1.0f), visible(true), hovered(false) {
    color = Color(1, 1, 1, 1);
}

void UIElement::render() {
    if (!visible || !active) return;

    // Render children
    GameObject::render();
}

bool UIElement::containsPoint(float x, float y) const {
    Transform worldTrans = getWorldTransform();
    return (x >= worldTrans.x && x <= worldTrans.x + width &&
            y >= worldTrans.y && y <= worldTrans.y + height);
}

void UIElement::drawBox(float x, float y, float w, float h, bool filled) {
    if (filled) {
        glBegin(GL_QUADS);
    } else {
        glBegin(GL_LINE_LOOP);
    }
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void UIElement::setGLColor(const Color& col) {
    glColor4f(col.r, col.g, col.b, col.a);
}
