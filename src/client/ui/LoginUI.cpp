#include "LoginUI.h"
#include "../../engine/core/Platform.h"
#include <iostream>
#include <cstring>
#include <cmath>

void LoginUI::update(float deltaTime) {
    // Update animation
    animTime += deltaTime;

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
    // Clear screen with dark background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
    glLoadIdentity();

    // Animated title with pulse effect
    float pulse = 0.85f + 0.15f * std::sin(animTime * 2.0f);
    glColor3f(pulse * 1.0f, pulse * 0.8f, pulse * 0.4f);
    TextRenderer::drawTextCentered("EXTRACTION SHOOTER", 0.75f, 2.0f);

    // Subtitle
    glColor3f(0.7f, 0.7f, 0.7f);
    std::string subtitle = (mode == Mode::LOGIN) ? "LOGIN" : "REGISTER";
    TextRenderer::drawTextCentered(subtitle, 0.6f, 1.3f);

    // Main panel (centered, with proper aspect ratio)
    float panelW = 0.65f;
    float panelH = (mode == Mode::LOGIN) ? 0.8f : 1.0f;
    float panelX = -panelW / 2.0f;
    float panelY = -0.35f;

    // Draw main panel with shadow effect
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Shadow
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    drawBox(panelX + 0.02f, panelY - 0.02f, panelW, panelH, true);

    // Panel background
    glColor4f(0.12f, 0.12f, 0.16f, 0.95f);
    drawBox(panelX, panelY, panelW, panelH, true);

    // Panel border
    glColor3f(0.4f, 0.35f, 0.25f);
    glLineWidth(2.0f);
    drawBox(panelX, panelY, panelW, panelH, false);
    glLineWidth(1.0f);
    glDisable(GL_BLEND);

    // Input fields with proper spacing
    float fieldW = 0.55f;
    float fieldH = 0.08f;
    float fieldX = panelX + (panelW - fieldW) / 2.0f;
    float currentY = panelY + panelH - 0.15f;

    // Username field
    glColor3f(0.9f, 0.9f, 0.9f);
    TextRenderer::drawText("Username", fieldX, currentY, 0.85f);
    currentY -= 0.05f;
    drawInputField(fieldX, currentY, fieldW, fieldH, username, selectedField == 0);
    currentY -= fieldH + 0.08f;

    // Password field
    glColor3f(0.9f, 0.9f, 0.9f);
    TextRenderer::drawText("Password", fieldX, currentY, 0.85f);
    currentY -= 0.05f;
    std::string maskedPassword(password.length(), '*');
    drawInputField(fieldX, currentY, fieldW, fieldH, maskedPassword, selectedField == 1);
    currentY -= fieldH + 0.08f;

    // Email field (register only)
    if (mode == Mode::REGISTER) {
        glColor3f(0.9f, 0.9f, 0.9f);
        TextRenderer::drawText("Email", fieldX, currentY, 0.85f);
        currentY -= 0.05f;
        drawInputField(fieldX, currentY, fieldW, fieldH, email, selectedField == 2);
        currentY -= fieldH + 0.08f;
    }

    // Buttons (properly spaced and centered)
    float btnW = 0.5f;
    float btnH = 0.1f;
    float btnX = panelX + (panelW - btnW) / 2.0f;
    currentY -= 0.05f;

    // Primary button (Login/Register)
    bool btn1Hover = isPointInRect(mouseX, mouseY, btnX, currentY, btnW, btnH);
    if (btn1Hover) {
        glColor3f(0.35f, 0.55f, 0.85f);
    } else if (selectedField == 3) {
        glColor3f(0.25f, 0.45f, 0.75f);
    } else {
        glColor3f(0.2f, 0.35f, 0.6f);
    }
    drawBox(btnX, currentY, btnW, btnH, true);

    // Button border with glow on hover
    if (btn1Hover) {
        float glowPulse = 0.7f + 0.3f * std::sin(animTime * 8.0f);
        glColor3f(glowPulse, glowPulse * 0.8f, 1.0f);
        glLineWidth(3.0f);
    } else {
        glColor3f(0.5f, 0.5f, 0.6f);
        glLineWidth(2.0f);
    }
    drawBox(btnX, currentY, btnW, btnH, false);
    glLineWidth(1.0f);

    // Button text (properly centered)
    std::string btn1Text = (mode == Mode::LOGIN) ? "LOGIN" : "REGISTER";
    glColor3f(1.0f, 1.0f, 1.0f);
    float textX1 = btnX + btnW / 2.0f - btn1Text.length() * 0.009f;
    float textY1 = currentY + btnH / 2.0f - 0.02f;
    TextRenderer::drawText(btn1Text, textX1, textY1, 1.1f);

    currentY -= btnH + 0.04f;

    // Secondary button (Switch mode)
    bool btn2Hover = isPointInRect(mouseX, mouseY, btnX, currentY, btnW, btnH);
    if (btn2Hover) {
        glColor3f(0.3f, 0.3f, 0.35f);
    } else if (selectedField == 4) {
        glColor3f(0.25f, 0.25f, 0.3f);
    } else {
        glColor3f(0.18f, 0.18f, 0.22f);
    }
    drawBox(btnX, currentY, btnW, btnH, true);

    glColor3f(0.5f, 0.5f, 0.6f);
    glLineWidth(2.0f);
    drawBox(btnX, currentY, btnW, btnH, false);
    glLineWidth(1.0f);

    // Button text
    std::string btn2Text = (mode == Mode::LOGIN) ? "CREATE ACCOUNT" : "BACK TO LOGIN";
    glColor3f(0.9f, 0.9f, 0.9f);
    float textX2 = btnX + btnW / 2.0f - btn2Text.length() * 0.0075f;
    float textY2 = currentY + btnH / 2.0f - 0.02f;
    TextRenderer::drawText(btn2Text, textX2, textY2, 1.0f);

    // Status messages at bottom
    if (!errorMessage.empty()) {
        glColor3f(1.0f, 0.4f, 0.4f);
        TextRenderer::drawTextCentered(errorMessage, -0.75f, 1.2f);
    }

    if (!statusMessage.empty()) {
        glColor3f(0.4f, 1.0f, 0.4f);
        TextRenderer::drawTextCentered(statusMessage, -0.75f, 1.2f);
    }

    if (waitingForResponse) {
        float dots = std::fmod(animTime * 2.0f, 1.0f);
        std::string waitText = "Connecting";
        if (dots > 0.66f) waitText += "...";
        else if (dots > 0.33f) waitText += "..";
        else waitText += ".";

        glColor3f(1.0f, 1.0f, 0.5f);
        TextRenderer::drawTextCentered(waitText, -0.8f, 1.2f);
    }

    // Connection status
    if (!networkClient->isConnected()) {
        glColor3f(1.0f, 0.3f, 0.3f);
        TextRenderer::drawText("Disconnected", 0.65f, 0.9f, 0.8f);
    } else {
        glColor3f(0.3f, 1.0f, 0.3f);
        TextRenderer::drawText("Connected", 0.65f, 0.9f, 0.8f);
    }

    // Controls hint
    glColor3f(0.5f, 0.5f, 0.5f);
    TextRenderer::drawText("TAB: Next Field  |  ENTER: Submit  |  ESC: Switch Mode", -0.88f, -0.92f, 0.7f);
}

