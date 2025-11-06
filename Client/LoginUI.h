#pragma once
#include "UIManager.h"
#include "NetworkClient.h"
#include "../Common/NetworkProtocol.h"
#include "../Common/Utils.h"
#include <string>
#include <cstring>

// Login/Register UI
class LoginUI : public BaseUI {
public:
    LoginUI(NetworkClient* netClient) : networkClient(netClient), mode(Mode::LOGIN),
                                         selectedField(0), waitingForResponse(false),
                                         accountId(0) {}

    void update(float deltaTime) override;
    void render() override;
    void handleInput(char key) override;

    uint64_t getAccountId() const {
        return accountId;
    }

private:
    enum class Mode {
        LOGIN,
        REGISTER
    };

    NetworkClient* networkClient;
    Mode mode;
    int selectedField;
    std::string username;
    std::string password;
    std::string email;
    std::string errorMessage;
    std::string statusMessage;
    bool waitingForResponse;
    uint64_t accountId;

    void submitAction();
    void sendLoginRequest(const std::string& passwordHash);
    void sendRegisterRequest(const std::string& passwordHash);
    void handleLoginResponse(const std::vector<uint8_t>& payload);
    void handleRegisterResponse(const std::vector<uint8_t>& payload);
    void toggleMode();
};
