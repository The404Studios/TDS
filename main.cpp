// Enhanced Neural Network Civilization - MULTIPLAYER CLIENT
// Complete with networking, advanced features, and enhanced visualization

#define NOMINMAX // Prevent Windows min/max macros
#define _USE_MATH_DEFINES
#define WIN32_LEAN_AND_MEAN // Prevent windows.h from including winsock.h

// IMPORTANT: Must include winsock2 BEFORE windows.h to avoid conflicts
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <map>
#include <algorithm>
#include <string>
#include <vector>
#include "CivilizationAI.h"
#include "NetworkManager.h"
#include "GameplayFeatures.h"
#include "ExtractionShooter.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper functions to avoid min/max conflicts
template<typename T>
T clamp(T value, T minVal, T maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

template<typename T>
T minVal(T a, T b) {
    return (a < b) ? a : b;
}

template<typename T>
T maxVal(T a, T b) {
    return (a > b) ? a : b;
}

// Color structure
struct Color {
    float r;
    float g;
    float b;
    float a;
    Color(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f)
        : r(red), g(green), b(blue), a(alpha) {
    }
};

// Global variables
World* g_world = nullptr;
std::unique_ptr<ClientNetworkManager> g_networkManager = nullptr;
std::unique_ptr<AdvancedGameManager> g_gameManager = nullptr;
std::unique_ptr<MarketSystem> g_marketSystem = nullptr;

// Extraction Shooter game mode
ExtractionShooter* g_extractionShooter = nullptr;
bool g_useExtractionMode = true;  // Switch between old strategy view and new extraction shooter

HWND g_hWnd = nullptr;
HDC g_hDC = nullptr;
HGLRC g_hRC = nullptr;
int g_windowWidth = 1600;
int g_windowHeight = 900;

// Multiplayer mode flag
bool g_multiplayerMode = false;
bool g_connectingToServer = false;
std::string g_serverAddress = "127.0.0.1";
int g_serverPort = 27015;

// Camera
float g_cameraX = 250.0f;
float g_cameraY = 250.0f;
float g_cameraZ = 150.0f;
float g_cameraRotX = -60.0f;
float g_cameraRotY = 0.0f;

// Mouse
int g_lastMouseX = 0;
int g_lastMouseY = 0;
bool g_leftMouseDown = false;
bool g_rightMouseDown = false;
bool g_middleMouseDown = false;

// Simulation
bool g_paused = false;
float g_simulationSpeed = 1.0f;
DWORD g_lastTime = 0;
bool g_active = true;

// UI
bool g_showDetailedUI = true;
bool g_menuOpen = false;
float g_menuAnimation = 0.0f;
float g_uiAnimationTime = 0.0f;

// Selection
Agent* g_selectedAgent = nullptr;
Building* g_selectedBuilding = nullptr;

// Text rendering - bitmap font
GLuint g_fontBase = 0;

// Role-based building counters
struct RoleStats {
    int citizens;
    int workers;
    int soldiers;
    int governors;
    float lastBuildTime;

    RoleStats() : citizens(0), workers(0), soldiers(0), governors(0), lastBuildTime(0) {}
};
std::map<Faction, RoleStats> g_roleStats;

// Building queue system
struct BuildOrder {
    Building::Type type;
    Vector3 position;
    Faction faction;
    float priority;
    int workersAssigned;
};
std::vector<BuildOrder> g_buildQueue;

// Navigation mesh
struct NavNode {
    Vector3 position;
    std::vector<NavNode*> connections;
    float weight;
    bool visited;
};
std::map<std::pair<int, int>, NavNode> g_navMesh;

// Particles
struct Particle {
    Vector3 position;
    Vector3 velocity;
    float r;
    float g;
    float b;
    float a;
    float lifetime;
    float size;
    int type;
};
std::vector<Particle> g_particles;

// Combat effects
struct CombatEffect {
    Vector3 start;
    Vector3 end;
    float progress;
    float r;
    float g;
    float b;
};
std::vector<CombatEffect> g_combatEffects;

// Forward declarations
void addParticle(Vector3 pos, float r, float g, float b, int type);
void updateParticles(float deltaTime);
void drawParticles();
void addCombatEffect(Vector3 start, Vector3 end, Color col);
void updateCombatEffects(float deltaTime);
void drawCombatEffects();

// Initialize bitmap font for text rendering
void initFont() {
    HFONT font;
    HFONT oldFont;

    g_fontBase = glGenLists(256);

    font = CreateFont(
        -14,                        // Height
        0,                         // Width
        0,                         // Angle
        0,                         // Orientation
        FW_NORMAL,                 // Weight
        FALSE,                     // Italic
        FALSE,                     // Underline
        FALSE,                     // Strikeout
        ANSI_CHARSET,              // Character Set
        OUT_TT_PRECIS,            // Output Precision
        CLIP_DEFAULT_PRECIS,       // Clipping Precision
        ANTIALIASED_QUALITY,       // Quality
        FF_DONTCARE | DEFAULT_PITCH, // Family And Pitch
        L"Arial");                 // Font Name

    oldFont = (HFONT)SelectObject(g_hDC, font);
    wglUseFontBitmaps(g_hDC, 0, 255, g_fontBase);
    SelectObject(g_hDC, oldFont);
    DeleteObject(font);
}

void killFont() {
    if (g_fontBase != 0) {
        glDeleteLists(g_fontBase, 256);
        g_fontBase = 0;
    }
}

void drawText(float x, float y, const std::string& text, Color color = Color(1, 1, 1, 1)) {
    if (g_fontBase == 0) return;

    glColor4f(color.r, color.g, color.b, color.a);
    glRasterPos2f(x, y);

    glPushAttrib(GL_LIST_BIT);
    glListBase(g_fontBase);
    glCallLists(text.length(), GL_UNSIGNED_BYTE, text.c_str());
    glPopAttrib();
}

void drawText3D(float x, float y, float z, const std::string& text, Color color = Color(1, 1, 1, 1)) {
    if (g_fontBase == 0) return;

    glColor4f(color.r, color.g, color.b, color.a);
    glRasterPos3f(x, y, z);

    glPushAttrib(GL_LIST_BIT);
    glListBase(g_fontBase);
    glCallLists(text.length(), GL_UNSIGNED_BYTE, text.c_str());
    glPopAttrib();
}

// Helper functions
Color getFactionColor(Faction f) {
    switch (f) {
    case Faction::RED: return Color(1.0f, 0.2f, 0.2f);
    case Faction::BLUE: return Color(0.2f, 0.4f, 1.0f);
    case Faction::GREEN: return Color(0.2f, 0.9f, 0.3f);
    default: return Color(0.5f, 0.5f, 0.5f);
    }
}

const char* getFactionName(Faction f) {
    switch (f) {
    case Faction::RED: return "Red Empire";
    case Faction::BLUE: return "Blue Kingdom";
    case Faction::GREEN: return "Green Republic";
    default: return "Unknown";
    }
}

const char* getRoleName(Role role) {
    switch (role) {
    case Role::CITIZEN: return "Citizen";
    case Role::WORKER: return "Worker";
    case Role::SOLDIER: return "Soldier";
    case Role::GOVERNMENT: return "Governor";
    default: return "Unknown";
    }
}

const char* getBuildingName(Building::Type type) {
    switch (type) {
    case Building::HOUSE: return "House";
    case Building::FARM: return "Farm";
    case Building::MINE: return "Mine";
    case Building::BARRACKS: return "Barracks";
    case Building::GOVERNMENT_CENTER: return "Gov Center";
    case Building::STORAGE: return "Storage";
    default: return "Unknown";
    }
}

// Update role statistics
void updateRoleStats() {
    // Clear stats
    for (auto& kvp : g_roleStats) {
        kvp.second.citizens = 0;
        kvp.second.workers = 0;
        kvp.second.soldiers = 0;
        kvp.second.governors = 0;
    }

    // Count roles per faction
    for (auto& agent : g_world->allAgents) {
        if (agent->isAlive()) {
            RoleStats& stats = g_roleStats[agent->faction];
            switch (agent->role) {
            case Role::CITIZEN: stats.citizens++; break;
            case Role::WORKER: stats.workers++; break;
            case Role::SOLDIER: stats.soldiers++; break;
            case Role::GOVERNMENT: stats.governors++; break;
            }
        }
    }
}

// Decide what to build based on role distribution
Building::Type decideBuilding(Faction faction) {
    RoleStats& stats = g_roleStats[faction];
    int total = stats.citizens + stats.workers + stats.soldiers + stats.governors;

    if (total == 0) return Building::HOUSE;

    float workerRatio = (float)stats.workers / total;
    float soldierRatio = (float)stats.soldiers / total;
    float citizenRatio = (float)stats.citizens / total;

    // Building decision logic based on role ratios
    if (citizenRatio > 0.5f) {
        // Too many citizens, need jobs
        if (rand() % 2 == 0) return Building::FARM;
        else return Building::MINE;
    }

    if (workerRatio > 0.4f && rand() % 100 < 30) {
        // Many workers, need storage
        return Building::STORAGE;
    }

    if (soldierRatio < 0.2f && rand() % 100 < 40) {
        // Need more military
        return Building::BARRACKS;
    }

    if (stats.governors < 2 && rand() % 100 < 10) {
        // Need government
        return Building::GOVERNMENT_CENTER;
    }

    // Default to houses for population growth
    return Building::HOUSE;
}

// Find good building location
Vector3 findBuildingLocation(Faction faction) {
    // Calculate faction center
    Vector3 center(0, 0, 0);
    int count = 0;

    for (auto& building : g_world->allBuildings) {
        if (building->faction == faction) {
            center = center + building->position;
            count++;
        }
    }

    if (count > 0) {
        center = center * (1.0f / count);
    }
    else {
        // Default positions for each faction
        switch (faction) {
        case Faction::RED: center = Vector3(100, 100, 0); break;
        case Faction::BLUE: center = Vector3(400, 100, 0); break;
        case Faction::GREEN: center = Vector3(250, 400, 0); break;
        }
    }

    // Try to find clear spot
    for (int attempt = 0; attempt < 10; attempt++) {
        float angle = (rand() % 360) * M_PI / 180.0f;
        float distance = 20.0f + (rand() % 60);

        Vector3 pos;
        pos.x = center.x + cos(angle) * distance;
        pos.y = center.y + sin(angle) * distance;
        pos.z = 0;

        // Bounds check
        pos.x = clamp(pos.x, 30.0f, 470.0f);
        pos.y = clamp(pos.y, 30.0f, 470.0f);

        // Check if clear
        bool clear = true;
        for (auto& building : g_world->allBuildings) {
            if ((building->position - pos).length() < 25.0f) {
                clear = false;
                break;
            }
        }

        if (clear) return pos;
    }

    return center + Vector3(rand() % 40 - 20, rand() % 40 - 20, 0);
}

// Automatic building expansion based on roles
void processAutomaticExpansion(float deltaTime) {
    static float expansionCheckTimer = 0;
    expansionCheckTimer += deltaTime;

    if (expansionCheckTimer < 10.0f) return; // Check every 10 seconds
    expansionCheckTimer = 0;

    updateRoleStats();

    for (auto& faction : g_world->factions) {
        RoleStats& stats = g_roleStats[faction->faction];

        // Calculate if we should build
        int totalPop = faction->agents.size();
        int totalBuildings = faction->buildings.size();

        // Build if we have resources and reasonable pop/building ratio
        int totalResources = 0;
        for (int r : faction->resources) {
            totalResources += r;
        }

        bool shouldBuild = false;
        Building::Type buildType = Building::HOUSE;

        // Different building triggers based on role distribution
        if (totalResources > 200) {
            if (totalPop > totalBuildings * 3) {
                // Need more buildings for population
                shouldBuild = true;
                buildType = Building::HOUSE;
            }
            else if (stats.workers > stats.citizens * 2 && rand() % 100 < 50) {
                // Workers need storage/workplace
                shouldBuild = true;
                buildType = (rand() % 2) ? Building::STORAGE : Building::FARM;
            }
            else if (stats.soldiers > 5 && totalBuildings < 15) {
                // Military expansion
                shouldBuild = true;
                buildType = Building::BARRACKS;
            }
            else if (rand() % 100 < 20) {
                // Random expansion based on needs
                shouldBuild = true;
                buildType = decideBuilding(faction->faction);
            }
        }

        if (shouldBuild) {
            Vector3 buildPos = findBuildingLocation(faction->faction);

            // Check resource cost
            bool canAfford = false;
            switch (buildType) {
            case Building::HOUSE:
                canAfford = faction->resources[1] >= 30; // Wood
                if (canAfford) faction->resources[1] -= 30;
                break;
            case Building::FARM:
                canAfford = faction->resources[1] >= 50; // Wood
                if (canAfford) faction->resources[1] -= 50;
                break;
            case Building::BARRACKS:
                canAfford = faction->resources[2] >= 100; // Stone
                if (canAfford) faction->resources[2] -= 100;
                break;
            case Building::STORAGE:
                canAfford = faction->resources[1] >= 75; // Wood
                if (canAfford) faction->resources[1] -= 75;
                break;
            case Building::MINE:
                canAfford = faction->resources[1] >= 60; // Wood
                if (canAfford) faction->resources[1] -= 60;
                break;
            case Building::GOVERNMENT_CENTER:
                canAfford = faction->resources[2] >= 200; // Stone
                if (canAfford) faction->resources[2] -= 200;
                break;
            }

            if (canAfford) {
                // Create building
                auto newBuilding = std::make_unique<Building>(buildType, buildPos, faction->faction);
                faction->buildings.push_back(newBuilding.get());
                g_world->allBuildings.push_back(std::move(newBuilding));

                // Visual feedback
                for (int i = 0; i < 30; ++i) {
                    addParticle(buildPos + Vector3(0, 0, 2),
                        0.8f, 0.6f, 0.3f, 1); // Construction dust
                }

                // Assign workers
                int workersNeeded = 2 + rand() % 3;
                int assigned = 0;
                for (Agent* agent : faction->agents) {
                    if (agent->role == Role::WORKER && assigned < workersNeeded) {
                        agent->targetPosition = buildPos;
                        assigned++;
                    }
                }

                std::cout << getFactionName(faction->faction) << " auto-built "
                    << getBuildingName(buildType)
                    << " (Workers:" << stats.workers
                    << " Soldiers:" << stats.soldiers
                    << " Citizens:" << stats.citizens << ")" << std::endl;
            }
        }
    }
}

// Particle system
void addParticle(Vector3 pos, float r, float g, float b, int type) {
    Particle p;
    p.position = pos;
    p.r = r; p.g = g; p.b = b; p.a = 1.0f;
    p.type = type;
    p.lifetime = 2.0f;

    switch (type) {
    case 0: // Combat
        p.velocity = Vector3(
            (rand() % 100 - 50) / 25.0f,
            (rand() % 100 - 50) / 25.0f,
            (rand() % 100) / 10.0f
        );
        p.size = 3.0f;
        break;
    case 1: // Resource/Construction
        p.velocity = Vector3(
            (rand() % 100 - 50) / 50.0f,
            (rand() % 100 - 50) / 50.0f,
            3.0f
        );
        p.size = 2.0f;
        break;
    case 2: // Birth
        p.velocity = Vector3(
            (rand() % 100 - 50) / 50.0f,
            (rand() % 100 - 50) / 50.0f,
            5.0f
        );
        p.size = 4.0f;
        break;
    }

    g_particles.push_back(p);
}

void updateParticles(float deltaTime) {
    for (auto it = g_particles.begin(); it != g_particles.end();) {
        it->lifetime -= deltaTime;
        it->position = it->position + it->velocity * deltaTime;
        it->a = it->lifetime / 2.0f;

        if (it->type == 0) {
            it->velocity.z -= 9.8f * deltaTime;
            if (it->position.z < 0) {
                it->position.z = 0;
                it->velocity.z *= -0.5f;
            }
        }

        if (it->lifetime <= 0) {
            it = g_particles.erase(it);
        }
        else {
            ++it;
        }
    }
}

void drawParticles() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (const auto& p : g_particles) {
        glPointSize(p.size);
        glBegin(GL_POINTS);
        glColor4f(p.r, p.g, p.b, p.a);
        glVertex3f(p.position.x, p.position.y, p.position.z);
        glEnd();
    }

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// Combat effects
void addCombatEffect(Vector3 start, Vector3 end, Color col) {
    CombatEffect effect;
    effect.start = start;
    effect.end = end;
    effect.progress = 0.0f;
    effect.r = col.r;
    effect.g = col.g;
    effect.b = col.b;
    g_combatEffects.push_back(effect);
}

void updateCombatEffects(float deltaTime) {
    for (auto it = g_combatEffects.begin(); it != g_combatEffects.end();) {
        it->progress += deltaTime * 3.0f;

        if (it->progress >= 1.0f) {
            for (int i = 0; i < 8; ++i) {
                addParticle(it->end, it->r, it->g, it->b, 0);
            }
            it = g_combatEffects.erase(it);
        }
        else {
            ++it;
        }
    }
}

void drawCombatEffects() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (const auto& effect : g_combatEffects) {
        Vector3 current = effect.start + (effect.end - effect.start) * effect.progress;

        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glColor4f(effect.r, effect.g, effect.b, 1.0f);
        glVertex3f(current.x, current.y, current.z + 1.0f);
        glColor4f(effect.r, effect.g, effect.b, 0.0f);
        Vector3 trail = current - (effect.end - effect.start).normalize() * 5.0f;
        glVertex3f(trail.x, trail.y, trail.z + 1.0f);
        glEnd();

        glPointSize(6.0f);
        glBegin(GL_POINTS);
        glColor3f(effect.r, effect.g, effect.b);
        glVertex3f(current.x, current.y, current.z + 1.0f);
        glEnd();
    }

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// Drawing functions
void drawCube(float size) {
    float h = size / 2.0f;

    glBegin(GL_QUADS);
    // Bottom
    glNormal3f(0, 0, -1);
    glVertex3f(-h, -h, 0);
    glVertex3f(h, -h, 0);
    glVertex3f(h, h, 0);
    glVertex3f(-h, h, 0);

    // Top
    glNormal3f(0, 0, 1);
    glVertex3f(-h, -h, size);
    glVertex3f(-h, h, size);
    glVertex3f(h, h, size);
    glVertex3f(h, -h, size);

    // Front
    glNormal3f(0, -1, 0);
    glVertex3f(-h, -h, 0);
    glVertex3f(h, -h, 0);
    glVertex3f(h, -h, size);
    glVertex3f(-h, -h, size);

    // Back
    glNormal3f(0, 1, 0);
    glVertex3f(-h, h, 0);
    glVertex3f(-h, h, size);
    glVertex3f(h, h, size);
    glVertex3f(h, h, 0);

    // Right
    glNormal3f(1, 0, 0);
    glVertex3f(h, -h, 0);
    glVertex3f(h, h, 0);
    glVertex3f(h, h, size);
    glVertex3f(h, -h, size);

    // Left
    glNormal3f(-1, 0, 0);
    glVertex3f(-h, -h, 0);
    glVertex3f(-h, -h, size);
    glVertex3f(-h, h, size);
    glVertex3f(-h, h, 0);
    glEnd();
}

void drawSphere(float radius, int slices = 12, int stacks = 12) {
    for (int i = 0; i < stacks; ++i) {
        float lat0 = M_PI * (-0.5f + (float)i / stacks);
        float z0 = sin(lat0) * radius;
        float r0 = cos(lat0) * radius;

        float lat1 = M_PI * (-0.5f + (float)(i + 1) / stacks);
        float z1 = sin(lat1) * radius;
        float r1 = cos(lat1) * radius;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float lng = 2 * M_PI * (float)j / slices;
            float x = cos(lng);
            float y = sin(lng);

            glNormal3f(x * r0 / radius, y * r0 / radius, z0 / radius);
            glVertex3f(x * r0, y * r0, z0 + radius);

            glNormal3f(x * r1 / radius, y * r1 / radius, z1 / radius);
            glVertex3f(x * r1, y * r1, z1 + radius);
        }
        glEnd();
    }
}

