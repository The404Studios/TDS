#include "Server.h"
#include <iostream>
#include <chrono>
#include <thread>

namespace TDS {

Server::Server() : running(false), port(0) {}
Server::~Server() { shutdown(); }

bool Server::initialize(uint16_t p) {
    port = p;
    // TODO: Initialize ENet server
    std::cout << "[Server] Initialized on port " << port << "\n";
    running = true;
    return true;
}

void Server::run() {
    auto lastTime = std::chrono::steady_clock::now();
    const float targetDt = 1.0f / 60.0f;

    while (running) {
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        update(dt);

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void Server::update(float dt) {
    // TODO: Process packets, update game state
}

void Server::shutdown() {
    running = false;
    std::cout << "[Server] Shutting down\n";
}

}
