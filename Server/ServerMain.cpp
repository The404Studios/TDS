// Neural Civilization - Dedicated Server
// Handles authoritative game state and synchronizes to clients

#include "../CivilizationAI.h"
#include "../NetworkManager.h"
#include "../GameplayFeatures.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>
#include <atomic>

std::atomic<bool> g_serverRunning(true);

void signalHandler(int signal) {
    std::cout << "\nShutdown signal received..." << std::endl;
    g_serverRunning = false;
}

class GameServer {
private:
    std::unique_ptr<World> world;
    std::unique_ptr<ServerNetworkManager> networkManager;
    std::unique_ptr<AdvancedGameManager> gameManager;
    std::unique_ptr<MarketSystem> marketSystem;

    float tickRate;
    float accumulator;
    bool running;

    // Server statistics
    int totalTicks;
    float totalTime;
    int totalPlayersConnected;

public:
    GameServer(int port = 27015)
        : tickRate(1.0f / 60.0f), accumulator(0), running(false),
          totalTicks(0), totalTime(0), totalPlayersConnected(0) {

        // Create network manager
        networkManager = std::make_unique<ServerNetworkManager>(port);

        // Create world
        world = std::make_unique<World>();
        world->WORLD_SIZE = 500;
        world->MAX_AGENTS_PER_FACTION = 100;

        // Create game systems
        gameManager = std::make_unique<AdvancedGameManager>();
        marketSystem = std::make_unique<MarketSystem>();

        std::cout << "========================================" << std::endl;
        std::cout << " NEURAL CIVILIZATION - DEDICATED SERVER" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Server Port: " << port << std::endl;
        std::cout << "World Size: " << world->WORLD_SIZE << std::endl;
        std::cout << "Tick Rate: " << (1.0f / tickRate) << " Hz" << std::endl;
        std::cout << "========================================" << std::endl;
    }

    bool initialize() {
        std::cout << "Initializing server..." << std::endl;

        // Initialize network
        if (!networkManager->initialize()) {
            std::cerr << "Failed to initialize network manager!" << std::endl;
            return false;
        }

        // Initialize world
        std::cout << "Initializing world simulation..." << std::endl;
        world->initialize();

        running = true;
        std::cout << "Server initialized successfully!" << std::endl;
        std::cout << "Waiting for clients..." << std::endl;

        return true;
    }

    void update(float deltaTime) {
        // Update network (accept clients, receive packets)
        networkManager->update(deltaTime);

        // Process incoming packets
        while (networkManager->hasPackets()) {
            NetworkPacket packet = networkManager->getNextPacket();
            processClientPacket(packet);
        }

        // Update world simulation
        world->update(deltaTime);

        // Update advanced game systems
        gameManager->update(deltaTime, world.get());
        marketSystem->updatePrices(deltaTime);

        // Update faction AI with advanced features
        for (auto& faction : world->factions) {
            // Cast to AdvancedFactionManager if needed
            // For now, use base class update
            faction->update(deltaTime);
        }

        // Broadcast world state to clients (every 100ms)
        static float syncTimer = 0;
        syncTimer += deltaTime;
        if (syncTimer >= 0.1f) {
            networkManager->broadcastWorldState(world.get());
            syncTimer = 0;
        }

        // Update statistics
        totalTicks++;
        totalTime += deltaTime;
    }

    void processClientPacket(const NetworkPacket& packet) {
        switch (packet.type) {
            case PacketType::PLAYER_INPUT:
                handlePlayerInput(packet);
                break;

            case PacketType::CHANGE_ROLE:
                handleChangeRole(packet);
                break;

            case PacketType::BUILD_ORDER:
                handleBuildOrder(packet);
                break;

            case PacketType::CHAT_MESSAGE:
                handleChatMessage(packet);
                break;

            case PacketType::DIPLOMACY_ACTION:
                handleDiplomacyAction(packet);
                break;

            case PacketType::TRADE_OFFER:
                handleTradeOffer(packet);
                break;

            default:
                break;
        }
    }

    void handlePlayerInput(const NetworkPacket& packet) {
        size_t offset = 0;
        Vector3 clickPos = packet.readVector3(offset);
        // Process player click/input
    }

    void handleChangeRole(const NetworkPacket& packet) {
        size_t offset = 0;
        uint32_t agentId = packet.readUInt32(offset);
        uint8_t roleVal = packet.readUInt8(offset);
        Role newRole = static_cast<Role>(roleVal);

        // Find agent and change role
        // Note: In production, you'd need agent IDs for proper lookup
        if (!world->allAgents.empty() && agentId < world->allAgents.size()) {
            world->allAgents[agentId]->changeRole(newRole);
        }
    }

