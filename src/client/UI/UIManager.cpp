#include "UIManager.h"

// Define raygui implementation (header-only library)
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "raylib.h"

namespace TDS {

// Initialize static theme
UITheme UIManager::theme = {
    // Dark military theme (Tarkov-inspired)
    Color{25, 28, 32, 255},        // background - very dark gray
    Color{35, 38, 42, 255},        // panelDark - dark gray
    Color{45, 48, 52, 255},        // panelLight - lighter gray
    Color{206, 147, 54, 255},      // accent - gold/brass
    Color{226, 167, 74, 255},      // accentHover - brighter gold
    Color{220, 220, 220, 255},     // text - light gray
    Color{160, 160, 160, 255},     // textDark - medium gray
    Color{220, 80, 80, 255},       // danger - red
    Color{80, 220, 120, 255}       // success - green
};

UIManager::UIManager(Game* game) : game(game) {
    // Configure raygui style
    GuiSetStyle(DEFAULT, TEXT_SIZE, 18);
    GuiSetStyle(DEFAULT, BORDER_WIDTH, 2);
}

UIManager::~UIManager() {}

void UIManager::update(float dt) {}

void UIManager::render() {
    // UIManager doesn't render directly - screens handle their own rendering
}

void UIManager::onStateChanged(GameState newState) {}

const UITheme& UIManager::getTheme() {
    return theme;
}

void UIManager::drawPanel(Rectangle bounds, const char* title) {
    // Draw panel background
    DrawRectangleRec(bounds, theme.panelDark);
    
    // Draw border
    DrawRectangleLinesEx(bounds, 2, theme.accent);
    
    // Draw title if provided
    if (title != nullptr) {
        int titleSize = 24;
        int titleWidth = MeasureText(title, titleSize);
        Vector2 titlePos = {
            bounds.x + (bounds.width - titleWidth) / 2,
            bounds.y + 15
        };
        DrawText(title, (int)titlePos.x, (int)titlePos.y, titleSize, theme.accent);
        
        // Draw separator line under title
        DrawLineEx(
            Vector2{bounds.x + 20, bounds.y + 50},
            Vector2{bounds.x + bounds.width - 20, bounds.y + 50},
            2, theme.panelLight
        );
    }
}

bool UIManager::drawButton(Rectangle bounds, const char* text) {
    return drawButtonEx(bounds, text, theme.panelLight, theme.accent);
}

bool UIManager::drawButtonEx(Rectangle bounds, const char* text, Color bgColor, Color hoverColor) {
    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, bounds);
    bool isClicked = isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    // Determine button color
    Color btnColor = isHovered ? hoverColor : bgColor;
    
    // Draw button background
    DrawRectangleRec(bounds, btnColor);
    
    // Draw border
    Color borderColor = isHovered ? theme.text : theme.textDark;
    DrawRectangleLinesEx(bounds, 2, borderColor);
    
    // Draw text centered
    int fontSize = 20;
    int textWidth = MeasureText(text, fontSize);
    Vector2 textPos = {
        bounds.x + (bounds.width - textWidth) / 2,
        bounds.y + (bounds.height - fontSize) / 2
    };
    
    Color textColor = isHovered ? theme.background : theme.text;
    DrawText(text, (int)textPos.x, (int)textPos.y, fontSize, textColor);
    
    return isClicked;
}

void UIManager::drawHealthBar(Rectangle bounds, float current, float max) {
    // Clamp values
    if (current < 0) current = 0;
    if (current > max) current = max;
    if (max <= 0) max = 1;
    
    float percent = current / max;
    
    // Draw background
    DrawRectangleRec(bounds, theme.panelDark);
    
    // Draw health fill
    Rectangle fillRect = bounds;
    fillRect.width *= percent;
    
    // Color based on health percentage
    Color fillColor;
    if (percent > 0.6f) {
        fillColor = theme.success;
    } else if (percent > 0.3f) {
        fillColor = theme.accent;
    } else {
        fillColor = theme.danger;
    }
    
    DrawRectangleRec(fillRect, fillColor);
    
    // Draw border
    DrawRectangleLinesEx(bounds, 2, theme.text);
    
    // Draw text
    char healthText[32];
    snprintf(healthText, sizeof(healthText), "%.0f / %.0f", current, max);
    int fontSize = 16;
    int textWidth = MeasureText(healthText, fontSize);
    Vector2 textPos = {
        bounds.x + (bounds.width - textWidth) / 2,
        bounds.y + (bounds.height - fontSize) / 2
    };
    DrawText(healthText, (int)textPos.x, (int)textPos.y, fontSize, theme.text);
}

void UIManager::drawProgressBar(Rectangle bounds, float progress, Color color) {
    // Clamp progress
    if (progress < 0) progress = 0;
    if (progress > 1) progress = 1;
    
    // Draw background
    DrawRectangleRec(bounds, theme.panelDark);
    
    // Draw progress fill
    Rectangle fillRect = bounds;
    fillRect.width *= progress;
    DrawRectangleRec(fillRect, color);
    
    // Draw border
    DrawRectangleLinesEx(bounds, 2, theme.text);
    
    // Draw percentage text
    char progressText[16];
    snprintf(progressText, sizeof(progressText), "%.0f%%", progress * 100);
    int fontSize = 16;
    int textWidth = MeasureText(progressText, fontSize);
    Vector2 textPos = {
        bounds.x + (bounds.width - textWidth) / 2,
        bounds.y + (bounds.height - fontSize) / 2
    };
    DrawText(progressText, (int)textPos.x, (int)textPos.y, fontSize, theme.text);
}

}
