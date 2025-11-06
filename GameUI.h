#ifndef GAME_UI_H
#define GAME_UI_H

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include "PlayerCharacter.h"

// UI Colors
struct UIColor {
    float r, g, b, a;
    UIColor(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f)
        : r(red), g(green), b(blue), a(alpha) {}
};

// Menu button
struct MenuButton {
    std::string text;
    float x, y, width, height;
    std::function<void()> onClick;
    bool hovered;
    UIColor normalColor;
    UIColor hoverColor;

    MenuButton(const std::string& txt, float posX, float posY, float w, float h, std::function<void()> callback)
        : text(txt), x(posX), y(posY), width(w), height(h), onClick(callback), hovered(false),
          normalColor(0.3f, 0.3f, 0.3f, 0.8f), hoverColor(0.5f, 0.5f, 0.5f, 0.9f) {}

    bool isPointInside(float px, float py) const {
        return px >= x && px <= x + width && py >= y && py <= y + height;
    }
};

// Game state
enum class GameState {
    MAIN_MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

// Main Menu System
class MainMenu {
private:
    std::vector<MenuButton> buttons;
    GLuint fontBase;
    int windowWidth;
    int windowHeight;
    GameState* gameState;

public:
    MainMenu(int screenWidth, int screenHeight, GameState* state);

    void initialize();
    void render();
    void handleClick(int x, int y);
    void handleMouseMove(int x, int y);

    void drawButton(const MenuButton& button);
    void drawText(float x, float y, const std::string& text, const UIColor& color);
};

// In-Game HUD
class GameHUD {
private:
    GLuint fontBase;
    int windowWidth;
    int windowHeight;
    PlayerCharacter* player;

    // Crosshair
    float crosshairSize;
    UIColor crosshairColor;

public:
    GameHUD(int screenWidth, int screenHeight, PlayerCharacter* playerRef);

    void render();
    void renderCrosshair();
    void renderHealthBar();
    void renderStaminaBar();
    void renderAmmoCounter();
    void renderWeaponInfo();
    void renderMinimap();

    void drawText(float x, float y, const std::string& text, const UIColor& color, float scale = 1.0f);
    void drawBar(float x, float y, float width, float height, float fillPercent, const UIColor& color, const UIColor& bgColor);

    void setWindowSize(int width, int height) {
        windowWidth = width;
        windowHeight = height;
    }
};

// Pause Menu
class PauseMenu {
private:
    std::vector<MenuButton> buttons;
    GLuint fontBase;
    int windowWidth;
    int windowHeight;
    GameState* gameState;

public:
    PauseMenu(int screenWidth, int screenHeight, GameState* state);

    void initialize();
    void render();
    void handleClick(int x, int y);
    void handleMouseMove(int x, int y);
};

// Game Over Screen
class GameOverScreen {
private:
    GLuint fontBase;
    int windowWidth;
    int windowHeight;
    std::string message;
    float displayTime;

public:
    GameOverScreen(int screenWidth, int screenHeight);

    void show(const std::string& msg);
    void render();
    void update(float deltaTime);

    bool shouldRestart() const { return displayTime > 3.0f; }
};

#endif // GAME_UI_H
