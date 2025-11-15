// Extraction Shooter - Raylib Client Entry Point
// 3D Multiplayer Looter Shooter with raylib, raygui, and NAT punchthrough

#include "raylib.h"
#include "RaylibGameClient.h"
#include "network/NetworkClient.h"
#include "../natpunch/NatPunchClient.h"
#include <iostream>
#include <memory>

// Global settings
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const char* WINDOW_TITLE = "Extraction Shooter - 3D Looter Shooter";

// Server configuration
const char* GAME_SERVER_IP = "127.0.0.1";
const int GAME_SERVER_PORT = 7777;
const char* NAT_SERVER_IP = "127.0.0.1";
const int NAT_SERVER_PORT = 3478;

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "  EXTRACTION SHOOTER" << std::endl;
    std::cout << "  3D Multiplayer Looter Shooter" << std::endl;
    std::cout << "  Built with raylib & raygui" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Parse command line arguments
    std::string serverIp = GAME_SERVER_IP;
    int serverPort = GAME_SERVER_PORT;
    bool useNatPunch = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--server" && i + 1 < argc) {
            serverIp = argv[++i];
        }
        else if (arg == "--port" && i + 1 < argc) {
            serverPort = atoi(argv[++i]);
        }
        else if (arg == "--nat") {
            useNatPunch = true;
        }
        else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --server <ip>    Game server IP (default: 127.0.0.1)" << std::endl;
            std::cout << "  --port <port>    Game server port (default: 7777)" << std::endl;
            std::cout << "  --nat            Use NAT punchthrough" << std::endl;
            std::cout << "  --help           Show this help message" << std::endl;
            return 0;
        }
    }

    std::cout << "[Client] Connecting to game server: " << serverIp << ":" << serverPort << std::endl;

    // Initialize raylib
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(60);
    SetExitKey(0); // Disable ESC to close window (we'll handle it manually)

    std::cout << "[Client] Raylib initialized: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << std::endl;

    // Initialize audio (for future sound effects)
    InitAudioDevice();
    std::cout << "[Client] Audio device initialized" << std::endl;

    // Create network client
    std::unique_ptr<NetworkClient> networkClient = std::make_unique<NetworkClient>();

    // Optional: Initialize NAT punchthrough
    std::unique_ptr<NatPunchClient> natClient;
    if (useNatPunch) {
        std::cout << "[Client] Initializing NAT punchthrough..." << std::endl;
        natClient = std::make_unique<NatPunchClient>("Player_" + std::to_string(GetRandomValue(1000, 9999)),
                                                      NAT_SERVER_IP, NAT_SERVER_PORT);

        if (natClient->connect()) {
            std::cout << "[Client] NAT punchthrough enabled" << std::endl;

            // Set up peer connection callback
            natClient->setOnPeerInfoReceived([&](const PeerInfo& peer) {
                std::cout << "[Client] Peer discovered: " << peer.clientId
                          << " at " << peer.ipAddress << ":" << peer.port << std::endl;
                // TODO: Establish direct P2P connection for voice chat or data sharing
            });
        } else {
            std::cout << "[Client] NAT punchthrough failed, continuing without it" << std::endl;
            natClient.reset();
        }
    }

    // Connect to game server
    std::cout << "[Client] Connecting to game server..." << std::endl;
    if (!networkClient->connect(serverIp.c_str(), serverPort)) {
        std::cout << "[Client] Failed to connect to game server!" << std::endl;
        std::cout << "[Client] Make sure the server is running on " << serverIp << ":" << serverPort << std::endl;

        DrawText("Failed to connect to server!", 350, 300, 30, RED);
        DrawText("Make sure the server is running", 380, 350, 20, DARKGRAY);
        DrawText("Press any key to exit", 440, 400, 20, DARKGRAY);
        EndDrawing();

        WaitTime(3.0f);

        CloseAudioDevice();
        CloseWindow();
        return 1;
    }

    std::cout << "[Client] Connected to game server successfully!" << std::endl;

    // Create game client
    RaylibGameClient gameClient(networkClient.get(), SCREEN_WIDTH, SCREEN_HEIGHT);
    gameClient.initialize();

    std::cout << "[Client] Game client initialized" << std::endl;
    std::cout << "[Client] Starting main game loop..." << std::endl;
    std::cout << std::endl;

    // Main game loop
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // Update network
        networkClient->update();

        // Update NAT client if enabled
        if (natClient) {
            natClient->update();
        }

        // Update game
        gameClient.update(deltaTime);

        // Render game
        gameClient.render();
    }

    // Cleanup
    std::cout << "[Client] Shutting down..." << std::endl;

    if (natClient) {
        natClient->disconnect();
    }

    networkClient.reset();

    CloseAudioDevice();
    CloseWindow();

    std::cout << "[Client] Shutdown complete" << std::endl;

    return 0;
}
