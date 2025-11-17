#pragma once
#include "../Game.h"
#include <string>

namespace TDS {

class LoginUI {
public:
    LoginUI(Game* game);
    ~LoginUI();
    void update(float dt);
    void render();
    void onStateChanged(GameState newState);

private:
    void handleLogin();
    void handleRegister();

    Game* game;

    // Input fields
    char usernameInput[64];
    char passwordInput[64];
    bool usernameEditMode;
    bool passwordEditMode;

    // UI state
    bool showPassword;
    std::string statusMessage;
    float statusMessageTime;
    bool isRegistering;  // false = login mode, true = register mode
};

} // namespace TDS
