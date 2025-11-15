#pragma once
#include "../Game.h"
#include "raylib.h"

namespace TDS {

// UI Theme colors
struct UITheme {
    Color background;
    Color panelDark;
    Color panelLight;
    Color accent;
    Color accentHover;
    Color text;
    Color textDark;
    Color danger;
    Color success;
};

class UIManager {
public:
    UIManager(Game* game);
    ~UIManager();
    void update(float dt);
    void render();
    void onStateChanged(GameState newState);

    // Static theme and utility methods
    static const UITheme& getTheme();
    static void drawPanel(Rectangle bounds, const char* title);
    static bool drawButton(Rectangle bounds, const char* text);
    static bool drawButtonEx(Rectangle bounds, const char* text, Color bgColor, Color hoverColor);
    static void drawHealthBar(Rectangle bounds, float current, float max);
    static void drawProgressBar(Rectangle bounds, float progress, Color color);

private:
    Game* game;
    static UITheme theme;
};
}
