#include "LoginUI.h"
#include "UIManager.h"
#include "raylib.h"
#include <cstring>

namespace TDS {

LoginUI::LoginUI(Game* game)
    : game(game)
    , usernameEditMode(false)
    , passwordEditMode(false)
    , showPassword(false)
    , statusMessageTime(0.0f)
    , isRegistering(false)
{
    // Initialize input buffers
    memset(usernameInput, 0, sizeof(usernameInput));
    memset(passwordInput, 0, sizeof(passwordInput));
    strcpy(usernameInput, "player1");  // Default username for testing
}

LoginUI::~LoginUI() {}

void LoginUI::update(float dt) {
    // Update status message timer
    if (statusMessageTime > 0.0f) {
        statusMessageTime -= dt;
        if (statusMessageTime <= 0.0f) {
            statusMessage.clear();
        }
    }

    // Handle ESC to toggle between login/register
    if (IsKeyPressed(KEY_ESCAPE)) {
        isRegistering = !isRegistering;
        statusMessage = isRegistering ? "Register new account" : "Login to existing account";
        statusMessageTime = 2.0f;
    }

    // Handle Enter to submit
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
        if (isRegistering) {
            handleRegister();
        } else {
            handleLogin();
        }
    }
}

void LoginUI::render() {
    auto theme = UIManager::getTheme();
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    // Background
    ClearBackground(theme.background);

    // Title
    const char* title = isRegistering ? "TDS - Register" : "TDS - Login";
    int titleSize = 48;
    int titleWidth = MeasureText(title, titleSize);
    DrawText(title, (screenWidth - titleWidth) / 2, 100, titleSize, theme.accent);

    // Subtitle
    const char* subtitle = "Tarkov Looter Shooter";
    int subtitleSize = 20;
    int subtitleWidth = MeasureText(subtitle, subtitleSize);
    DrawText(subtitle, (screenWidth - subtitleWidth) / 2, 160, subtitleSize, theme.textDark);

    // Login panel
    Rectangle panel = {
        (float)(screenWidth - 400) / 2,
        220,
        400,
        isRegistering ? 380.0f : 320.0f
    };
    UIManager::drawPanel(panel, nullptr);

    // Username field
    DrawText("USERNAME", panel.x + 40, panel.y + 30, 16, theme.text);
    Rectangle usernameBox = {panel.x + 40, panel.y + 55, 320, 35};

    // Draw textbox background
    DrawRectangleRec(usernameBox, theme.panelLight);
    DrawRectangleLinesEx(usernameBox, 2, usernameEditMode ? theme.accent : theme.textDark);

    // Handle username input
    if (CheckCollisionPointRec(GetMousePosition(), usernameBox)) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            usernameEditMode = true;
            passwordEditMode = false;
        }
    }

    if (usernameEditMode) {
        // Simple text input handling
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (strlen(usernameInput) < 63)) {
                int len = strlen(usernameInput);
                usernameInput[len] = (char)key;
                usernameInput[len + 1] = '\0';
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = strlen(usernameInput);
            if (len > 0) usernameInput[len - 1] = '\0';
        }
    }

    DrawText(usernameInput, usernameBox.x + 10, usernameBox.y + 9, 20, theme.text);
    if (usernameEditMode) {
        int textWidth = MeasureText(usernameInput, 20);
        DrawText("_", usernameBox.x + 10 + textWidth, usernameBox.y + 9, 20, theme.accent);
    }

    // Password field
    DrawText("PASSWORD", panel.x + 40, panel.y + 110, 16, theme.text);
    Rectangle passwordBox = {panel.x + 40, panel.y + 135, 320, 35};

    DrawRectangleRec(passwordBox, theme.panelLight);
    DrawRectangleLinesEx(passwordBox, 2, passwordEditMode ? theme.accent : theme.textDark);

    if (CheckCollisionPointRec(GetMousePosition(), passwordBox)) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            passwordEditMode = true;
            usernameEditMode = false;
        }
    }

    if (passwordEditMode) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (strlen(passwordInput) < 63)) {
                int len = strlen(passwordInput);
                passwordInput[len] = (char)key;
                passwordInput[len + 1] = '\0';
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = strlen(passwordInput);
            if (len > 0) passwordInput[len - 1] = '\0';
        }
    }

    // Display password as asterisks or plain text
    if (showPassword) {
        DrawText(passwordInput, passwordBox.x + 10, passwordBox.y + 9, 20, theme.text);
    } else {
        char masked[64];
        int len = strlen(passwordInput);
        for (int i = 0; i < len && i < 63; i++) {
            masked[i] = '*';
        }
        masked[len] = '\0';
        DrawText(masked, passwordBox.x + 10, passwordBox.y + 9, 20, theme.text);
    }

    if (passwordEditMode) {
        int textWidth = MeasureText(showPassword ? passwordInput : "****", 20);
        DrawText("_", passwordBox.x + 10 + textWidth, passwordBox.y + 9, 20, theme.accent);
    }

    // Show/Hide password toggle
    Rectangle toggleBox = {panel.x + 40, panel.y + 180, 20, 20};
    DrawRectangleRec(toggleBox, theme.panelLight);
    DrawRectangleLinesEx(toggleBox, 2, theme.textDark);
    if (showPassword) {
        DrawRectangle(toggleBox.x + 4, toggleBox.y + 4, 12, 12, theme.accent);
    }
    DrawText("Show Password", panel.x + 70, panel.y + 182, 16, theme.textDark);

    if (CheckCollisionPointRec(GetMousePosition(), toggleBox)) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            showPassword = !showPassword;
        }
    }

    // Submit button
    Rectangle submitBtn = {panel.x + 40, panel.y + 225, 320, 45};
    const char* submitText = isRegistering ? "REGISTER" : "LOGIN";
    if (UIManager::drawButtonEx(submitBtn, submitText, theme.accent, theme.accentHover)) {
        if (isRegistering) {
            handleRegister();
        } else {
            handleLogin();
        }
    }

    // Toggle mode button
    Rectangle toggleBtn = {panel.x + 40, panel.y + 285, 320, 35};
    const char* toggleText = isRegistering ? "Already have an account? Login" : "Don't have an account? Register";
    if (UIManager::drawButton(toggleBtn, toggleText)) {
        isRegistering = !isRegistering;
    }

    // Status message
    if (!statusMessage.empty()) {
        Color msgColor = (statusMessage.find("Error") != std::string::npos ||
                         statusMessage.find("Failed") != std::string::npos)
                         ? theme.danger : theme.success;

        int msgWidth = MeasureText(statusMessage.c_str(), 18);
        DrawText(statusMessage.c_str(), (screenWidth - msgWidth) / 2,
                screenHeight - 100, 18, msgColor);
    }

    // Instructions
    const char* instructions = "Press ESC to toggle Login/Register | Press ENTER to submit";
    int instWidth = MeasureText(instructions, 14);
    DrawText(instructions, (screenWidth - instWidth) / 2, screenHeight - 50, 14, theme.textDark);
}

