#pragma once

#include "UIElement.h"
#include <string>

// Text alignment
enum class TextAlign {
    LEFT,
    CENTER,
    RIGHT
};

// UI Text - Renders text
class UIText : public UIElement {
public:
    UIText(const std::string& name = "Text", const std::string& text = "");

    void render() override;

    // Text properties
    void setText(const std::string& txt) { text = txt; }
    const std::string& getText() const { return text; }

    void setFontSize(float size) { fontSize = size; }
    float getFontSize() const { return fontSize; }

    void setAlignment(TextAlign align) { alignment = align; }
    TextAlign getAlignment() const { return alignment; }

    // Shadow
    void setShadow(bool shadow, float offsetX = 0.01f, float offsetY = 0.01f, float alpha = 0.4f) {
        hasShadow = shadow;
        shadowOffsetX = offsetX;
        shadowOffsetY = offsetY;
        shadowAlpha = alpha;
    }

    // Static font initialization
    static void initFont(HDC hDC);
    static bool isFontInitialized() { return g_fontInitialized; }

private:
    std::string text;
    float fontSize;
    TextAlign alignment;
    bool hasShadow;
    float shadowOffsetX, shadowOffsetY, shadowAlpha;

    static GLuint g_fontBase;
    static HDC g_hDC;
    static bool g_fontInitialized;

    void renderText(const std::string& txt, float x, float y, float size);
};
