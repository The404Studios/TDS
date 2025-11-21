// ImGui Integration with raylib
// Provides advanced debugging and developer UI

#pragma once

#include "../RaylibPlatform.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <string>
#include <vector>

// Stats tracking for debug UI
struct GameStats {
    float fps;
    int playerCount;
    int corpseCount;
    int particleCount;
    float networkLatency;
    size_t networkBytesIn;
    size_t networkBytesOut;
    float serverTickRate;

    // Performance metrics
    float updateTimeMs;
    float renderTimeMs;
    float networkTimeMs;

    // Memory usage
    size_t memoryUsageMB;

    GameStats() : fps(0), playerCount(0), corpseCount(0), particleCount(0),
                  networkLatency(0), networkBytesIn(0), networkBytesOut(0),
                  serverTickRate(0), updateTimeMs(0), renderTimeMs(0),
                  networkTimeMs(0), memoryUsageMB(0) {}
};

class ImGuiManager {
private:
    bool m_initialized;
    bool m_showDebugWindow;
    bool m_showPerformanceWindow;
    bool m_showNetworkWindow;
    bool m_showConsole;
    bool m_showEntityList;

    // Console
    std::vector<std::string> m_consoleLog;
    char m_consoleInput[256];

    // Performance tracking
    std::vector<float> m_fpsHistory;
    std::vector<float> m_frameTimeHistory;
    static constexpr int HISTORY_SIZE = 100;

    GameStats m_stats;

public:
    ImGuiManager() : m_initialized(false), m_showDebugWindow(true),
                     m_showPerformanceWindow(true), m_showNetworkWindow(false),
                     m_showConsole(false), m_showEntityList(false) {
        memset(m_consoleInput, 0, sizeof(m_consoleInput));
        m_fpsHistory.reserve(HISTORY_SIZE);
        m_frameTimeHistory.reserve(HISTORY_SIZE);
    }

    ~ImGuiManager() {
        if (m_initialized) {
            shutdown();
        }
    }

    // Initialize ImGui with raylib
    bool initialize() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        // Enable keyboard navigation
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        // Setup style - dark theme
        ImGui::StyleColorsDark();

