// Simplified version using only Windows OpenGL (no external dependencies required)
/*#define _USE_MATH_DEFINES
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include "CivilizationAI.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

// Global variables
World* g_world = nullptr;
HWND g_hWnd = nullptr;
HDC g_hDC = nullptr;
HGLRC g_hRC = nullptr;
int g_windowWidth = 1280;
int g_windowHeight = 720;
float g_cameraX = 50.0f;
float g_cameraY = 50.0f;
float g_cameraZ = 60.0f;
float g_cameraRotX = 45.0f;
float g_cameraRotY = 0.0f;
int g_lastMouseX = 0;
int g_lastMouseY = 0;
bool g_mouseDown = false;
bool g_paused = false;
float g_simulationSpeed = 1.0f;
DWORD g_lastTime = 0;

// Color definitions
struct Color {
    float r, g, b;
    Color(float r, float g, float b) : r(r), g(g), b(b) {}
};

Color getFactionColor(Faction f) {
    switch (f) {
        case Faction::RED: return Color(1.0f, 0.2f, 0.2f);
        case Faction::BLUE: return Color(0.2f, 0.2f, 1.0f);
        case Faction::GREEN: return Color(0.2f, 1.0f, 0.2f);
        default: return Color(0.5f, 0.5f, 0.5f);
    }
}

// Drawing functions
void drawCube(float size) {
    float h = size / 2.0f;
    
    glBegin(GL_QUADS);
    // Front
    glNormal3f(0, 0, 1);
    glVertex3f(-h, -h, h);
    glVertex3f(h, -h, h);
    glVertex3f(h, h, h);
    glVertex3f(-h, h, h);
    
    // Back
    glNormal3f(0, 0, -1);
    glVertex3f(-h, -h, -h);
    glVertex3f(-h, h, -h);
    glVertex3f(h, h, -h);
    glVertex3f(h, -h, -h);
    
    // Top
    glNormal3f(0, 1, 0);
    glVertex3f(-h, h, -h);
    glVertex3f(-h, h, h);
    glVertex3f(h, h, h);
    glVertex3f(h, h, -h);
    
    // Bottom
    glNormal3f(0, -1, 0);
    glVertex3f(-h, -h, -h);
    glVertex3f(h, -h, -h);
    glVertex3f(h, -h, h);
    glVertex3f(-h, -h, h);
    
    // Right
    glNormal3f(1, 0, 0);
    glVertex3f(h, -h, -h);
    glVertex3f(h, h, -h);
    glVertex3f(h, h, h);
    glVertex3f(h, -h, h);
    
    // Left
    glNormal3f(-1, 0, 0);
    glVertex3f(-h, -h, -h);
    glVertex3f(-h, -h, h);
    glVertex3f(-h, h, h);
    glVertex3f(-h, h, -h);
    glEnd();
}

void drawSphere(float radius, int slices = 16, int stacks = 16) {
    for (int i = 0; i < stacks; ++i) {
        float lat0 = M_PI * (-0.5f + float(i) / stacks);
        float z0 = sin(lat0) * radius;
        float r0 = cos(lat0) * radius;
        
        float lat1 = M_PI * (-0.5f + float(i + 1) / stacks);
        float z1 = sin(lat1) * radius;
        float r1 = cos(lat1) * radius;
        
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float lng = 2 * M_PI * float(j) / slices;
            float x = cos(lng);
            float y = sin(lng);
            
            glNormal3f(x * r0 / radius, y * r0 / radius, z0 / radius);
            glVertex3f(x * r0, y * r0, z0);
            
            glNormal3f(x * r1 / radius, y * r1 / radius, z1 / radius);
            glVertex3f(x * r1, y * r1, z1);
        }
        glEnd();
    }
}

void drawPyramid(float size) {
    float h = size;
    float w = size / 2.0f;
    
    glBegin(GL_TRIANGLES);
    // Front
    glNormal3f(0, 0.707f, 0.707f);
    glVertex3f(0, 0, h);
    glVertex3f(-w, -w, 0);
    glVertex3f(w, -w, 0);
    
    // Right
    glNormal3f(0.707f, 0.707f, 0);
    glVertex3f(0, 0, h);
    glVertex3f(w, -w, 0);
    glVertex3f(w, w, 0);
    
    // Back
    glNormal3f(0, 0.707f, -0.707f);
    glVertex3f(0, 0, h);
    glVertex3f(w, w, 0);
    glVertex3f(-w, w, 0);
    
    // Left
    glNormal3f(-0.707f, 0.707f, 0);
    glVertex3f(0, 0, h);
    glVertex3f(-w, w, 0);
    glVertex3f(-w, -w, 0);
    glEnd();
    
    // Base
    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    glVertex3f(-w, -w, 0);
    glVertex3f(-w, w, 0);
    glVertex3f(w, w, 0);
    glVertex3f(w, -w, 0);
    glEnd();
}

void drawAgent(Agent* agent) {
    glPushMatrix();
    glTranslatef(agent->position.x, agent->position.y, agent->position.z);
    
    Color col = getFactionColor(agent->faction);
    
    switch (agent->role) {
        case Role::SOLDIER:
            col.r *= 0.7f; col.g *= 0.7f; col.b *= 0.7f;
            break;
        case Role::GOVERNMENT:
            col.r *= 1.3f; col.g *= 1.3f; col.b *= 1.3f;
            break;
        case Role::WORKER:
            col.g *= 1.2f;
            break;
        default:
            break;
    }
    
    glColor3f(col.r, col.g, col.b);
    
    switch (agent->role) {
        case Role::SOLDIER:
            drawCube(1.0f);
            break;
        case Role::GOVERNMENT:
            drawPyramid(1.5f);
            break;
        default:
            drawSphere(0.5f);
            break;
    }
    
    // Draw health bar
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, 0, 1.5f);
    glVertex3f(0.5f, 0, 1.5f);
    glVertex3f(0.5f, 0, 1.6f);
    glVertex3f(-0.5f, 0, 1.6f);
    glEnd();
    
    glColor3f(0.0f, 1.0f, 0.0f);
    float healthWidth = agent->health / 100.0f;
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, 0, 1.5f);
    glVertex3f(-0.5f + healthWidth, 0, 1.5f);
    glVertex3f(-0.5f + healthWidth, 0, 1.6f);
    glVertex3f(-0.5f, 0, 1.6f);
    glEnd();
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
}

void drawBuilding(Building* building) {
    glPushMatrix();
    glTranslatef(building->position.x, building->position.y, building->position.z);
    
    Color col = getFactionColor(building->faction);
    glColor3f(col.r * 0.8f, col.g * 0.8f, col.b * 0.8f);
    
    switch (building->type) {
        case Building::HOUSE:
            drawCube(3.0f);
            break;
        case Building::GOVERNMENT_CENTER:
            glScalef(1.0f, 1.0f, 2.0f);
            drawCube(5.0f);
            break;
        case Building::BARRACKS:
            glScalef(2.0f, 1.0f, 1.0f);
            drawCube(4.0f);
            break;
        case Building::FARM:
            glScalef(1.5f, 1.5f, 0.5f);
            drawCube(4.0f);
            break;
        case Building::MINE:
            drawPyramid(3.0f);
            break;
        case Building::STORAGE:
            drawCube(2.5f);
            break;
    }
    
    glPopMatrix();
}

void drawGround() {
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINES);
    for (int i = 0; i <= 100; i += 10) {
        glVertex3f((float)i, 0, 0);
        glVertex3f((float)i, 100, 0);
        glVertex3f(0, (float)i, 0);
        glVertex3f(100, (float)i, 0);
    }
    glEnd();
    
    glColor3f(0.1f, 0.2f, 0.1f);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(0, 0, -0.1f);
    glVertex3f(100, 0, -0.1f);
    glVertex3f(100, 100, -0.1f);
    glVertex3f(0, 100, -0.1f);
    glEnd();
}

void drawText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        // Simple bitmap character rendering (would need proper font in real app)
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // Set camera
    glTranslatef(0, 0, -g_cameraZ);
    glRotatef(g_cameraRotX, 1, 0, 0);
    glRotatef(g_cameraRotY, 0, 0, 1);
    glTranslatef(-g_cameraX, -g_cameraY, 0);
    
    // Draw world
    drawGround();
    
    for (auto& building : g_world->allBuildings) {
        drawBuilding(building.get());
    }
    
    for (auto& agent : g_world->allAgents) {
        if (agent->isAlive()) {
            drawAgent(agent.get());
        }
    }
    
    // Draw simple UI text
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, g_windowWidth, 0, g_windowHeight, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    
    glColor3f(1.0f, 1.0f, 1.0f);
    std::stringstream ss;
    ss << "Generation: " << g_world->currentGeneration << " | ";
    ss << "Speed: " << g_simulationSpeed << "x | ";
    ss << (g_paused ? "PAUSED" : "RUNNING");
    
    // Simple text display (position at top)
    glRasterPos2f(10, g_windowHeight - 20);
    for (char c : ss.str()) {
        // In a real implementation, you'd use a font library here
    }
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    SwapBuffers(g_hDC);
}

void initGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    
    GLfloat lightPos[] = { 50.0f, 50.0f, 100.0f, 1.0f };
    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
}

void resize(int width, int height) {
    g_windowWidth = width;
    g_windowHeight = height;
    
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / height, 0.1, 500.0);
    glMatrixMode(GL_MODELVIEW);
}

void update() {
    if (!g_paused) {
        DWORD currentTime = GetTickCount();
        float deltaTime = (currentTime - g_lastTime) / 1000.0f;
        g_lastTime = currentTime;
        
        g_world->update(deltaTime * g_simulationSpeed);
    } else {
        g_lastTime = GetTickCount();
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            return 0;
            
        case WM_SIZE:
            resize(LOWORD(lParam), HIWORD(lParam));
            return 0;
            
        case WM_KEYDOWN:
            switch (wParam) {
                case VK_SPACE:
                    g_paused = !g_paused;
                    break;
                case '1':
                    g_simulationSpeed = 1.0f;
                    break;
                case '2':
                    g_simulationSpeed = 2.0f;
                    break;
                case '3':
                    g_simulationSpeed = 3.0f;
                    break;
                case '4':
                    g_simulationSpeed = 5.0f;
                    break;
                case '5':
                    g_simulationSpeed = 10.0f;
                    break;
                case 'R':
                    delete g_world;
                    g_world = new World();
                    g_world->initialize();
                    break;
                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;
            }
            return 0;
            
        case WM_LBUTTONDOWN:
            g_mouseDown = true;
            g_lastMouseX = LOWORD(lParam);
            g_lastMouseY = HIWORD(lParam);
            return 0;
            
        case WM_LBUTTONUP:
            g_mouseDown = false;
            return 0;
            
        case WM_MOUSEMOVE:
            if (g_mouseDown) {
                int x = LOWORD(lParam);
                int y = HIWORD(lParam);
                float dx = (float)(x - g_lastMouseX);
                float dy = (float)(y - g_lastMouseY);
                
                g_cameraRotY += dx * 0.5f;
                g_cameraRotX += dy * 0.5f;
                
                if (g_cameraRotX < -89.0f) g_cameraRotX = -89.0f;
                if (g_cameraRotX > 89.0f) g_cameraRotX = 89.0f;
                
                g_lastMouseX = x;
                g_lastMouseY = y;
            }
            return 0;
            
        case WM_MOUSEWHEEL:
            {
                short delta = GET_WHEEL_DELTA_WPARAM(wParam);
                g_cameraZ -= delta / 120.0f * 5.0f;
                if (g_cameraZ < 10.0f) g_cameraZ = 10.0f;
                if (g_cameraZ > 150.0f) g_cameraZ = 150.0f;
            }
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register window class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"NeuralCivilization";
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    
    if (!RegisterClassEx(&wcex)) {
        MessageBox(nullptr, L"Failed to register window class", L"Error", MB_OK);
        return 1;
    }
    
    // Create window
    g_hWnd = CreateWindowEx(
        0,
        L"NeuralCivilization",
        L"Neural Network Civilization Simulation",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        g_windowWidth, g_windowHeight,
        nullptr, nullptr, hInstance, nullptr
    );
    
    if (!g_hWnd) {
        MessageBox(nullptr, L"Failed to create window", L"Error", MB_OK);
        return 1;
    }
    
    // Initialize OpenGL
    g_hDC = GetDC(g_hWnd);
    
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0, 0,
        0, 0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    
    int pixelFormat = ChoosePixelFormat(g_hDC, &pfd);
    SetPixelFormat(g_hDC, pixelFormat, &pfd);
    
    g_hRC = wglCreateContext(g_hDC);
    wglMakeCurrent(g_hDC, g_hRC);
    
    initGL();
    resize(g_windowWidth, g_windowHeight);
    
    // Create and initialize world
    g_world = new World();
    g_world->initialize();
    
    // Show window
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    
    // Print instructions
    std::cout << "Neural Network Civilization Simulation Started" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  SPACE - Pause/Resume" << std::endl;
    std::cout << "  1-5   - Change simulation speed" << std::endl;
    std::cout << "  R     - Reset simulation" << std::endl;
    std::cout << "  Mouse - Rotate camera" << std::endl;
    std::cout << "  Scroll - Zoom in/out" << std::endl;
    std::cout << "  ESC   - Exit" << std::endl;
    
    // Main message loop
    MSG msg = {};
    g_lastTime = GetTickCount();
    
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            update();
            render();
            Sleep(16); // ~60 FPS
        }
    }
    
    // Cleanup
    delete g_world;
    
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);
    
    return (int)msg.wParam;
}*/
