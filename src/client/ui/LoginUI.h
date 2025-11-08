#pragma once
#include "UIManager.h"
#include "../network/NetworkClient.h"
#include "../../common/NetworkProtocol.h"
#include "../../common/Utils.h"
#include <string>
#include <cstring>

// Login/Register UI
class LoginUI : public BaseUI {
public:
    LoginUI(NetworkClient* netClient)
        : networkClient(netClient), accountId(0), selectedField(0),
          mode(Mode::LOGIN), waitingForResponse(false), animTime(0.0f) {}

    void update(float deltaTime) override;
    void render() override;
    void handleInput(char key) override;
    void handleMouseClick(float x, float y) override;

    uint64_t getAccountId() const {
        return accountId;
    }

private:
    enum class Mode {
        LOGIN,
        REGISTER
    };

    NetworkClient* networkClient;
    uint64_t accountId;
    int selectedField;
    Mode mode;
    bool waitingForResponse;
    std::string username;
    std::string password;
    std::string email;
    std::string errorMessage;
    std::string statusMessage;
    float animTime;

    void submitAction();
    void toggleMode();
    void sendLoginRequest();
    void sendRegisterRequest();
    void handleLoginResponse(const std::vector<uint8_t>& payload);
    void handleRegisterResponse(const std::vector<uint8_t>& payload);
};
