#pragma once
#include "UIManager.h"
#include "NetworkClient.h"
#include "../Common/NetworkProtocol.h"
#include "../Common/DataStructures.h"
#include <vector>
#include <cstring>

// Lobby UI
class LobbyUI : public BaseUI {
public:
    LobbyUI(NetworkClient* netClient, uint64_t accId)
        : networkClient(netClient), accountId(accId), inLobby(false),
          isReady(false), isOwner(false), inQueue(false), selectedAction(0) {}

    void update(float deltaTime) override {
        // Process server packets
        while (networkClient->hasPackets()) {
            auto packet = networkClient->getNextPacket();

            switch (packet.type) {
                case PacketType::LOBBY_CREATE_RESPONSE:
                    handleLobbyCreateResponse(packet.payload);
                    break;

                case PacketType::LOBBY_JOIN_RESPONSE:
                    handleLobbyJoinResponse(packet.payload);
                    break;

                case PacketType::LOBBY_UPDATE:
                    handleLobbyUpdate(packet.payload);
                    break;

                case PacketType::MATCH_FOUND:
                    handleMatchFound(packet.payload);
                    break;

                default:
                    break;
            }
        }
    }

    void render() override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // Draw title
        glColor3f(1.0f, 1.0f, 1.0f);
        TextRenderer::drawTextCentered("EXTRACTION SHOOTER - LOBBY", 0.9f, 1.5f);

        if (!inLobby) {
            renderLobbySelection();
        } else {
            renderLobbyView();
        }

        // Draw controls
        glColor3f(0.6f, 0.6f, 0.6f);
        std::string controls = inLobby ?
            "R - Ready | L - Leave | S - Start Queue (Owner)" :
            "C - Create Lobby | M - Main Menu";
        TextRenderer::drawText(controls, -0.9f, -0.9f, 0.8f);
    }

    void handleInput(char key) override {
        if (!inLobby) {
            // Lobby selection screen
            if (key == 'c' || key == 'C') {
                createLobby();
            } else if (key == 'm' || key == 'M') {
                // Go to main menu (stash/merchant)
                nextState = UIState::STASH;
                changeState = true;
            }
        } else {
            // In lobby screen
            if (key == 'r' || key == 'R') {
                toggleReady();
            } else if (key == 'l' || key == 'L') {
                leaveLobby();
            } else if ((key == 's' || key == 'S') && isOwner) {
                startQueue();
            }
        }
    }

