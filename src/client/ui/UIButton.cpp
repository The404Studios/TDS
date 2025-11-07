#include "UIButton.h"
#include <cmath>

UIButton::UIButton(const std::string& name, const std::string& label)
    : UIPanel(name), animated(true), animTime(0.0f) {

    normalColor = Color(0.15f, 0.15f, 0.2f, 0.95f);
    hoverColor = Color(0.25f, 0.22f, 0.18f, 0.95f);
    setColor(normalColor);
    originalColor = normalColor;

    // Create label as child
    labelText = std::make_shared<UIText>("Label", label);
    labelText->setAlignment(TextAlign::CENTER);
    labelText->setColor(Color(0.9f, 0.9f, 0.9f, 1.0f));
    labelText->getTransform().y = 0.0f;
    labelText->setSize(1.0f, 1.0f);
    addChild(labelText);

    // Enable border by default
    setDrawBorder(true);
    setBorderColor(Color(0.4f, 0.35f, 0.25f, 1.0f));
    setBorderWidth(2.0f);

    // Enable shadow by default
    setShadow(true, 0.015f, 0.015f, 0.4f);
}

void UIButton::setLabel(const std::string& label) {
    if (labelText) {
        labelText->setText(label);
    }
}

std::string UIButton::getLabel() const {
    return labelText ? labelText->getText() : "";
}

void UIButton::onClick() {
    if (onClickCallback) {
        onClickCallback();
    }
}

void UIButton::onMouseEnter() {
    setColor(hoverColor);
    if (labelText) {
        labelText->setColor(Color(1.0f, 0.95f, 0.8f, 1.0f));
    }
}

void UIButton::onMouseExit() {
    setColor(normalColor);
    if (labelText) {
        labelText->setColor(Color(0.9f, 0.9f, 0.9f, 1.0f));
    }
}

void UIButton::update(float deltaTime) {
    animTime += deltaTime;

    // Hover animation
    if (animated && hovered) {
        float pulse = 0.9f + 0.1f * std::sinf(animTime * 3.0f);
        Color animColor = hoverColor;
        animColor.r *= pulse;
        animColor.g *= pulse;
        animColor.b *= pulse;
        setColor(animColor);
    }

    UIPanel::update(deltaTime);
}

void UIButton::render() {
    // Update label size and position to match button
    if (labelText) {
        labelText->setSize(width, height);
        labelText->getTransform().x = 0;
        labelText->getTransform().y = height / 2.0f - 0.02f;
    }

    UIPanel::render();
}
