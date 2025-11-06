#pragma once
#include "../Common/DataStructures.h"
#include <string>
#include <windows.h>
#include <gl/GL.h>

// UI State enum
enum class UIState {
    LOGIN,
    LOBBY,
    STASH,
    MERCHANT,
    IN_GAME,
    LOADING
};

// Simple text rendering helper
class TextRenderer {
public:
    static void drawText(const std::string& text, float x, float y, float size = 1.0f) {
        glRasterPos2f(x, y);
        // In production, use proper font rendering (FreeType, stb_truetype, etc.)
        // For now, this is a placeholder
    }

    static void drawTextCentered(const std::string& text, float y, float size = 1.0f) {
        float x = 0.0f - (text.length() * 0.01f * size);
        drawText(text, x, y, size);
    }
};

// Base UI class
class BaseUI {
public:
    virtual ~BaseUI() {}
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void handleInput(char key) = 0;
    virtual UIState getNextState() { return UIState::LOGIN; }
    virtual bool shouldChangeState() { return false; }
    virtual void resetStateChange() { changeState = false; }

protected:
    bool changeState = false;
    UIState nextState = UIState::LOGIN;

    void drawBox(float x, float y, float width, float height, bool filled = true) {
        if (filled) {
            glBegin(GL_QUADS);
        } else {
            glBegin(GL_LINE_LOOP);
        }
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
        glEnd();
    }

    void drawButton(float x, float y, float width, float height, const std::string& text, bool highlighted = false) {
        // Draw button background
        if (highlighted) {
            glColor3f(0.3f, 0.5f, 0.8f);  // Blue when highlighted
        } else {
            glColor3f(0.2f, 0.2f, 0.2f);  // Dark gray
        }
        drawBox(x, y, width, height, true);

        // Draw button border
        glColor3f(0.8f, 0.8f, 0.8f);  // Light gray
        drawBox(x, y, width, height, false);

        // Draw text
        glColor3f(1.0f, 1.0f, 1.0f);  // White
        float textX = x + width / 2.0f - (text.length() * 0.005f);
        float textY = y + height / 2.0f - 0.01f;
        TextRenderer::drawText(text, textX, textY, 1.0f);
    }

    void drawInputField(float x, float y, float width, float height, const std::string& text, bool active = false) {
        // Draw field background
        if (active) {
            glColor3f(0.15f, 0.15f, 0.25f);  // Slightly blue when active
        } else {
            glColor3f(0.1f, 0.1f, 0.1f);  // Very dark
        }
        drawBox(x, y, width, height, true);

        // Draw field border
        if (active) {
            glColor3f(0.5f, 0.7f, 1.0f);  // Blue border when active
        } else {
            glColor3f(0.5f, 0.5f, 0.5f);  // Gray border
        }
        drawBox(x, y, width, height, false);

        // Draw text
        glColor3f(1.0f, 1.0f, 1.0f);  // White
        float textX = x + 0.01f;
        float textY = y + height / 2.0f - 0.01f;
        std::string displayText = text;
        if (active) {
            displayText += "_";  // Cursor
        }
        TextRenderer::drawText(displayText, textX, textY, 1.0f);
    }

    void drawPanel(float x, float y, float width, float height, const std::string& title) {
        // Draw panel background
        glColor3f(0.15f, 0.15f, 0.15f);
        drawBox(x, y, width, height, true);

        // Draw panel border
        glColor3f(0.6f, 0.6f, 0.6f);
        drawBox(x, y, width, height, false);

        // Draw title bar
        glColor3f(0.25f, 0.25f, 0.35f);
        drawBox(x, y, width, 0.05f, true);

        // Draw title text
        glColor3f(1.0f, 1.0f, 1.0f);
        TextRenderer::drawText(title, x + 0.01f, y + 0.02f, 1.2f);
    }

    void drawList(float x, float y, float width, float height, const std::vector<std::string>& items, int selectedIndex = -1) {
        float itemHeight = 0.05f;
        float currentY = y;

        for (size_t i = 0; i < items.size(); i++) {
            if (currentY + itemHeight > y + height) break;

            // Draw item background
            if ((int)i == selectedIndex) {
                glColor3f(0.3f, 0.4f, 0.6f);  // Highlighted
            } else if (i % 2 == 0) {
                glColor3f(0.12f, 0.12f, 0.12f);  // Alternate row
            } else {
                glColor3f(0.15f, 0.15f, 0.15f);
            }
            drawBox(x, currentY, width, itemHeight, true);

            // Draw item text
            glColor3f(1.0f, 1.0f, 1.0f);
            TextRenderer::drawText(items[i], x + 0.01f, currentY + 0.02f, 1.0f);

            currentY += itemHeight;
        }
    }
};

// UI Manager - handles UI state and rendering
class UIManager {
public:
    UIManager() : currentState(UIState::LOGIN), currentUI(nullptr) {}

    void setState(UIState state, BaseUI* ui) {
        currentState = state;
        currentUI = ui;
    }

    UIState getCurrentState() const {
        return currentState;
    }

    void update(float deltaTime) {
        if (currentUI) {
            currentUI->update(deltaTime);

            // Check if UI wants to change state
            if (currentUI->shouldChangeState()) {
                nextState = currentUI->getNextState();
                shouldTransition = true;
                currentUI->resetStateChange();
            }
        }
    }

    void render() {
        if (currentUI) {
            currentUI->render();
        }
    }

    void handleInput(char key) {
        if (currentUI) {
            currentUI->handleInput(key);
        }
    }

    bool shouldChangeState() const {
        return shouldTransition;
    }

    UIState getNextState() const {
        return nextState;
    }

    void resetTransition() {
        shouldTransition = false;
    }

private:
    UIState currentState;
    UIState nextState;
    BaseUI* currentUI;
    bool shouldTransition = false;
};