void LoginUI::handleLogin() {
    if (strlen(usernameInput) == 0) {
        statusMessage = "Error: Username cannot be empty";
        statusMessageTime = 3.0f;
        return;
    }

    if (strlen(passwordInput) == 0) {
        statusMessage = "Error: Password cannot be empty";
        statusMessageTime = 3.0f;
        return;
    }

    statusMessage = "Logging in...";
    statusMessageTime = 2.0f;

    // Send login request to server through network client
    // This will be handled by NetworkClient
    // For now, just transition to main menu (will implement proper auth later)

    TraceLog(LOG_INFO, "Login attempt: username=%s", usernameInput);

    // TODO: Send actual login packet to server
    // game->getNetwork()->sendLogin(usernameInput, passwordInput);
}

void LoginUI::handleRegister() {
    if (strlen(usernameInput) < 3) {
        statusMessage = "Error: Username must be at least 3 characters";
        statusMessageTime = 3.0f;
        return;
    }

    if (strlen(passwordInput) < 6) {
        statusMessage = "Error: Password must be at least 6 characters";
        statusMessageTime = 3.0f;
        return;
    }

    statusMessage = "Registering...";
    statusMessageTime = 2.0f;

    TraceLog(LOG_INFO, "Register attempt: username=%s", usernameInput);

    // TODO: Send actual register packet to server
    // game->getNetwork()->sendRegister(usernameInput, passwordInput);
}

void LoginUI::onStateChanged(GameState newState) {}

} // namespace TDS
