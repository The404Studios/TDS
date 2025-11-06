#include "LoginUI.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <gl/GL.h>
#include <iostream>
#include <cstring>

void LoginUI::update(float deltaTime) {
    // Check for server responses
    if (waitingForResponse && networkClient->hasPackets()) {
        auto packet = networkClient->getNextPacket();

        if (mode == Mode::LOGIN && packet.type == PacketType::LOGIN_RESPONSE) {
            handleLoginResponse(packet.payload);
        }
        else if (mode == Mode::REGISTER && packet.type == PacketType::REGISTER_RESPONSE) {
            handleRegisterResponse(packet.payload);
        }
    }
}

void LoginUI::render() {
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Draw title
    glColor3f(1.0f, 1.0f, 1.0f);
    std::string title = (mode == Mode::LOGIN) ? "EXTRACTION SHOOTER - LOGIN" : "EXTRACTION SHOOTER - REGISTER";
    TextRenderer::drawTextCentered(title, 0.8f, 1.5f);

    // Draw login panel
    drawPanel(-0.4f, -0.1f, 0.8f, 0.7f, (mode == Mode::LOGIN) ? "Login" : "Register");

    // Draw username field
    glColor3f(0.8f, 0.8f, 0.8f);
    TextRenderer::drawText("Username:", -0.35f, 0.4f);
    drawInputField(-0.35f, 0.3f, 0.7f, 0.08f, username, selectedField == 0);

    // Draw password field
    TextRenderer::drawText("Password:", -0.35f, 0.15f);
    std::string maskedPassword(password.length(), '*');
    drawInputField(-0.35f, 0.05f, 0.7f, 0.08f, maskedPassword, selectedField == 1);

    // Draw email field (register only)
    if (mode == Mode::REGISTER) {
        TextRenderer::drawText("Email:", -0.35f, -0.1f);
        drawInputField(-0.35f, -0.2f, 0.7f, 0.08f, email, selectedField == 2);
    }

    // Draw buttons
    float buttonY = (mode == Mode::LOGIN) ? -0.15f : -0.35f;
    drawButton(-0.2f, buttonY, 0.4f, 0.08f, (mode == Mode::LOGIN) ? "Login" : "Register", selectedField == 3);

    // Draw mode switch button
    std::string switchText = (mode == Mode::LOGIN) ? "Create Account" : "Back to Login";
    drawButton(-0.2f, buttonY - 0.12f, 0.4f, 0.08f, switchText, selectedField == 4);

    // Draw error message
    if (!errorMessage.empty()) {
        glColor3f(1.0f, 0.3f, 0.3f);  // Red
        TextRenderer::drawTextCentered(errorMessage, -0.55f, 1.0f);
    }

    // Draw status message
    if (!statusMessage.empty()) {
        glColor3f(0.3f, 1.0f, 0.3f);  // Green
        TextRenderer::drawTextCentered(statusMessage, -0.55f, 1.0f);
    }

    // Draw waiting indicator
    if (waitingForResponse) {
        glColor3f(1.0f, 1.0f, 0.3f);  // Yellow
        TextRenderer::drawTextCentered("Connecting to server...", -0.65f, 1.0f);
    }

    // Draw connection status
    if (!networkClient->isConnected()) {
        glColor3f(1.0f, 0.3f, 0.3f);  // Red
        TextRenderer::drawTextCentered("Not connected to server", -0.75f, 0.8f);
    }

    // Draw controls
    glColor3f(0.6f, 0.6f, 0.6f);
    TextRenderer::drawText("Controls: TAB - Next Field | ENTER - Submit | ESC - Switch Mode", -0.9f, -0.9f, 0.8f);
}

void LoginUI::handleInput(char key) {
    if (waitingForResponse) return;  // Ignore input while waiting

    // Tab - cycle through fields
    if (key == '\t' || key == 9) {
        int maxFields = (mode == Mode::LOGIN) ? 5 : 6;
        selectedField = (selectedField + 1) % maxFields;
        return;
    }

    // Enter - submit
    if (key == '\r' || key == '\n' || key == 13) {
        if (selectedField == 3) {
            submitAction();
        } else if (selectedField == 4) {
            toggleMode();
        }
        return;
    }

    // Escape - toggle mode
    if (key == 27) {
        toggleMode();
        return;
    }

    // Backspace
    if (key == '\b' || key == 8) {
        if (selectedField == 0 && !username.empty()) {
            username.pop_back();
        } else if (selectedField == 1 && !password.empty()) {
            password.pop_back();
        } else if (selectedField == 2 && !email.empty()) {
            email.pop_back();
        }
        return;
    }

    // Text input
    if (key >= 32 && key <= 126) {
        if (selectedField == 0 && username.length() < 16) {
            username += key;
        } else if (selectedField == 1 && password.length() < 32) {
            password += key;
        } else if (selectedField == 2 && email.length() < 64) {
            email += key;
        }
    }
}

