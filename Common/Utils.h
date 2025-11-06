#pragma once
#include <string>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <random>

// ============================================================================
// UTILITY FUNCTIONS
// Shared helper functions for both client and server
// ============================================================================

// Simple hash function (in production, use proper crypto library like OpenSSL)
inline std::string simpleHash(const std::string& input) {
    uint64_t hash = 0;
    for (char c : input) {
        hash = hash * 31 + static_cast<uint8_t>(c);
    }

    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << hash;
    return ss.str();
}

// Generate random session token
inline uint64_t generateSessionToken() {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;
    return dis(gen);
}

// Generate random unique ID
inline uint32_t generateUniqueId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis(1, 0xFFFFFFFF);
    return dis(gen);
}