void LoginUI::handleInput(char key) {
    if (waitingForResponse) return;

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
    if (waitingForResponse) return;

    float panelW = 0.65f;
    float panelH = (mode == Mode::LOGIN) ? 0.8f : 1.0f;
    float panelX = -panelW / 2.0f;
    float panelY = -0.35f;

    float fieldW = 0.55f;
    float fieldH = 0.08f;
    float fieldX = panelX + (panelW - fieldW) / 2.0f;
    float currentY = panelY + panelH - 0.2f;

    // Check username field
    if (isPointInRect(x, y, fieldX, currentY, fieldW, fieldH)) {
        selectedField = 0;
        return;
    }
    currentY -= fieldH + 0.13f;

    // Check password field
    if (isPointInRect(x, y, fieldX, currentY, fieldW, fieldH)) {
        selectedField = 1;
        return;
    }
    currentY -= fieldH + 0.13f;

    // Check email field (if in register mode)
    if (mode == Mode::REGISTER) {
        if (isPointInRect(x, y, fieldX, currentY, fieldW, fieldH)) {
            selectedField = 2;
            return;
        }
        currentY -= fieldH + 0.13f;
    }

    // Check buttons
    float btnW = 0.5f;
    float btnH = 0.1f;
    float btnX = panelX + (panelW - btnW) / 2.0f;

    // Primary button
    if (isPointInRect(x, y, btnX, currentY, btnW, btnH)) {
        selectedField = 3;
        submitAction();
        return;
    }
    currentY -= btnH + 0.04f;

    // Secondary button
    if (isPointInRect(x, y, btnX, currentY, btnW, btnH)) {
        selectedField = 4;
        toggleMode();
        return;
    }
}

void LoginUI::submitAction() {
    errorMessage.clear();
    statusMessage.clear();

    if (mode == Mode::LOGIN) {
        if (username.empty() || password.empty()) {
            errorMessage = "Please fill in all fields";
            return;
        }
        sendLoginRequest();
    } else {
        if (username.empty() || password.empty() || email.empty()) {
            errorMessage = "Please fill in all fields";
            return;
        }
        if (email.find('@') == std::string::npos) {
            errorMessage = "Invalid email address";
            return;
        }
        sendRegisterRequest();
    }
}

void LoginUI::toggleMode() {
    mode = (mode == Mode::LOGIN) ? Mode::REGISTER : Mode::LOGIN;
    selectedField = 0;
    errorMessage.clear();
    statusMessage.clear();
}

void LoginUI::sendLoginRequest() {
    std::cout << "[LoginUI] Attempting login: " << username << std::endl;

    LoginRequest req;
    std::string passwordHash = simpleHash(password);

    strncpy_s(req.username, username.c_str(), sizeof(req.username) - 1);
    strncpy_s(req.passwordHash, passwordHash.c_str(), sizeof(req.passwordHash) - 1);

    networkClient->sendPacket(PacketType::LOGIN_REQUEST, &req, sizeof(req));
    waitingForResponse = true;
}

void LoginUI::sendRegisterRequest() {
    std::cout << "[LoginUI] Attempting registration: " << username << std::endl;

    RegisterRequest req;
    std::string passwordHash = simpleHash(password);

    strncpy_s(req.username, username.c_str(), sizeof(req.username) - 1);
    strncpy_s(req.passwordHash, passwordHash.c_str(), sizeof(req.passwordHash) - 1);
    strncpy_s(req.email, email.c_str(), sizeof(req.email) - 1);

    networkClient->sendPacket(PacketType::REGISTER_REQUEST, &req, sizeof(req));
    waitingForResponse = true;
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
        statusMessage = "Registration successful! You can now login.";
        std::cout << "[LoginUI] Registration successful!" << std::endl;

        // Switch to login mode
        mode = Mode::LOGIN;
        password.clear();
        email.clear();
        selectedField = 0;
    } else {
        errorMessage = std::string(resp.errorMessage);
    }
}