void LoginUI::handleMouseClick(float x, float y) {
    if (waitingForResponse) return;  // Ignore input while waiting

    // Check username field (-0.35f, 0.3f, 0.7f, 0.08f)
    if (isPointInRect(x, y, -0.35f, 0.3f, 0.7f, 0.08f)) {
        selectedField = 0;
        return;
    }

    // Check password field (-0.35f, 0.05f, 0.7f, 0.08f)
    if (isPointInRect(x, y, -0.35f, 0.05f, 0.7f, 0.08f)) {
        selectedField = 1;
        return;
    }

    // Check email field (register mode only) (-0.35f, -0.2f, 0.7f, 0.08f)
    if (mode == Mode::REGISTER) {
        if (isPointInRect(x, y, -0.35f, -0.2f, 0.7f, 0.08f)) {
            selectedField = 2;
            return;
        }
    }

    // Calculate button Y position
    float buttonY = (mode == Mode::LOGIN) ? -0.15f : -0.35f;

    // Check Login/Register button (-0.2f, buttonY, 0.4f, 0.08f)
    if (isPointInRect(x, y, -0.2f, buttonY, 0.4f, 0.08f)) {
        submitAction();
        return;
    }

    // Check switch mode button (-0.2f, buttonY - 0.12f, 0.4f, 0.08f)
    if (isPointInRect(x, y, -0.2f, buttonY - 0.12f, 0.4f, 0.08f)) {
        toggleMode();
        return;
    }
}

void LoginUI::submitAction() {
    errorMessage.clear();
    statusMessage.clear();

    // Validate input
    if (username.empty() || password.empty()) {
        errorMessage = "Username and password required";
        return;
    }

    if (mode == Mode::REGISTER && email.empty()) {
        errorMessage = "Email required for registration";
        return;
    }

    if (!networkClient->isConnected()) {
        errorMessage = "Not connected to server";
        return;
    }

    // Simple password hash (in production, use proper hashing)
    std::string passwordHash = simpleHash(password);

    if (mode == Mode::LOGIN) {
        sendLoginRequest(passwordHash);
    } else {
        sendRegisterRequest(passwordHash);
    }

    waitingForResponse = true;
}

void LoginUI::sendLoginRequest(const std::string& passwordHash) {
    LoginRequest req;
    strncpy_s(req.username, username.c_str(), sizeof(req.username) - 1);
    strncpy_s(req.passwordHash, passwordHash.c_str(), sizeof(req.passwordHash) - 1);

    networkClient->sendPacket(PacketType::LOGIN_REQUEST, &req, sizeof(req));
}

void LoginUI::sendRegisterRequest(const std::string& passwordHash) {
    RegisterRequest req;
    strncpy_s(req.username, username.c_str(), sizeof(req.username) - 1);
    strncpy_s(req.passwordHash, passwordHash.c_str(), sizeof(req.passwordHash) - 1);
    strncpy_s(req.email, email.c_str(), sizeof(req.email) - 1);

    networkClient->sendPacket(PacketType::REGISTER_REQUEST, &req, sizeof(req));
}

void LoginUI::handleLoginResponse(const std::vector<uint8_t>& payload) {
    waitingForResponse = false;

    if (payload.size() < sizeof(LoginResponse)) {
        errorMessage = "Invalid server response";
        return;
    }

    LoginResponse resp;
    memcpy(&resp, payload.data(), sizeof(LoginResponse));

    if (resp.success) {
        accountId = resp.accountId;
        networkClient->setSessionToken(resp.sessionToken);

        statusMessage = "Login successful!";
        std::cout << "[LoginUI] Login successful! AccountID: " << accountId << std::endl;

        // Transition to main menu
        nextState = UIState::MAIN_MENU;
        changeState = true;
    } else {
        errorMessage = std::string(resp.errorMessage);
    }
}

void LoginUI::handleRegisterResponse(const std::vector<uint8_t>& payload) {
    waitingForResponse = false;

    if (payload.size() < sizeof(RegisterResponse)) {
        errorMessage = "Invalid server response";
        return;
    }

    RegisterResponse resp;
    memcpy(&resp, payload.data(), sizeof(RegisterResponse));

    if (resp.success) {
        accountId = resp.accountId;
        statusMessage = "Registration successful! You can now login.";
        mode = Mode::LOGIN;
        password.clear();
        email.clear();
    } else {
        errorMessage = std::string(resp.errorMessage);
    }
}

void LoginUI::toggleMode() {
    mode = (mode == Mode::LOGIN) ? Mode::REGISTER : Mode::LOGIN;
    selectedField = 0;
    errorMessage.clear();
    statusMessage.clear();
}
