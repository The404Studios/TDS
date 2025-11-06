#pragma once

#include "UIPanel.h"
#include "UIText.h"
#include <functional>
#include <memory>

// UI Button - Interactive button with text
class UIButton : public UIPanel {
public:
    UIButton(const std::string& name = "Button", const std::string& label = "");

    void render() override;
    void update(float deltaTime) override;

    // Button label
    void setLabel(const std::string& label);
    std::string getLabel() const;

    // Click callback
    void setOnClick(std::function<void()> callback) { onClickCallback = callback; }
    void onClick() override;

    // Hover effects
    void onMouseEnter() override;
    void onMouseExit() override;

    // Hover colors
    void setHoverColor(const Color& col) { hoverColor = col; }
    void setNormalColor(const Color& col) { normalColor = col; setColor(col); }

    // Animation
    void setAnimated(bool anim) { animated = anim; }

private:
    std::shared_ptr<UIText> labelText;
    std::function<void()> onClickCallback;
    Color normalColor;
    Color hoverColor;
    bool animated;
    float animTime;
    Color originalColor;
};
