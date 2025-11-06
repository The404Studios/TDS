// Extraction Shooter - Client Entry Point
// Multiplayer Tarkov-style extraction shooter with lobby system

// IMPORTANT: Include winsock2 BEFORE windows.h to avoid conflicts
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <iostream>
#include <chrono>

#include "network/NetworkClient.h"
#include "ui/UIManager.h"
#include "ui/LoginUI.h"
#include "ui/LobbyUI.h"
#include "ui/MainMenuUI.h"
#include "ui/GameClient.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

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

NetworkClient* g_networkClient = nullptr;
UIManager* g_uiManager = nullptr;

// Current UI screens
LoginUI* g_loginUI = nullptr;
LobbyUI* g_lobbyUI = nullptr;
MainMenuUI* g_mainMenuUI = nullptr;
GameClient* g_gameClient = nullptr;

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            g_running = false;
            PostQuitMessage(0);
            return 0;

        case WM_CHAR:
            // Forward input to UI
            if (g_uiManager) {
                g_uiManager->handleInput((char)wParam);
            }
            return 0;

        case WM_SIZE:
            // Update window dimensions
            g_windowWidth = LOWORD(lParam);
            g_windowHeight = HIWORD(lParam);
            if (g_windowHeight > 0) {
                g_aspectRatio = (float)g_windowWidth / (float)g_windowHeight;
                if (g_uiManager) {
                    g_uiManager->setAspectRatio(g_aspectRatio);
                }
            }
            glViewport(0, 0, g_windowWidth, g_windowHeight);
            return 0;

        case WM_MOUSEMOVE:
            {
                // Get mouse position in screen coordinates
                int screenX = LOWORD(lParam);
                int screenY = HIWORD(lParam);

                // Convert to OpenGL coordinates (centered coordinate system)
                // X: -aspectRatio/2 to aspectRatio/2
                // Y: -1 to 1 (flipped)
                float halfAspect = g_aspectRatio / 2.0f;
                g_mouseX = (screenX / (float)g_windowWidth) * g_aspectRatio - halfAspect;
                g_mouseY = 1.0f - (screenY / (float)g_windowHeight) * 2.0f;

                // Forward mouse position to UI manager
                if (g_uiManager) {
                    g_uiManager->setMousePosition(g_mouseX, g_mouseY);
                }
            }
            return 0;

        case WM_LBUTTONDOWN:
            {
                // Get mouse position in screen coordinates
                int screenX = LOWORD(lParam);
                int screenY = HIWORD(lParam);

                // Convert to OpenGL coordinates (centered coordinate system)
                float halfAspect = g_aspectRatio / 2.0f;
                float glX = (screenX / (float)g_windowWidth) * g_aspectRatio - halfAspect;
                float glY = 1.0f - (screenY / (float)g_windowHeight) * 2.0f;

                // Forward click to UI manager
                if (g_uiManager) {
                    g_uiManager->handleMouseClick(glX, glY);
                }
            }
            return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Initialize OpenGL
bool initializeOpenGL() {
    // Create window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0, 0,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      L"ExtractionShooter", NULL };
    RegisterClassEx(&wc);

    g_hwnd = CreateWindow(L"ExtractionShooter", L"Extraction Shooter - Multiplayer",
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
    TextRenderer::initFont(g_hdc);

    // Initialize OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    // Set up 2D orthographic projection for UI
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Use aspect ratio-aware orthographic projection centered at origin
    // X: -aspectRatio/2 to aspectRatio/2, Y: -1 to 1
    float halfAspect = g_aspectRatio / 2.0f;
    glOrtho(-halfAspect, halfAspect, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    std::cout << "[Client] OpenGL initialized" << std::endl;

    return true;
}

// Cleanup
void cleanup() {
    if (g_gameClient) delete g_gameClient;
    if (g_mainMenuUI) delete g_mainMenuUI;
    if (g_lobbyUI) delete g_lobbyUI;
    if (g_loginUI) delete g_loginUI;
    if (g_uiManager) delete g_uiManager;
    if (g_networkClient) delete g_networkClient;

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
    std::cout << "  EXTRACTION SHOOTER - Multiplayer Game" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Initialize OpenGL
    if (!initializeOpenGL()) {
        std::cout << "[Client] Failed to initialize OpenGL!" << std::endl;
        return 1;
    }

    // Create network client
    g_networkClient = new NetworkClient();

    // Connect to server
    std::cout << "[Client] Connecting to server..." << std::endl;
    if (!g_networkClient->connect("127.0.0.1", 7777)) {
        std::cout << "[Client] Failed to connect to server!" << std::endl;
        std::cout << "[Client] Make sure the server is running on port 7777" << std::endl;
        cleanup();
        return 1;
    }

    std::cout << "[Client] Connected to server successfully!" << std::endl;

    // Create UI manager
    g_uiManager = new UIManager();
    g_uiManager->setAspectRatio(g_aspectRatio);

    // Create login UI (starting state)
    g_loginUI = new LoginUI(g_networkClient);
    g_uiManager->setState(UIState::LOGIN, g_loginUI);

    std::cout << "[Client] Client initialized successfully!" << std::endl;
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

        // Update UI
        g_uiManager->update(deltaTime);

        // Handle state transitions
        if (g_uiManager->shouldChangeState()) {
            UIState nextState = g_uiManager->getNextState();

            switch (nextState) {
                case UIState::MAIN_MENU:
                    if (!g_mainMenuUI) {
                        g_mainMenuUI = new MainMenuUI(g_networkClient, g_loginUI->getAccountId());
                    }
                    g_uiManager->setState(UIState::MAIN_MENU, g_mainMenuUI);
                    break;

                case UIState::LOBBY:
                    if (!g_lobbyUI) {
                        g_lobbyUI = new LobbyUI(g_networkClient, g_loginUI->getAccountId());
                    }
                    g_uiManager->setState(UIState::LOBBY, g_lobbyUI);
                    break;

                case UIState::STASH:
                    // TODO: Create StashUI class
                    // For now, redirect to main menu
                    if (!g_mainMenuUI) {
                        g_mainMenuUI = new MainMenuUI(g_networkClient, g_loginUI->getAccountId());
                    }
                    g_uiManager->setState(UIState::MAIN_MENU, g_mainMenuUI);
                    break;

                case UIState::MERCHANT:
                    // TODO: Create MerchantUI class
                    // For now, redirect to main menu
                    if (!g_mainMenuUI) {
                        g_mainMenuUI = new MainMenuUI(g_networkClient, g_loginUI->getAccountId());
                    }
                    g_uiManager->setState(UIState::MAIN_MENU, g_mainMenuUI);
                    break;

                case UIState::IN_GAME:
                    if (!g_gameClient) {
                        g_gameClient = new GameClient(g_networkClient, g_loginUI->getAccountId());
                    }
                    g_uiManager->setState(UIState::IN_GAME, g_gameClient);
                    break;

                case UIState::LOGIN:
                    // Cleanup UI instances on logout
                    if (g_gameClient) {
                        delete g_gameClient;
                        g_gameClient = nullptr;
                    }
                    if (g_mainMenuUI) {
                        delete g_mainMenuUI;
                        g_mainMenuUI = nullptr;
                    }
                    if (g_lobbyUI) {
                        delete g_lobbyUI;
                        g_lobbyUI = nullptr;
                    }
                    g_uiManager->setState(UIState::LOGIN, g_loginUI);
                    break;

                default:
                    break;
            }

            g_uiManager->resetTransition();
        }

        // Render
        g_uiManager->render();

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
