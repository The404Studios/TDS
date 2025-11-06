#pragma once
#include "../../common/DataStructures.h"
#include <string>
#include <vector>

// UI State enum
enum class UIState {
    LOGIN,
    MAIN_MENU,
    LOBBY,
    STASH,
    MERCHANT,
    IN_GAME,
    LOADING
};

// Simple text rendering helper
class TextRenderer {
public:
    static void initFont(HDC hDC);
    static void drawText(const std::string& text, float x, float y, float size = 1.0f);
    static void drawTextCentered(const std::string& text, float y, float size = 1.0f);
};

// Base UI class
class BaseUI {
public:
    virtual ~BaseUI() {}
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void handleInput(char key) = 0;
    virtual void handleMouseClick(float x, float y) {}
    virtual void setMousePosition(float x, float y) { mouseX = x; mouseY = y; }
    virtual UIState getNextState();
    virtual bool shouldChangeState();
    virtual void resetStateChange();

protected:
    bool changeState = false;
    UIState nextState = UIState::LOGIN;
    float mouseX = 0.0f;
    float mouseY = 0.0f;

    void drawBox(float x, float y, float width, float height, bool filled = true);
    void drawButton(float x, float y, float width, float height, const std::string& text, bool highlighted = false);
    void drawInputField(float x, float y, float width, float height, const std::string& text, bool active = false);
    void drawPanel(float x, float y, float width, float height, const std::string& title);
    void drawList(float x, float y, float width, float height, const std::vector<std::string>& items, int selectedIndex = -1);

    // Helper function to check if a point is inside a rectangle
    bool isPointInRect(float px, float py, float x, float y, float w, float h) {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }
};

// UI Manager - handles UI state and rendering
class UIManager {
public:
    UIManager();
    void setState(UIState state, BaseUI* ui);
    UIState getCurrentState() const;
    void update(float deltaTime);
    void render();
    void handleInput(char key);
    void handleMouseClick(float x, float y);
    void setMousePosition(float x, float y);
    void setAspectRatio(float aspectRatio);
    bool shouldChangeState() const;
    UIState getNextState() const;
    void resetTransition();

private:
    UIState currentState;
    UIState nextState;
    BaseUI* currentUI;
    bool shouldTransition = false;
    float aspectRatio = 16.0f / 9.0f;
};