    void handleBuildOrder(const NetworkPacket& packet) {
        size_t offset = 0;
        uint8_t typeVal = packet.readUInt8(offset);
        Vector3 position = packet.readVector3(offset);

        Building::Type buildingType = static_cast<Building::Type>(typeVal);

        // Create building (should check resources, permissions, etc.)
        // For now, just add to first faction as example
        if (!world->factions.empty()) {
            auto building = std::make_unique<Building>(buildingType, position, Faction::RED);
            world->allBuildings.push_back(std::move(building));

            // Broadcast to all clients
            networkManager->broadcastBuildingUpdate(world->allBuildings.back().get());
        }
    }

    void handleChatMessage(const NetworkPacket& packet) {
        size_t offset = 0;
        std::string message = packet.readString(offset);
        std::cout << "[CHAT] " << message << std::endl;

        // Broadcast to all clients
        networkManager->broadcastPacket(packet);
    }

    void handleDiplomacyAction(const NetworkPacket& packet) {
        // Handle diplomatic actions between factions
        size_t offset = 0;
        uint8_t factionA = packet.readUInt8(offset);
        uint8_t factionB = packet.readUInt8(offset);
        uint8_t action = packet.readUInt8(offset);

        std::cout << "Diplomacy: Faction " << (int)factionA
                  << " performs action " << (int)action
                  << " toward Faction " << (int)factionB << std::endl;
    }

    void handleTradeOffer(const NetworkPacket& packet) {
        // Handle trade offers between factions
        std::cout << "Trade offer received" << std::endl;
    }

    void run() {
        using clock = std::chrono::high_resolution_clock;
        auto lastTime = clock::now();

        while (running && g_serverRunning) {
            auto currentTime = clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            // Cap delta time to prevent spiral of death
            if (deltaTime > 0.25f) {
                deltaTime = 0.25f;
            }

            accumulator += deltaTime;

            // Fixed timestep updates
            while (accumulator >= tickRate) {
                update(tickRate);
                accumulator -= tickRate;
            }

            // Print statistics every 10 seconds
            static float statTimer = 0;
            statTimer += deltaTime;
            if (statTimer >= 10.0f) {
                printStatistics();
                statTimer = 0;
            }

            // Sleep to maintain tick rate
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        shutdown();
    }

    void printStatistics() {
        const NetworkStats& netStats = networkManager->getStats();

        std::cout << "\n========== SERVER STATISTICS ==========" << std::endl;
        std::cout << "Uptime: " << (int)totalTime << " seconds" << std::endl;
        std::cout << "Total Ticks: " << totalTicks << std::endl;
        std::cout << "Generation: " << world->currentGeneration << std::endl;
        std::cout << "Connected Clients: " << networkManager->getClientCount() << std::endl;
        std::cout << "Total Agents: " << world->allAgents.size() << std::endl;
        std::cout << "Total Buildings: " << world->allBuildings.size() << std::endl;
        std::cout << "Weather: " << (int)gameManager->getCurrentWeather().type << std::endl;
        std::cout << "Season: " << (int)gameManager->getCurrentSeason() << std::endl;

        std::cout << "\n-- Network Statistics --" << std::endl;
        std::cout << "Packets Sent: " << netStats.packetsSent << std::endl;
        std::cout << "Packets Received: " << netStats.packetsReceived << std::endl;
        std::cout << "Bytes Sent: " << netStats.bytesSent << std::endl;
        std::cout << "Bytes Received: " << netStats.bytesReceived << std::endl;

        std::cout << "\n-- Faction Statistics --" << std::endl;
        for (int i = 0; i < 3; ++i) {
            std::cout << "Faction " << i << ": "
                      << world->factions[i]->getPopulation() << " agents, "
                      << world->factions[i]->buildings.size() << " buildings"
                      << std::endl;
        }
        std::cout << "======================================\n" << std::endl;
    }

    void shutdown() {
        std::cout << "Shutting down server..." << std::endl;
        networkManager->shutdown();
        std::cout << "Server shut down successfully." << std::endl;
    }

    void stop() {
        running = false;
    }
};

int main(int argc, char* argv[]) {
    // Set up signal handlers for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Parse command line arguments
    int port = 27015;
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }

    std::cout << "Starting Neural Civilization Server on port " << port << "..." << std::endl;

    // Create and initialize server
    GameServer server(port);

    if (!server.initialize()) {
        std::cerr << "Failed to initialize server!" << std::endl;
        return 1;
    }

    // Run server
    std::cout << "\nServer running. Press Ctrl+C to stop.\n" << std::endl;
    server.run();

    std::cout << "Server terminated." << std::endl;
    return 0;
}
