// JSON-based Configuration Manager
// Modern replacement for INI-based configuration

#pragma once

#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <string>

using json = nlohmann::json;

class ConfigManager {
private:
    json m_config;
    std::string m_configPath;
    bool m_loaded;

public:
    ConfigManager() : m_loaded(false) {}

    // Load configuration from JSON file
    bool load(const std::string& path) {
        m_configPath = path;

        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[Config] Failed to open: " << path << std::endl;
            // Create default config
            createDefaultConfig();
            save();
            m_loaded = true;
            return true;
        }

        try {
            file >> m_config;
            m_loaded = true;
            std::cout << "[Config] Loaded from: " << path << std::endl;
            return true;
        } catch (const json::exception& e) {
            std::cerr << "[Config] JSON parse error: " << e.what() << std::endl;
            createDefaultConfig();
            m_loaded = true;
            return false;
        }
    }

    // Save configuration to JSON file
    bool save() {
        if (m_configPath.empty()) {
            std::cerr << "[Config] No config path set" << std::endl;
            return false;
        }

        std::ofstream file(m_configPath);
        if (!file.is_open()) {
            std::cerr << "[Config] Failed to save: " << m_configPath << std::endl;
            return false;
        }

        file << m_config.dump(4); // Pretty print with 4 spaces
        std::cout << "[Config] Saved to: " << m_configPath << std::endl;
        return true;
    }

    // Get value with default
    template<typename T>
    T get(const std::string& key, const T& defaultValue) const {
        try {
            return m_config.value(key, defaultValue);
        } catch (const json::exception& e) {
            std::cerr << "[Config] Error getting " << key << ": " << e.what() << std::endl;
            return defaultValue;
        }
    }

    // Get nested value with path (e.g., "graphics.resolution.width")
    template<typename T>
    T getNested(const std::string& path, const T& defaultValue) const {
        try {
            json value = m_config;
            size_t start = 0;
            size_t end = path.find('.');

            while (end != std::string::npos) {
                std::string key = path.substr(start, end - start);
                if (!value.contains(key)) return defaultValue;
                value = value[key];
                start = end + 1;
                end = path.find('.', start);
            }

            std::string key = path.substr(start);
            if (!value.contains(key)) return defaultValue;

            return value[key].get<T>();
        } catch (const json::exception& e) {
            return defaultValue;
        }
    }

    // Set value
    template<typename T>
    void set(const std::string& key, const T& value) {
        m_config[key] = value;
    }

    // Set nested value
    template<typename T>
    void setNested(const std::string& path, const T& value) {
        json* current = &m_config;
        size_t start = 0;
        size_t end = path.find('.');

        while (end != std::string::npos) {
            std::string key = path.substr(start, end - start);
            if (!current->contains(key)) {
                (*current)[key] = json::object();
            }
            current = &(*current)[key];
            start = end + 1;
            end = path.find('.', start);
        }

        std::string key = path.substr(start);
        (*current)[key] = value;
    }

    // Check if key exists
    bool has(const std::string& key) const {
        return m_config.contains(key);
    }

    // Get entire section as JSON
    json getSection(const std::string& section) const {
        if (m_config.contains(section)) {
            return m_config[section];
        }
        return json::object();
    }

    // Create default configuration
    void createDefaultConfig() {
        m_config = {
            {"game", {
                {"title", "Extraction Shooter"},
                {"version", "1.0.0"},
                {"maxPlayers", 100}
            }},
            {"graphics", {
                {"resolution", {
                    {"width", 1920},
                    {"height", 1080}
                }},
                {"fullscreen", false},
                {"vsync", true},
                {"targetFPS", 60},
                {"renderDistance", 1000.0f},
                {"shadows", true},
                {"shadowQuality", "high"},
                {"antialiasing", true},
                {"postProcessing", true},
                {"bloom", true},
                {"ambientOcclusion", true}
            }},
            {"audio", {
                {"masterVolume", 0.7f},
                {"musicVolume", 0.5f},
                {"sfxVolume", 0.8f},
                {"voiceVolume", 0.9f},
                {"enable3DAudio", true}
            }},
            {"network", {
                {"serverIP", "127.0.0.1"},
                {"serverPort", 7777},
                {"timeout", 10000},
                {"tickRate", 60},
                {"compression", true},
                {"nat", {
                    {"enabled", true},
                    {"serverIP", "127.0.0.1"},
                    {"serverPort", 3478}
                }}
            }},
            {"server", {
                {"port", 7777},
                {"maxPlayers", 100},
                {"tickRate", 60},
                {"savePath", "Data/"},
                {"antiCheat", {
                    {"enabled", true},
                    {"maxSpeed", 15.0f},
                    {"teleportThreshold", 50.0f}
                }},
                {"match", {
                    {"duration", 1800},
                    {"extractionZones", 3},
                    {"lootSpawnMin", 30},
                    {"lootSpawnMax", 60},
                    {"aiCount", 15}
                }}
            }},
            {"controls", {
                {"mouseSensitivity", 1.0f},
                {"invertY", false},
                {"toggleCrouch", false},
                {"toggleSprint", false}
            }},
            {"ui", {
                {"showFPS", true},
                {"showPing", true},
                {"hudScale", 1.0f},
                {"crosshairSize", 10},
                {"damageIndicators", true}
            }},
            {"debug", {
                {"enabled", false},
                {"showImGui", true},
                {"showColliders", false},
                {"showNetworkStats", true},
                {"logLevel", "info"}
            }}
        };
    }

    bool isLoaded() const { return m_loaded; }

    // Get raw JSON (for advanced usage)
    const json& getRaw() const { return m_config; }
    json& getRaw() { return m_config; }
};

// Global config instance
extern ConfigManager g_config;
