// Extraction Shooter - Client Entry Point with Scene Management
// Completely redesigned with hierarchical scene system, threading, and scheduling

#include "../engine/core/Platform.h"
#include <iostream>
#include <chrono>
#include <memory>

#include "network/NetworkClient.h"
#include "ui/UISceneManager.h"
#include "ui/LoginScene.h"
#include "ui/MainMenuScene.h"
#include "ui/UIText.h"
#include "../engine/GameEngine.h"
#include "../game/scenes/MenuScene.h"
#include "../game/scenes/RaidScene.h"

// Global variables
HWND g_hwnd = nullptr;
HDC g_hdc = nullptr;
HGLRC g_hglrc = nullptr;
bool g_running = true;

// Window dimensions and aspect ratio
int g_windowWidth = 1280;
int g_windowHeight = 720;
float g_aspectRatio = 16.0f / 9.0f;

// Mouse state
float g_mouseX = 0.0f;
float g_mouseY = 0.0f;

// Core systems
std::unique_ptr<NetworkClient> g_networkClient;
std::unique_ptr<UISceneManager> g_sceneManager;

// Scene references
std::shared_ptr<LoginScene> g_loginScene;
std::shared_ptr<MainMenuScene> g_mainMenuScene;

// Forward declarations
void handleSceneInput(char key);
void handleSceneMouseClick(float x, float y);
void handleSceneMouseMove(float x, float y);

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            g_running = false;
            PostQuitMessage(0);
            return 0;

        case WM_CHAR:
            handleSceneInput((char)wParam);
            return 0;

        case WM_SIZE:
            {
                // Update window dimensions
                g_windowWidth = LOWORD(lParam);
                g_windowHeight = HIWORD(lParam);
                if (g_windowHeight > 0) {
                    g_aspectRatio = (float)g_windowWidth / (float)g_windowHeight;
                }
                glViewport(0, 0, g_windowWidth, g_windowHeight);

                // Update projection matrix
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                float halfAspect = g_aspectRatio / 2.0f;
                glOrtho(-halfAspect, halfAspect, -1.0, 1.0, -1.0, 1.0);
                glMatrixMode(GL_MODELVIEW);
            }
            return 0;

        case WM_MOUSEMOVE:
            {
                int screenX = LOWORD(lParam);
                int screenY = HIWORD(lParam);

                // Convert to OpenGL coordinates
                float halfAspect = g_aspectRatio / 2.0f;
                g_mouseX = (screenX / (float)g_windowWidth) * g_aspectRatio - halfAspect;
                g_mouseY = 1.0f - (screenY / (float)g_windowHeight) * 2.0f;

                handleSceneMouseMove(g_mouseX, g_mouseY);
            }
            return 0;

        case WM_LBUTTONDOWN:
            {
                int screenX = LOWORD(lParam);
                int screenY = HIWORD(lParam);

                float halfAspect = g_aspectRatio / 2.0f;
                float glX = (screenX / (float)g_windowWidth) * g_aspectRatio - halfAspect;
                float glY = 1.0f - (screenY / (float)g_windowHeight) * 2.0f;

                handleSceneMouseClick(glX, glY);
            }
            return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Scene input handlers
void handleSceneInput(char key) {
    // Try UI scenes first
    if (g_sceneManager) {
        auto currentScene = g_sceneManager->getCurrentScene();
        if (currentScene) {
            // Route input to appropriate UI scene
            if (auto loginScene = std::dynamic_pointer_cast<LoginScene>(currentScene)) {
                loginScene->handleInput(key);
                return;
            } else if (auto mainMenuScene = std::dynamic_pointer_cast<MainMenuScene>(currentScene)) {
                mainMenuScene->handleInput(key);
                return;
            }
        }
    }

    // Try engine scenes
    if (ENGINE.isRunning() && ENGINE.getSceneManager() && ENGINE.getSceneManager()->hasActive()) {
        auto currentScene = ENGINE.getSceneManager()->current();
        if (auto menuScene = dynamic_cast<MenuScene*>(currentScene)) {
            menuScene->handleInput(key);
        } else if (auto raidScene = dynamic_cast<RaidScene*>(currentScene)) {
            raidScene->handleInput(key);
        }
    }
}