void drawPyramid(float size) {
    float h = size * 1.5f;
    float w = size / 2.0f;

    glBegin(GL_TRIANGLES);
    glNormal3f(0, -0.707f, 0.707f);
    glVertex3f(0, 0, h);
    glVertex3f(-w, -w, 0);
    glVertex3f(w, -w, 0);

    glNormal3f(0.707f, 0, 0.707f);
    glVertex3f(0, 0, h);
    glVertex3f(w, -w, 0);
    glVertex3f(w, w, 0);

    glNormal3f(0, 0.707f, 0.707f);
    glVertex3f(0, 0, h);
    glVertex3f(w, w, 0);
    glVertex3f(-w, w, 0);

    glNormal3f(-0.707f, 0, 0.707f);
    glVertex3f(0, 0, h);
    glVertex3f(-w, w, 0);
    glVertex3f(-w, -w, 0);
    glEnd();

    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    glVertex3f(-w, -w, 0);
    glVertex3f(-w, w, 0);
    glVertex3f(w, w, 0);
    glVertex3f(w, -w, 0);
    glEnd();
}

void drawAgent(Agent* agent) {
    if (!agent || !agent->isAlive()) return;

    glPushMatrix();
    glTranslatef(agent->position.x, agent->position.y, 0);

    // Selection highlight
    if (agent == g_selectedAgent) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 16; ++i) {
            float angle = i * 2 * M_PI / 16;
            glVertex3f(cos(angle) * 2.0f, sin(angle) * 2.0f, 0.1f);
        }
        glEnd();
        glEnable(GL_LIGHTING);
    }

    Color col = getFactionColor(agent->faction);

    switch (agent->role) {
    case Role::SOLDIER:
        col.r *= 0.7f; col.g *= 0.7f; col.b *= 0.7f;
        break;
    case Role::GOVERNMENT:
        col.r = minVal(1.0f, col.r * 1.3f);
        col.g = minVal(1.0f, col.g * 1.3f);
        col.b = minVal(1.0f, col.b * 1.3f);
        break;
    case Role::WORKER:
        col.g = minVal(1.0f, col.g * 1.2f);
        break;
    }

    glColor3f(col.r, col.g, col.b);

    float bobHeight = sin(g_uiAnimationTime * 3.0f + agent->age * 0.1f) * 0.2f;
    glTranslatef(0, 0, bobHeight);

    switch (agent->role) {
    case Role::SOLDIER:
        drawCube(1.2f);
        break;
    case Role::GOVERNMENT:
        drawPyramid(1.0f);
        break;
    default:
        drawSphere(0.5f);
        break;
    }

    // Draw status above agent
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Health bar
    glColor3f(0.2f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(-0.6f, -0.6f, 2.0f);
    glVertex3f(0.6f, -0.6f, 2.0f);
    glVertex3f(0.6f, -0.4f, 2.0f);
    glVertex3f(-0.6f, -0.4f, 2.0f);
    glEnd();

    float healthRatio = agent->health / 100.0f;
    if (healthRatio > 0.5f)
        glColor3f(0.0f, 1.0f, 0.0f);
    else if (healthRatio > 0.25f)
        glColor3f(1.0f, 1.0f, 0.0f);
    else
        glColor3f(1.0f, 0.0f, 0.0f);

    glBegin(GL_QUADS);
    glVertex3f(-0.6f, -0.6f, 2.0f);
    glVertex3f(-0.6f + healthRatio * 1.2f, -0.6f, 2.0f);
    glVertex3f(-0.6f + healthRatio * 1.2f, -0.4f, 2.0f);
    glVertex3f(-0.6f, -0.4f, 2.0f);
    glEnd();

    // Role text
    drawText3D(-1.0f, -1.0f, 2.5f, getRoleName(agent->role), Color(1, 1, 1, 0.8f));

    // Resource indicator
    if (!agent->getResourceText().empty()) {
        drawText3D(-1.0f, -1.0f, 3.0f, agent->getResourceText(), Color(1, 0.8f, 0, 1));
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void drawBuilding(Building* building) {
    if (!building) return;

    glPushMatrix();
    glTranslatef(building->position.x, building->position.y, 0);

    if (building == g_selectedBuilding) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex3f(-4, -4, 0.1f);
        glVertex3f(4, -4, 0.1f);
        glVertex3f(4, 4, 0.1f);
        glVertex3f(-4, 4, 0.1f);
        glEnd();
        glEnable(GL_LIGHTING);
    }

    Color col = getFactionColor(building->faction);
    float healthRatio = building->health / 100.0f;
    glColor3f(col.r * healthRatio, col.g * healthRatio, col.b * healthRatio);

    switch (building->type) {
    case Building::HOUSE:
        drawCube(3.0f);
        glColor3f(col.r * 0.6f, col.g * 0.6f, col.b * 0.6f);
        glTranslatef(0, 0, 3.0f);
        drawPyramid(2.0f);
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
        drawCube(2.0f);
        break;
    case Building::MINE:
        drawPyramid(3.0f);
        break;
    case Building::STORAGE:
        drawCube(2.5f);
        break;
    }

    // Draw building label
    glDisable(GL_LIGHTING);
    drawText3D(-3.0f, -3.0f, 6.0f, getBuildingName(building->type),
        Color(1, 1, 1, 0.8f));
    glEnable(GL_LIGHTING);

    glPopMatrix();
}

void drawGround() {
    glDisable(GL_LIGHTING);
    glLineWidth(1.0f);

    glColor3f(0.2f, 0.3f, 0.2f);
    glBegin(GL_LINES);
    for (int i = 0; i <= 500; i += 25) {
        glVertex3f((float)i, 0, 0);
        glVertex3f((float)i, 500, 0);
        glVertex3f(0, (float)i, 0);
        glVertex3f(500, (float)i, 0);
    }
    glEnd();

    glLineWidth(2.0f);
    glColor3f(0.3f, 0.4f, 0.3f);
    glBegin(GL_LINES);
    for (int i = 0; i <= 500; i += 100) {
        glVertex3f((float)i, 0, 0);
        glVertex3f((float)i, 500, 0);
        glVertex3f(0, (float)i, 0);
        glVertex3f(500, (float)i, 0);
    }
    glEnd();

    glEnable(GL_LIGHTING);

    glColor3f(0.15f, 0.25f, 0.15f);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(0, 0, -0.1f);
    glVertex3f(500, 0, -0.1f);
    glVertex3f(500, 500, -0.1f);
    glVertex3f(0, 500, -0.1f);
    glEnd();
}

void drawUI() {
    g_uiAnimationTime += 0.016f;

    if (g_menuOpen) {
        g_menuAnimation = minVal(1.0f, g_menuAnimation + 0.1f);
    }
    else {
        g_menuAnimation = maxVal(0.0f, g_menuAnimation - 0.1f);
    }

    // 2D mode
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, g_windowWidth, g_windowHeight, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Always show top info bar
    glColor4f(0.1f, 0.1f, 0.2f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(g_windowWidth, 0);
    glVertex2f(g_windowWidth, 40);
    glVertex2f(0, 40);
    glEnd();

    // Top bar info
    std::stringstream ss;
    ss << "Generation: " << g_world->currentGeneration
        << " | Speed: " << (int)g_simulationSpeed << "x | "
        << (g_paused ? "PAUSED" : "RUNNING");

    // Add multiplayer status
    if (g_multiplayerMode && g_networkManager) {
        if (g_networkManager->isConnected()) {
            ss << " | ONLINE (Ping: " << (int)g_networkManager->getPing() << "ms)";
        } else {
            ss << " | OFFLINE (Connecting...)";
        }
    } else {
        ss << " | SINGLEPLAYER";
    }

    drawText(10, 25, ss.str(), Color(1, 1, 1, 1));

    // Weather and Season info
    if (g_gameManager) {
        ss.str("");
        const char* weatherNames[] = {"Clear", "Rain", "Storm", "Snow", "Fog", "Drought"};
        const char* seasonNames[] = {"Spring", "Summer", "Fall", "Winter"};

        ss << "Weather: " << weatherNames[(int)g_gameManager->getCurrentWeather().type]
           << " | Season: " << seasonNames[(int)g_gameManager->getCurrentSeason()];
        drawText(g_windowWidth - 400, 25, ss.str(), Color(0.8f, 0.9f, 1.0f, 1));
    }

    // Faction quick stats
    float xPos = 300;
    for (int i = 0; i < 3; ++i) {
        Color col = getFactionColor(static_cast<Faction>(i));
        ss.str("");
        ss << getFactionName(static_cast<Faction>(i)) << ": "
            << g_world->factions[i]->agents.size() << " agents";
        drawText(xPos, 25, ss.str(), col);
        xPos += 200;
    }

    // Detailed menu
    if (g_menuAnimation > 0.01f) {
        float slideX = (1.0f - g_menuAnimation) * -450;

        // Main panel
        glColor4f(0.1f, 0.1f, 0.2f, 0.95f * g_menuAnimation);
        glBegin(GL_QUADS);
        glVertex2f(slideX + 10, 50);
        glVertex2f(slideX + 450, 50);
        glVertex2f(slideX + 450, 600);
        glVertex2f(slideX + 10, 600);
        glEnd();

        // Title
        drawText(slideX + 20, 70, "CIVILIZATION CONTROL PANEL",
            Color(1, 1, 1, g_menuAnimation));

        // Faction details
        float yPos = 100;
        for (int i = 0; i < 3; ++i) {
            Color col = getFactionColor(static_cast<Faction>(i));
            RoleStats& stats = g_roleStats[static_cast<Faction>(i)];

            // Faction header
            glColor4f(col.r * 0.3f, col.g * 0.3f, col.b * 0.3f, 0.5f * g_menuAnimation);
            glBegin(GL_QUADS);
            glVertex2f(slideX + 20, yPos);
            glVertex2f(slideX + 430, yPos);
            glVertex2f(slideX + 430, yPos + 100);
            glVertex2f(slideX + 20, yPos + 100);
            glEnd();

            drawText(slideX + 30, yPos + 20, getFactionName(static_cast<Faction>(i)), col);

            ss.str("");
            ss << "Population: " << g_world->factions[i]->agents.size()
                << " | Buildings: " << g_world->factions[i]->buildings.size();
            drawText(slideX + 30, yPos + 40, ss.str(), Color(1, 1, 1, g_menuAnimation));

            ss.str("");
            ss << "Citizens: " << stats.citizens
                << " Workers: " << stats.workers
                << " Soldiers: " << stats.soldiers
                << " Governors: " << stats.governors;
            drawText(slideX + 30, yPos + 55, ss.str(), Color(0.9f, 0.9f, 0.9f, g_menuAnimation));

            ss.str("");
            ss << "Resources - Food:" << g_world->factions[i]->resources[0]
                << " Wood:" << g_world->factions[i]->resources[1]
                << " Stone:" << g_world->factions[i]->resources[2]
                << " Gold:" << g_world->factions[i]->resources[3];
            drawText(slideX + 30, yPos + 70, ss.str(), Color(0.8f, 0.8f, 0.6f, g_menuAnimation));

            // Expansion readiness
            int totalRes = 0;
            for (int r : g_world->factions[i]->resources) totalRes += r;
            if (totalRes > 200) {
                drawText(slideX + 30, yPos + 85, "READY TO EXPAND!",
                    Color(0, 1, 0, g_menuAnimation));
            }

            yPos += 110;
        }

        // Selected entity info
        if (g_selectedAgent && g_selectedAgent->isAlive()) {
            glColor4f(0.3f, 0.3f, 0.1f, 0.7f * g_menuAnimation);
            glBegin(GL_QUADS);
            glVertex2f(slideX + 20, 440);
            glVertex2f(slideX + 430, 440);
            glVertex2f(slideX + 430, 580);
            glVertex2f(slideX + 20, 580);
            glEnd();

            drawText(slideX + 30, 460, "SELECTED AGENT", Color(1, 1, 0, g_menuAnimation));

            ss.str("");
            ss << "Faction: " << getFactionName(g_selectedAgent->faction)
                << " | Role: " << getRoleName(g_selectedAgent->role);
            drawText(slideX + 30, 480, ss.str());

            ss.str("");
            ss << "Health: " << (int)g_selectedAgent->health
                << "/100 | Energy: " << (int)g_selectedAgent->energy << "/100";
            drawText(slideX + 30, 500, ss.str());

            ss.str("");
            ss << "Age: " << g_selectedAgent->age
                << " | Kills: " << g_selectedAgent->kills;
            drawText(slideX + 30, 520, ss.str());

            if (!g_selectedAgent->getResourceText().empty()) {
                ss.str("");
                ss << "Carrying: " << g_selectedAgent->getResourceText();
                drawText(slideX + 30, 540, ss.str());
            }

            drawText(slideX + 30, 560, "[C] Change Role | [D] Delete | [M] Move",
                Color(0.7f, 0.7f, 0.7f, g_menuAnimation));
        }
        else if (g_selectedBuilding) {
            glColor4f(0.3f, 0.2f, 0.1f, 0.7f * g_menuAnimation);
            glBegin(GL_QUADS);
            glVertex2f(slideX + 20, 440);
            glVertex2f(slideX + 430, 440);
            glVertex2f(slideX + 430, 540);
            glVertex2f(slideX + 20, 540);
            glEnd();

            drawText(slideX + 30, 460, "SELECTED BUILDING", Color(1, 1, 0, g_menuAnimation));

            ss.str("");
            ss << "Type: " << getBuildingName(g_selectedBuilding->type)
                << " | Faction: " << getFactionName(g_selectedBuilding->faction);
            drawText(slideX + 30, 480, ss.str());

            ss.str("");
            ss << "Health: " << (int)g_selectedBuilding->health << "/100"
                << " | Occupancy: " << g_selectedBuilding->occupants
                << "/" << g_selectedBuilding->capacity;
            drawText(slideX + 30, 500, ss.str());

            ss.str("");
            ss << "Position: (" << (int)g_selectedBuilding->position.x
                << ", " << (int)g_selectedBuilding->position.y << ")";
            drawText(slideX + 30, 520, ss.str());
        }
    }

    // Controls hint (bottom right)
    drawText(g_windowWidth - 350, g_windowHeight - 80,
        "TAB: Menu | Click: Select | Ctrl+Drag: Rotate",
        Color(0.7f, 0.7f, 0.7f, 0.8f));
    drawText(g_windowWidth - 350, g_windowHeight - 60,
        "Right/Middle: Pan | Wheel: Zoom | C: Change Role",
        Color(0.7f, 0.7f, 0.7f, 0.8f));
    drawText(g_windowWidth - 350, g_windowHeight - 40,
        "SPACE: Pause | 1-5: Speed | R: Reset",
        Color(0.7f, 0.7f, 0.7f, 0.8f));
    drawText(g_windowWidth - 350, g_windowHeight - 20,
        "N: Toggle Multiplayer | ESC: Exit",
        Color(0.7f, 0.7f, 0.7f, 0.8f));

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void render() {
    if (g_useExtractionMode && g_extractionShooter) {
        // Use new extraction shooter rendering
        g_extractionShooter->render();
        SwapBuffers(g_hDC);
    }
    else {
        // Old strategy view rendering
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        gluLookAt(
            g_cameraX - sin(g_cameraRotY * M_PI / 180) * cos(g_cameraRotX * M_PI / 180) * g_cameraZ,
            g_cameraY - cos(g_cameraRotY * M_PI / 180) * cos(g_cameraRotX * M_PI / 180) * g_cameraZ,
            g_cameraZ + sin(g_cameraRotX * M_PI / 180) * g_cameraZ,
            g_cameraX, g_cameraY, 0,
            0, 0, 1
        );

        drawGround();

        for (auto& building : g_world->allBuildings) {
            drawBuilding(building.get());
        }

        for (auto& agent : g_world->allAgents) {
            if (agent->isAlive()) {
                drawAgent(agent.get());
            }
        }

        drawCombatEffects();
        drawParticles();
        drawUI();

        SwapBuffers(g_hDC);
    }
}

void initOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    GLfloat light0Pos[] = { 250.0f, 250.0f, 300.0f, 1.0f };
    GLfloat light0Amb[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat light0Dif[] = { 0.8f, 0.8f, 0.7f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0Amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Dif);

    glClearColor(0.05f, 0.05f, 0.15f, 1.0f);
}

void resize(int width, int height) {
    if (height == 0) height = 1;

    g_windowWidth = width;
    g_windowHeight = height;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)width / (double)height, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

void selectObject(int x, int y) {
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble objX, objY, objZ;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels(x, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

    gluUnProject(winX, winY, winZ, modelview, projection, viewport, &objX, &objY, &objZ);

    g_selectedAgent = nullptr;
    g_selectedBuilding = nullptr;

    float minDist = 10.0f;

    for (auto& agent : g_world->allAgents) {
        if (agent->isAlive()) {
            float dist = sqrt(pow(agent->position.x - objX, 2) + pow(agent->position.y - objY, 2));
            if (dist < minDist) {
                minDist = dist;
                g_selectedAgent = agent.get();
            }
        }
    }

    if (!g_selectedAgent) {
        minDist = 15.0f;
        for (auto& building : g_world->allBuildings) {
            float dist = sqrt(pow(building->position.x - objX, 2) + pow(building->position.y - objY, 2));
            if (dist < minDist) {
                minDist = dist;
                g_selectedBuilding = building.get();
            }
        }
    }
}

void update() {
    DWORD currentTime = GetTickCount();
    float deltaTime = (currentTime - g_lastTime) / 1000.0f;
    if (deltaTime > 0.1f) deltaTime = 0.1f;
    g_lastTime = currentTime;

    if (g_useExtractionMode && g_extractionShooter) {
        // Use new extraction shooter update
        g_extractionShooter->update(deltaTime);
    }
    else if (!g_paused && g_active) {
        // Old strategy view update

        // Update networking if in multiplayer mode
        if (g_multiplayerMode && g_networkManager) {
            g_networkManager->update(deltaTime);

            // Process incoming packets from server
            while (g_networkManager->hasPackets()) {
                NetworkPacket packet = g_networkManager->getNextPacket();
                // Handle packet (world state updates, etc.)
            }

            // Try to connect if not connected
            if (!g_networkManager->isConnected() && !g_connectingToServer) {
                g_connectingToServer = true;
                if (g_networkManager->connectToServer()) {
                    std::cout << "Connected to server!" << std::endl;
                }
                g_connectingToServer = false;
            }
        }

        g_world->update(deltaTime * g_simulationSpeed);

        // Update advanced game systems
        if (g_gameManager) {
            g_gameManager->update(deltaTime, g_world);
        }

        if (g_marketSystem) {
            g_marketSystem->updatePrices(deltaTime);
        }

        updateParticles(deltaTime);
        updateCombatEffects(deltaTime);
        processAutomaticExpansion(deltaTime);

        // Combat visualization
        for (auto& agent : g_world->allAgents) {
            if (agent->role == Role::SOLDIER && agent->targetEnemy && agent->isAlive()) {
                float dist = (agent->position - agent->targetEnemy->position).length();
                if (dist < 15.0f && rand() % 60 == 0) {
                    addCombatEffect(
                        agent->position + Vector3(0, 0, 1),
                        agent->targetEnemy->position + Vector3(0, 0, 1),
                        getFactionColor(agent->faction)
                    );
                }
            }
        }

        g_lastTime = currentTime;
    }
    else {
        g_lastTime = GetTickCount();
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_SIZE:
        resize(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_KEYDOWN:
        // Forward to extraction shooter if active
        if (g_useExtractionMode && g_extractionShooter) {
            unsigned char key = (unsigned char)wParam;
            g_extractionShooter->handleKeyPress(key, true);
        }
        else {
            // Old strategy view controls
            switch (wParam) {
            case VK_SPACE:
                g_paused = !g_paused;
                break;
            case VK_TAB:
                g_menuOpen = !g_menuOpen;
                break;
            case '1': g_simulationSpeed = 1.0f; break;
            case '2': g_simulationSpeed = 2.0f; break;
            case '3': g_simulationSpeed = 3.0f; break;
            case '4': g_simulationSpeed = 5.0f; break;
            case '5': g_simulationSpeed = 10.0f; break;
            case 'C':
            case 'c':
                if (g_selectedAgent) {
                    int newRole = ((int)g_selectedAgent->role + 1) % (int)Role::COUNT;
                    g_selectedAgent->changeRole((Role)newRole);
                    updateRoleStats();
                }
                break;
            case 'R':
            case 'r':
                delete g_world;
                g_world = new World();
                g_world->WORLD_SIZE = 500;
                g_world->MAX_AGENTS_PER_FACTION = 100;
                g_world->initialize();
                g_selectedAgent = nullptr;
                g_selectedBuilding = nullptr;
                g_roleStats.clear();
                break;
            case 'N':
            case 'n':
                // Toggle multiplayer mode
                g_multiplayerMode = !g_multiplayerMode;
                if (g_multiplayerMode && !g_networkManager) {
                    g_networkManager = std::make_unique<ClientNetworkManager>(g_serverAddress, g_serverPort);
                    g_networkManager->initialize();
                    std::cout << "Multiplayer mode ENABLED. Connecting to server..." << std::endl;
                } else if (!g_multiplayerMode) {
                    g_networkManager.reset();
                    std::cout << "Multiplayer mode DISABLED" << std::endl;
                }
                break;
            case VK_ESCAPE:
                PostQuitMessage(0);
                break;
            }
        }
        return 0;

    case WM_KEYUP:
        // Forward to extraction shooter if active
        if (g_useExtractionMode && g_extractionShooter) {
            unsigned char key = (unsigned char)wParam;
            g_extractionShooter->handleKeyPress(key, false);
        }
        return 0;

    case WM_LBUTTONDOWN:
        g_leftMouseDown = true;
        g_lastMouseX = LOWORD(lParam);
        g_lastMouseY = HIWORD(lParam);

        if (!(GetKeyState(VK_CONTROL) & 0x8000)) {
            selectObject(g_lastMouseX, g_lastMouseY);
        }

        SetCapture(hWnd);
        return 0;

    case WM_RBUTTONDOWN:
        g_rightMouseDown = true;
        g_lastMouseX = LOWORD(lParam);
        g_lastMouseY = HIWORD(lParam);
        SetCapture(hWnd);
        return 0;

    case WM_LBUTTONUP:
        g_leftMouseDown = false;
        if (!g_rightMouseDown) ReleaseCapture();
        return 0;

    case WM_RBUTTONUP:
        g_rightMouseDown = false;
        if (!g_leftMouseDown) ReleaseCapture();
        return 0;

    case WM_MOUSEMOVE:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        // Forward to extraction shooter if active
        if (g_useExtractionMode && g_extractionShooter) {
            g_extractionShooter->handleMouseMove(x, y);
        }
        else {
            // Old strategy view mouse controls
            float dx = (float)(x - g_lastMouseX);
            float dy = (float)(y - g_lastMouseY);

            if (g_leftMouseDown && (GetKeyState(VK_CONTROL) & 0x8000)) {
                g_cameraRotY += dx * 0.5f;
                g_cameraRotX -= dy * 0.5f;
                g_cameraRotX = clamp(g_cameraRotX, -89.0f, 89.0f);
            }
            else if (g_rightMouseDown) {
                float panSpeed = g_cameraZ / 100.0f;
                g_cameraX -= dx * panSpeed;
                g_cameraY += dy * panSpeed;
                g_cameraX = clamp(g_cameraX, 0.0f, 500.0f);
                g_cameraY = clamp(g_cameraY, 0.0f, 500.0f);
            }
        }

        g_lastMouseX = x;
        g_lastMouseY = y;
    }
    return 0;

    case WM_MOUSEWHEEL:
    {
        short delta = GET_WHEEL_DELTA_WPARAM(wParam);
        g_cameraZ -= (delta / 120.0f) * 15.0f;
        g_cameraZ = clamp(g_cameraZ, 30.0f, 300.0f);
    }
    return 0;

    case WM_DESTROY:
        killFont();
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

int main(int argc, char* argv[]) {
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    std::cout << "========================================================" << std::endl;
    std::cout << " NEURAL CIVILIZATION - MULTIPLAYER CLIENT v2.0" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "NEW FEATURES:" << std::endl;
    std::cout << "  * CLIENT-SERVER MULTIPLAYER ARCHITECTURE" << std::endl;
    std::cout << "  * Advanced Diplomacy System (War, Peace, Alliances)" << std::endl;
    std::cout << "  * Technology Research Tree (Military, Economic, Civic)" << std::endl;
    std::cout << "  * Trading System with Dynamic Market Prices" << std::endl;
    std::cout << "  * Weather Effects (Rain, Storm, Snow, Fog, Drought)" << std::endl;
    std::cout << "  * Seasonal Cycles (Spring, Summer, Fall, Winter)" << std::endl;
    std::cout << "  * Advanced Combat with Unit Formations" << std::endl;
    std::cout << "  * Mission/Quest System" << std::endl;
    std::cout << "  * Population Happiness and Growth Mechanics" << std::endl;
    std::cout << "  * Full Network Synchronization" << std::endl;
    std::cout << std::endl;
    std::cout << "CONTROLS:" << std::endl;
    std::cout << "  N - Toggle Multiplayer Mode" << std::endl;
    std::cout << "  TAB - Open/Close Menu" << std::endl;
    std::cout << "  SPACE - Pause/Resume" << std::endl;
    std::cout << "  1-5 - Simulation Speed" << std::endl;
    std::cout << "  R - Reset World" << std::endl;
    std::cout << std::endl;

    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = L"EnhancedCivilization";
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    RegisterClassEx(&wcex);

    RECT rect = { 0, 0, g_windowWidth, g_windowHeight };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    g_hWnd = CreateWindowEx(
        0,
        L"EnhancedCivilization",
        L"Neural Network Civilization - Enhanced Edition",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, hInstance, nullptr
    );

    g_hDC = GetDC(g_hWnd);

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(g_hDC, &pfd);
    SetPixelFormat(g_hDC, pixelFormat, &pfd);

    g_hRC = wglCreateContext(g_hDC);
    wglMakeCurrent(g_hDC, g_hRC);

    initOpenGL();
    initFont();
    resize(g_windowWidth, g_windowHeight);

    g_world = new World();
    g_world->WORLD_SIZE = 500;
    g_world->MAX_AGENTS_PER_FACTION = 100;
    g_world->initialize();

    // Initialize advanced game systems
    g_gameManager = std::make_unique<AdvancedGameManager>();
    g_marketSystem = std::make_unique<MarketSystem>();

    std::cout << "Advanced game systems initialized!" << std::endl;
    std::cout << "Press 'N' to enable multiplayer mode." << std::endl;

    // Initialize Extraction Shooter mode
    g_extractionShooter = new ExtractionShooter();
    std::cout << "\n========================================================" << std::endl;
    std::cout << " EXTRACTION SHOOTER MODE" << std::endl;
    std::cout << "========================================================" << std::endl;
    std::cout << "CONTROLS:" << std::endl;
    std::cout << "  WASD - Move" << std::endl;
    std::cout << "  SPACE - Sprint" << std::endl;
    std::cout << "  E - Pick up items" << std::endl;
    std::cout << "  F - Extract (when in extraction zone)" << std::endl;
    std::cout << "  ESC - Toggle inventory" << std::endl;
    std::cout << "  1/2 - Menu options" << std::endl;
    std::cout << "  Mouse - Look around" << std::endl;
    std::cout << std::endl;

    ShowWindow(g_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hWnd);

    MSG msg = {};
    g_lastTime = GetTickCount();

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            update();
            render();
            Sleep(16);
        }
    }

    delete g_world;
    delete g_extractionShooter;
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);

    return 0;
}