private:
    NetworkClient* networkClient;
    uint64_t accountId;
    uint64_t currentLobbyId;
    bool inLobby;
    bool isReady;
    bool isOwner;
    bool inQueue;
    int selectedAction;
    std::vector<LobbyMember> lobbyMembers;
    std::string statusMessage;

    void renderLobbySelection() {
        // Draw main menu panel
        drawPanel(-0.5f, 0.2f, 1.0f, 0.5f, "Lobby Selection");

        // Draw options
        glColor3f(1.0f, 1.0f, 1.0f);
        TextRenderer::drawText("Press 'C' to Create a Lobby", -0.4f, 0.5f, 1.2f);
        TextRenderer::drawText("Press 'M' for Main Menu (Stash/Merchant)", -0.4f, 0.35f, 1.2f);

        // Draw status message
        if (!statusMessage.empty()) {
            glColor3f(0.3f, 1.0f, 0.3f);
            TextRenderer::drawTextCentered(statusMessage, 0.0f, 1.0f);
        }
    }

    void renderLobbyView() {
        // Draw lobby panel
        drawPanel(-0.6f, 0.1f, 1.2f, 0.7f, "Lobby");

        // Draw members list
        glColor3f(1.0f, 1.0f, 1.0f);
        TextRenderer::drawText("Players:", -0.55f, 0.6f, 1.2f);

        float yPos = 0.45f;
        for (const auto& member : lobbyMembers) {
            std::string memberText = std::string(member.username);

            if (member.isOwner) {
                memberText += " [OWNER]";
            }

            if (member.isReady) {
                glColor3f(0.3f, 1.0f, 0.3f);  // Green for ready
                memberText += " [READY]";
            } else {
                glColor3f(1.0f, 0.3f, 0.3f);  // Red for not ready
                memberText += " [NOT READY]";
            }

            TextRenderer::drawText(memberText, -0.5f, yPos, 1.0f);
            yPos -= 0.08f;
        }

        // Draw queue status
        if (inQueue) {
            glColor3f(1.0f, 1.0f, 0.3f);  // Yellow
            TextRenderer::drawTextCentered("IN QUEUE - Searching for match...", -0.3f, 1.2f);
        }

        // Draw ready status
        glColor3f(1.0f, 1.0f, 1.0f);
        std::string readyText = isReady ? "You are READY" : "You are NOT READY";
        TextRenderer::drawText(readyText, -0.55f, 0.0f, 1.1f);

        // Draw action buttons
        drawButton(-0.3f, -0.2f, 0.25f, 0.08f, "Toggle Ready", false);
        drawButton(-0.0f, -0.2f, 0.25f, 0.08f, "Leave Lobby", false);

        if (isOwner) {
            drawButton(-0.3f, -0.35f, 0.55f, 0.08f, "Start Queue", !inQueue);
        }
    }

    void createLobby() {
        LobbyCreateRequest req;
        strncpy_s(req.lobbyName, "Player's Lobby", sizeof(req.lobbyName) - 1);
        req.maxPlayers = 5;
        req.isPrivate = false;

        networkClient->sendPacket(PacketType::LOBBY_CREATE, &req, sizeof(req));
        statusMessage = "Creating lobby...";
    }

    void toggleReady() {
        LobbyReady req;
        req.ready = !isReady;

        networkClient->sendPacket(PacketType::LOBBY_READY, &req, sizeof(req));
    }

    void leaveLobby() {
        networkClient->sendPacket(PacketType::LOBBY_LEAVE, nullptr, 0);
        inLobby = false;
        isReady = false;
        isOwner = false;
        inQueue = false;
        lobbyMembers.clear();
    }

    void startQueue() {
        if (!isOwner) return;

        networkClient->sendPacket(PacketType::LOBBY_START_QUEUE, nullptr, 0);
    }

    void handleLobbyCreateResponse(const std::vector<uint8_t>& payload) {
        if (payload.size() < sizeof(LobbyCreateResponse)) return;

        LobbyCreateResponse resp;
        memcpy(&resp, payload.data(), sizeof(LobbyCreateResponse));

        if (resp.success) {
            inLobby = true;
            currentLobbyId = resp.lobbyId;
            isOwner = true;
            statusMessage = "Lobby created successfully!";
        } else {
            statusMessage = std::string("Failed: ") + resp.errorMessage;
        }
    }

    void handleLobbyJoinResponse(const std::vector<uint8_t>& payload) {
        if (payload.size() < sizeof(LobbyJoinResponse)) return;

        LobbyJoinResponse resp;
        memcpy(&resp, payload.data(), sizeof(LobbyJoinResponse));

        if (resp.success) {
            inLobby = true;
            currentLobbyId = resp.lobbyId;
            isOwner = false;
            statusMessage = "Joined lobby successfully!";
        } else {
            statusMessage = std::string("Failed: ") + resp.errorMessage;
        }
    }

    void handleLobbyUpdate(const std::vector<uint8_t>& payload) {
        if (payload.size() < sizeof(LobbyUpdate)) return;

        LobbyUpdate update;
        memcpy(&update, payload.data(), sizeof(LobbyUpdate));

        // Update lobby state
        currentLobbyId = update.lobbyId;
        inQueue = update.inQueue;

        // Update members list
        lobbyMembers.clear();
        for (int i = 0; i < update.memberCount; i++) {
            LobbyMember member;
            member.accountId = update.members[i].accountId;
            member.username = std::string(update.members[i].username);
            member.isReady = update.members[i].isReady;
            member.isOwner = update.members[i].isOwner;

            lobbyMembers.push_back(member);

            // Check if we are owner or ready
            if (member.accountId == accountId) {
                isOwner = member.isOwner;
                isReady = member.isReady;
            }
        }
    }

    void handleMatchFound(const std::vector<uint8_t>& payload) {
        if (payload.size() < sizeof(MatchFound)) return;

        MatchFound match;
        memcpy(&match, payload.data(), sizeof(MatchFound));

        std::cout << "[LobbyUI] Match found! Map: " << match.mapName << std::endl;

        // Transition to in-game
        nextState = UIState::IN_GAME;
        changeState = true;
    }
};