void handleSceneMouseClick(float x, float y) {
    // Try UI scenes first
    if (g_sceneManager) {
        auto currentScene = g_sceneManager->getCurrentScene();
        if (currentScene) {
            if (auto loginScene = std::dynamic_pointer_cast<LoginScene>(currentScene)) {
                loginScene->handleMouseClick(x, y);
                return;
            } else if (auto mainMenuScene = std::dynamic_pointer_cast<MainMenuScene>(currentScene)) {
                mainMenuScene->handleMouseClick(x, y);
                return;
            }
        }
    }

    // Try engine scenes
    if (ENGINE.isRunning() && ENGINE.getSceneManager() && ENGINE.getSceneManager()->hasActive()) {
        auto currentScene = ENGINE.getSceneManager()->current();
        if (auto menuScene = dynamic_cast<MenuScene*>(currentScene)) {
            menuScene->handleMouseClick(x, y);
        } else if (auto raidScene = dynamic_cast<RaidScene*>(currentScene)) {
            raidScene->handleMouseClick(x, y);
        }
    }
}

void handleSceneMouseMove(float x, float y) {
    // Try UI scenes first
    if (g_sceneManager) {
        auto currentScene = g_sceneManager->getCurrentScene();
        if (currentScene) {
            if (auto loginScene = std::dynamic_pointer_cast<LoginScene>(currentScene)) {
                loginScene->handleMouseMove(x, y);
                return;
            } else if (auto mainMenuScene = std::dynamic_pointer_cast<MainMenuScene>(currentScene)) {
                mainMenuScene->handleMouseMove(x, y);
                return;
            }
        }
    }

    // Try engine scenes
    if (ENGINE.isRunning() && ENGINE.getSceneManager() && ENGINE.getSceneManager()->hasActive()) {
        auto currentScene = ENGINE.getSceneManager()->current();
        if (auto menuScene = dynamic_cast<MenuScene*>(currentScene)) {
            menuScene->handleMouseMove(x, y);
        } else if (auto raidScene = dynamic_cast<RaidScene*>(currentScene)) {
            raidScene->handleMouseMove(x, y);
        }
    }
}

// Initialize OpenGL
bool initializeOpenGL() {
    // Create window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0, 0,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      L"ExtractionShooter", NULL };
    RegisterClassEx(&wc);

    g_hwnd = CreateWindow(L"ExtractionShooter", L"Extraction Shooter - Scene-Based Architecture",
                          WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720,
                          NULL, NULL, wc.hInstance, NULL);

    if (!g_hwnd) {
        std::cout << "[Client] Failed to create window!" << std::endl;
        return false;
    }

    // Get device context
    g_hdc = GetDC(g_hwnd);

    // Set pixel format
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), 1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        24, 8, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
    };

    int pixelFormat = ChoosePixelFormat(g_hdc, &pfd);
    SetPixelFormat(g_hdc, pixelFormat, &pfd);

    // Create OpenGL context
    g_hglrc = wglCreateContext(g_hdc);
    wglMakeCurrent(g_hdc, g_hglrc);

    // Initialize font for text rendering
    UIText::initFont(g_hdc);

    // Initialize OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    // Set up 2D orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float halfAspect = g_aspectRatio / 2.0f;
    glOrtho(-halfAspect, halfAspect, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    std::cout << "[Client] OpenGL initialized with scene system" << std::endl;

    return true;
}