        // Customize colors for gaming aesthetic
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.12f, 0.95f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.17f, 1.0f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.2f, 0.2f, 0.24f, 1.0f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.5f, 0.8f, 0.7f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.6f, 0.9f, 0.8f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);

        // Setup Platform/Renderer backends
        // Note: For raylib, we need to get GLFW window handle
        // This is a simplified version - actual implementation may vary
        ImGui_ImplOpenGL3_Init("#version 330");

        m_initialized = true;
        addLog("ImGui initialized successfully");

        return true;
    }

    void shutdown() {
        if (!m_initialized) return;

        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();

        m_initialized = false;
    }

    // Begin new frame
    void beginFrame() {
        if (!m_initialized) return;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
    }

    // Render ImGui
    void render() {
        if (!m_initialized) return;

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // Update stats
    void updateStats(const GameStats& stats) {
        m_stats = stats;

        // Update history
        m_fpsHistory.push_back(stats.fps);
        if (m_fpsHistory.size() > HISTORY_SIZE) {
            m_fpsHistory.erase(m_fpsHistory.begin());
        }

        float frameTime = (stats.fps > 0) ? (1000.0f / stats.fps) : 0.0f;
        m_frameTimeHistory.push_back(frameTime);
        if (m_frameTimeHistory.size() > HISTORY_SIZE) {
            m_frameTimeHistory.erase(m_frameTimeHistory.begin());
        }
    }

    // Show main menu bar
    void showMenuBar() {
        if (!m_initialized) return;

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Windows")) {
                ImGui::MenuItem("Debug Info", nullptr, &m_showDebugWindow);
                ImGui::MenuItem("Performance", nullptr, &m_showPerformanceWindow);
                ImGui::MenuItem("Network", nullptr, &m_showNetworkWindow);
                ImGui::MenuItem("Console", nullptr, &m_showConsole);
                ImGui::MenuItem("Entity List", nullptr, &m_showEntityList);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("Controls")) {
                    addLog("WASD: Move, Shift: Sprint, F: Loot, Tab: Inventory");
                }
                if (ImGui::MenuItem("About")) {
                    addLog("Extraction Shooter v1.0 - Built with raylib & ImGui");
                }
                ImGui::EndMenu();
            }

            // FPS display on right side
            ImGui::SameLine(ImGui::GetWindowWidth() - 120);
            ImGui::Text("FPS: %.1f", m_stats.fps);

            ImGui::EndMainMenuBar();
        }
    }

    // Show debug window
    void showDebugWindow() {
        if (!m_initialized || !m_showDebugWindow) return;

        ImGui::Begin("Debug Info", &m_showDebugWindow);

        ImGui::Text("Game Stats");
        ImGui::Separator();
        ImGui::Text("FPS: %.1f", m_stats.fps);
        ImGui::Text("Players: %d", m_stats.playerCount);
        ImGui::Text("Corpses: %d", m_stats.corpseCount);
        ImGui::Text("Particles: %d", m_stats.particleCount);

        ImGui::Spacing();
        ImGui::Text("Network");
        ImGui::Separator();
        ImGui::Text("Latency: %.1f ms", m_stats.networkLatency);
        ImGui::Text("In: %.2f KB/s", m_stats.networkBytesIn / 1024.0f);
        ImGui::Text("Out: %.2f KB/s", m_stats.networkBytesOut / 1024.0f);
        ImGui::Text("Server Tick: %.1f Hz", m_stats.serverTickRate);

        ImGui::Spacing();
        ImGui::Text("Memory");
        ImGui::Separator();
        ImGui::Text("Usage: %zu MB", m_stats.memoryUsageMB);

        ImGui::End();
    }

    // Show performance window with graphs
    void showPerformanceWindow() {
        if (!m_initialized || !m_showPerformanceWindow) return;

        ImGui::Begin("Performance", &m_showPerformanceWindow);

        // FPS graph
        ImGui::Text("FPS History");
        if (!m_fpsHistory.empty()) {
            ImGui::PlotLines("##FPS", m_fpsHistory.data(), (int)m_fpsHistory.size(),
                           0, nullptr, 0.0f, 120.0f, ImVec2(0, 80));
        }

        ImGui::Spacing();

        // Frame time graph
        ImGui::Text("Frame Time (ms)");
        if (!m_frameTimeHistory.empty()) {
            ImGui::PlotLines("##FrameTime", m_frameTimeHistory.data(),
                           (int)m_frameTimeHistory.size(),
                           0, nullptr, 0.0f, 33.0f, ImVec2(0, 80));
        }

        ImGui::Spacing();
        ImGui::Separator();

        // Timing breakdown
        ImGui::Text("Update: %.2f ms", m_stats.updateTimeMs);
        ImGui::Text("Render: %.2f ms", m_stats.renderTimeMs);
        ImGui::Text("Network: %.2f ms", m_stats.networkTimeMs);

        float total = m_stats.updateTimeMs + m_stats.renderTimeMs + m_stats.networkTimeMs;
        ImGui::Text("Total: %.2f ms", total);

        ImGui::End();
    }

    // Show network window
    void showNetworkWindow() {
        if (!m_initialized || !m_showNetworkWindow) return;

        ImGui::Begin("Network Stats", &m_showNetworkWindow);

        ImGui::Text("Latency: %.1f ms", m_stats.networkLatency);
        ImGui::ProgressBar(m_stats.networkLatency / 200.0f, ImVec2(-1, 0),
                          m_stats.networkLatency < 50 ? "Excellent" :
                          m_stats.networkLatency < 100 ? "Good" : "Poor");

        ImGui::Spacing();
        ImGui::Text("Bandwidth");
        ImGui::Text("Download: %.2f KB/s", m_stats.networkBytesIn / 1024.0f);
        ImGui::Text("Upload: %.2f KB/s", m_stats.networkBytesOut / 1024.0f);

        ImGui::Spacing();
        ImGui::Text("Server Tick Rate: %.1f Hz", m_stats.serverTickRate);

        ImGui::End();
    }

    // Show console
    void showConsole() {
        if (!m_initialized || !m_showConsole) return;

        ImGui::Begin("Console", &m_showConsole);

        // Log area
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -30), false,
                         ImGuiWindowFlags_HorizontalScrollbar);

        for (const auto& log : m_consoleLog) {
            ImGui::TextUnformatted(log.c_str());
        }

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();

        ImGui::Separator();

        // Input
        if (ImGui::InputText("##Input", m_consoleInput, sizeof(m_consoleInput),
                            ImGuiInputTextFlags_EnterReturnsTrue)) {
            addLog(std::string("> ") + m_consoleInput);
            // Process command here
            memset(m_consoleInput, 0, sizeof(m_consoleInput));
        }

        ImGui::End();
    }

    // Add log message
    void addLog(const std::string& message) {
        m_consoleLog.push_back(message);
        if (m_consoleLog.size() > 1000) {
            m_consoleLog.erase(m_consoleLog.begin());
        }
    }

    // Toggle windows
    void toggleDebugWindow() { m_showDebugWindow = !m_showDebugWindow; }
    void togglePerformanceWindow() { m_showPerformanceWindow = !m_showPerformanceWindow; }
    void toggleNetworkWindow() { m_showNetworkWindow = !m_showNetworkWindow; }
    void toggleConsole() { m_showConsole = !m_showConsole; }

    bool isInitialized() const { return m_initialized; }
};