// Initialize scenes
void initializeScenes() {
    std::cout << "[Client] Initializing scene system..." << std::endl;

    // Initialize GameEngine (required for game scenes)
    EngineConfig engineConfig;
    engineConfig.enableNetworking = true;
    engineConfig.fixedTimeStep = 1.0f / 60.0f;
    if (!ENGINE.initialize(engineConfig)) {
        std::cerr << "[Client] Failed to initialize GameEngine!" << std::endl;
        return;
    }

    // Register game scenes with engine SceneManager
    auto menuScene = ENGINE.getSceneManager()->registerScene<MenuScene>("menu", g_networkClient.get());
    auto raidScene = ENGINE.getSceneManager()->registerScene<RaidScene>("raid", g_networkClient.get(), 0);

    std::cout << "[Client] Registered engine scenes: menu, raid" << std::endl;

    // Create UI scene manager for login/main menu
    g_sceneManager = std::make_unique<UISceneManager>();

    // Create login scene
    g_loginScene = std::make_shared<LoginScene>(g_networkClient.get());
    g_sceneManager->registerScene("Login", g_loginScene);

    // Create main menu scene
    g_mainMenuScene = std::make_shared<MainMenuScene>(g_networkClient.get(), 0);
    g_sceneManager->registerScene("MainMenu", g_mainMenuScene);

    // Setup navigation callbacks for main menu
    g_mainMenuScene->setOnEnterLobby([]() {
        std::cout << "[Client] Starting game - switching to menu scene" << std::endl;
        // Switch to engine-managed menu scene
        ENGINE.getSceneManager()->switchTo("menu");
        // Hide UI scene manager rendering
        g_sceneManager->unloadAllScenes();
    });

    g_mainMenuScene->setOnViewStash([]() {
        std::cout << "[Client] Navigate to: Stash" << std::endl;
        // TODO: Implement stash scene
    });

    g_mainMenuScene->setOnOpenMerchants([]() {
        std::cout << "[Client] Navigate to: Merchants" << std::endl;
        // TODO: Implement merchant scene
    });

    g_mainMenuScene->setOnLogout([]() {
        std::cout << "[Client] Logging out..." << std::endl;
        g_sceneManager->transitionTo("Login", true);
    });

    // Load login scene initially
    g_sceneManager->loadScene("Login");

    // Schedule a task to check for successful login
    g_sceneManager->getScheduler().scheduleRepeating([]() {
        if (g_loginScene && g_loginScene->getAccountId() != 0) {
            std::cout << "[Client] Login successful! Transitioning to main menu..." << std::endl;

            // Update main menu with account ID
            uint64_t accountId = g_loginScene->getAccountId();
            g_mainMenuScene = std::make_shared<MainMenuScene>(g_networkClient.get(), accountId);
            g_sceneManager->registerScene("MainMenu", g_mainMenuScene);

            // Setup callbacks again
            g_mainMenuScene->setOnEnterLobby([]() {
                std::cout << "[Client] Navigate to: Lobby" << std::endl;
            });
            g_mainMenuScene->setOnViewStash([]() {
                std::cout << "[Client] Navigate to: Stash" << std::endl;
            });
            g_mainMenuScene->setOnOpenMerchants([]() {
                std::cout << "[Client] Navigate to: Merchants" << std::endl;
            });
            g_mainMenuScene->setOnLogout([]() {
                std::cout << "[Client] Logging out..." << std::endl;
                g_sceneManager->transitionTo("Login", true);
            });

            // Transition to main menu asynchronously
            g_sceneManager->transitionToAsync("MainMenu", true);
        }
    }, 0.1f); // Check every 100ms

    std::cout << "[Client] Scene system initialized!" << std::endl;
    std::cout << "[Client] Registered scenes: Login, MainMenu" << std::endl;
    std::cout << "[Client] Current scene: Login" << std::endl;
}

// Cleanup
void cleanup() {
    std::cout << "[Client] Unloading all scenes..." << std::endl;

    if (g_sceneManager) {
        g_sceneManager->unloadAllScenes();
        g_sceneManager.reset();
    }

    // Shutdown engine
    ENGINE.shutdown();

    g_mainMenuScene.reset();
    g_loginScene.reset();
    g_networkClient.reset();

    if (g_hglrc) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(g_hglrc);
    }

    if (g_hdc) {
        ReleaseDC(g_hwnd, g_hdc);
    }

    if (g_hwnd) {
        DestroyWindow(g_hwnd);
    }
}

// Main entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    std::cout << "========================================" << std::endl;
    std::cout << "  EXTRACTION SHOOTER - Scene System" << std::endl;
    std::cout << "  with Threading & Hierarchy" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Initialize OpenGL
    if (!initializeOpenGL()) {
        std::cout << "[Client] Failed to initialize OpenGL!" << std::endl;
        return 1;
    }

    // Create network client
    g_networkClient = std::make_unique<NetworkClient>();

    // Connect to server
    std::cout << "[Client] Connecting to server..." << std::endl;
    if (!g_networkClient->connect("127.0.0.1", 7777)) {
        std::cout << "[Client] Failed to connect to server!" << std::endl;
        std::cout << "[Client] Make sure the server is running on port 7777" << std::endl;
        cleanup();
        return 1;
    }

    std::cout << "[Client] Connected to server successfully!" << std::endl;

    // Initialize scene system
    initializeScenes();

    std::cout << "[Client] Client initialized successfully!" << std::endl;
    std::cout << "[Client] ThreadPool running with "
              << g_sceneManager->getThreadPool().getThreadCount() << " threads" << std::endl;
    std::cout << std::endl;

    // Main game loop
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (g_running) {
        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Process Windows messages
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                g_running = false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Update network
        g_networkClient->update();

        // Update UI scene manager (for login/main menu)
        if (g_sceneManager && g_sceneManager->getCurrentScene()) {
            g_sceneManager->update(deltaTime);
            g_sceneManager->render();
        }

        // Update engine (for game scenes - menu/raid)
        if (ENGINE.isRunning() && ENGINE.getSceneManager() && ENGINE.getSceneManager()->hasActive()) {
            ENGINE.update(deltaTime);
        }

        // Swap buffers
        SwapBuffers(g_hdc);

        // Limit frame rate (60 FPS)
        Sleep(16);
    }

    // Cleanup
    std::cout << "[Client] Shutting down..." << std::endl;
    cleanup();
    std::cout << "[Client] Shutdown complete" << std::endl;

    return 0;
}
