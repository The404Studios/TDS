// First-Person Zombie Survival Game
// Complete with FPS controls, working UI, zombies with NavMesh, base building, guns, and lighting

#define NOMINMAX
#define _USE_MATH_DEFINES
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include "ZombieGame.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Global variables
GameWorld* g_world = nullptr;
HWND g_hWnd = nullptr;
HDC g_hDC = nullptr;
HGLRC g_hRC = nullptr;
int g_windowWidth = 1600;
int g_windowHeight = 900;

// Input
bool g_keys[256] = { false };
bool g_mouseButtons[3] = { false };
int g_lastMouseX = 800;
int g_lastMouseY = 450;
bool g_mouseCaptured = false;
float g_mouseSensitivity = 0.2f;

// Game state
bool g_paused = false;
DWORD g_lastTime = 0;
float g_deltaTime = 0.0f;

// UI
GLuint g_fontBase = 0;
bool g_showUI = true;
bool g_buildMode = false;
BuildingType g_selectedBuilding = BuildingType::WALL;

// Rendering options
bool g_showFPS = true;
int g_fps = 0;
int g_frameCount = 0;
float g_fpsTimer = 0;

// Crosshair
void drawCrosshair() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, g_windowWidth, g_windowHeight, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    float cx = g_windowWidth / 2.0f;
    float cy = g_windowHeight / 2.0f;
    float size = 15.0f;

    glColor3f(0.0f, 1.0f, 0.0f);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
    // Horizontal
    glVertex2f(cx - size, cy);
    glVertex2f(cx + size, cy);
    // Vertical
    glVertex2f(cx, cy - size);
    glVertex2f(cx, cy + size);
    glEnd();

    // Dot in center
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    glVertex2f(cx, cy);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// Font rendering
void initFont() {
    HFONT font;
    HFONT oldFont;

    g_fontBase = glGenLists(256);

    font = CreateFont(
        -16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH,
        L"Arial"
    );

    oldFont = (HFONT)SelectObject(g_hDC, font);
    wglUseFontBitmaps(g_hDC, 0, 255, g_fontBase);
    SelectObject(g_hDC, oldFont);
    DeleteObject(font);
}

void drawText(float x, float y, const std::string& text, float r = 1, float g = 1, float b = 1) {
    if (g_fontBase == 0) return;

    glColor3f(r, g, b);
    glRasterPos2f(x, y);

    glPushAttrib(GL_LIST_BIT);
    glListBase(g_fontBase);
    glCallLists(text.length(), GL_UNSIGNED_BYTE, text.c_str());
    glPopAttrib();
}

// Draw 3D models
void drawCube(float size) {
    float h = size / 2.0f;
    glBegin(GL_QUADS);
    // Front
    glNormal3f(0, 1, 0);
    glVertex3f(-h, h, -h);
    glVertex3f(h, h, -h);
    glVertex3f(h, h, h);
    glVertex3f(-h, h, h);
    // Back
    glNormal3f(0, -1, 0);
    glVertex3f(-h, -h, -h);
    glVertex3f(-h, -h, h);
    glVertex3f(h, -h, h);
    glVertex3f(h, -h, -h);
    // Top
    glNormal3f(0, 0, 1);
    glVertex3f(-h, -h, h);
    glVertex3f(-h, h, h);
    glVertex3f(h, h, h);
    glVertex3f(h, -h, h);
    // Bottom
    glNormal3f(0, 0, -1);
    glVertex3f(-h, -h, -h);
    glVertex3f(h, -h, -h);
    glVertex3f(h, h, -h);
    glVertex3f(-h, h, -h);
    // Right
    glNormal3f(1, 0, 0);
    glVertex3f(h, -h, -h);
    glVertex3f(h, -h, h);
    glVertex3f(h, h, h);
    glVertex3f(h, h, -h);
    // Left
    glNormal3f(-1, 0, 0);
    glVertex3f(-h, -h, -h);
    glVertex3f(-h, h, -h);
    glVertex3f(-h, h, h);
    glVertex3f(-h, -h, h);
    glEnd();
}

void drawSphere(float radius, int slices = 16, int stacks = 16) {
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
            glVertex3f(x * r0, y * r0, z0);

            glNormal3f(x * r1 / radius, y * r1 / radius, z1 / radius);
            glVertex3f(x * r1, y * r1, z1);
        }
        glEnd();
    }
}

void drawCylinder(float radius, float height, int slices = 16) {
    float halfHeight = height / 2.0f;

    // Body
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; i++) {
        float angle = 2.0f * M_PI * i / slices;
        float x = cos(angle) * radius;
        float y = sin(angle) * radius;

        glNormal3f(x / radius, y / radius, 0);
        glVertex3f(x, y, -halfHeight);
        glVertex3f(x, y, halfHeight);
    }
    glEnd();

    // Top cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);
    glVertex3f(0, 0, halfHeight);
    for (int i = 0; i <= slices; i++) {
        float angle = 2.0f * M_PI * i / slices;
        glVertex3f(cos(angle) * radius, sin(angle) * radius, halfHeight);
    }
    glEnd();

    // Bottom cap
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, -1);
    glVertex3f(0, 0, -halfHeight);
    for (int i = slices; i >= 0; i--) {
        float angle = 2.0f * M_PI * i / slices;
        glVertex3f(cos(angle) * radius, sin(angle) * radius, -halfHeight);
    }
    glEnd();
}

// Draw weapon model
void drawWeapon(Weapon* weapon) {
    if (!weapon) return;

    glPushMatrix();

    // Position weapon in front of camera (lower right)
    glTranslatef(0.3f, -0.2f, -0.5f);
    glRotatef(-10, 0, 0, 1);

    glColor3f(0.2f, 0.2f, 0.2f);

    switch (weapon->type) {
    case WeaponType::PISTOL:
        // Pistol handle
        glPushMatrix();
        glTranslatef(0, -0.05f, 0);
        glRotatef(70, 1, 0, 0);
        drawCube(0.04f);
        glPopMatrix();
        // Barrel
        glPushMatrix();
        glTranslatef(0, 0.05f, 0);
        drawCube(0.12f);
        glPopMatrix();
        break;

    case WeaponType::SHOTGUN:
        // Stock
        glPushMatrix();
        glTranslatef(0, -0.15f, 0);
        drawCube(0.06f);
        glPopMatrix();
        // Barrel
        glPushMatrix();
        glTranslatef(0, 0.1f, 0);
        glScalef(0.5f, 2.5f, 0.5f);
        drawCube(0.05f);
        glPopMatrix();
        break;

    case WeaponType::RIFLE:
        // Stock
        glPushMatrix();
        glTranslatef(0, -0.2f, -0.02f);
        drawCube(0.05f);
        glPopMatrix();
        // Body
        glPushMatrix();
        glScalef(0.6f, 2.0f, 0.6f);
        drawCube(0.06f);
        glPopMatrix();
        // Barrel
        glPushMatrix();
        glTranslatef(0, 0.15f, 0);
        glScalef(0.4f, 1.5f, 0.4f);
        drawCylinder(0.02f, 0.3f);
        glPopMatrix();
        break;

    case WeaponType::MACHINE_GUN:
        // Large body
        glPushMatrix();
        glScalef(0.8f, 1.8f, 0.8f);
        drawCube(0.08f);
        glPopMatrix();
        // Barrel
        glPushMatrix();
        glTranslatef(0, 0.2f, 0);
        glScalef(0.5f, 2.0f, 0.5f);
        drawCylinder(0.025f, 0.25f);
        glPopMatrix();
        break;

    case WeaponType::SNIPER:
        // Scope
        glColor3f(0.1f, 0.1f, 0.3f);
        glPushMatrix();
        glTranslatef(0, 0, 0.05f);
        glScalef(0.5f, 1.5f, 0.5f);
        drawCylinder(0.03f, 0.15f);
        glPopMatrix();
        // Body
        glColor3f(0.2f, 0.2f, 0.2f);
        glPushMatrix();
        glScalef(0.6f, 2.5f, 0.6f);
        drawCube(0.05f);
        glPopMatrix();
        break;
    }

    glPopMatrix();
}

// Draw zombie model
void drawZombie(Zombie* zombie) {
    if (!zombie || zombie->isDead) {
        if (zombie && zombie->deathTimer < 2.0f) {
            // Draw dead zombie lying down
            glPushMatrix();
            glTranslatef(zombie->position.x, zombie->position.y, 0.3f);
            glRotatef(90, 1, 0, 0);
            glColor3f(0.4f, 0.5f, 0.3f);
            drawSphere(0.5f);
            glPopMatrix();
        }
        return;
    }

    glPushMatrix();
    glTranslatef(zombie->position.x, zombie->position.y, 0);

    // Head
    glPushMatrix();
    glTranslatef(0, 0, 1.6f);
    glColor3f(0.5f, 0.6f, 0.4f);
    drawSphere(0.3f);
    glPopMatrix();

    // Body
    glPushMatrix();
    glTranslatef(0, 0, 0.9f);
    glColor3f(0.4f, 0.5f, 0.3f);
    drawCube(0.8f);
    glPopMatrix();

    // Arms (animated)
    float armSwing = sin(zombie->walkCycle * 3.0f) * 30.0f;

    // Left arm
    glPushMatrix();
    glTranslatef(-0.5f, 0, 0.9f);
    glRotatef(armSwing, 1, 0, 0);
    glColor3f(0.5f, 0.6f, 0.4f);
    glPushMatrix();
    glTranslatef(0, 0, -0.3f);
    glScalef(0.3f, 0.3f, 1.2f);
    drawCube(0.5f);
    glPopMatrix();
    glPopMatrix();

    // Right arm
    glPushMatrix();
    glTranslatef(0.5f, 0, 0.9f);
    glRotatef(-armSwing, 1, 0, 0);
    glColor3f(0.5f, 0.6f, 0.4f);
    glPushMatrix();
    glTranslatef(0, 0, -0.3f);
    glScalef(0.3f, 0.3f, 1.2f);
    drawCube(0.5f);
    glPopMatrix();
    glPopMatrix();

    // Legs (animated)
    float legSwing = sin(zombie->walkCycle * 3.0f) * 20.0f;

    // Left leg
    glPushMatrix();
    glTranslatef(-0.2f, 0, 0.4f);
    glRotatef(legSwing, 1, 0, 0);
    glColor3f(0.3f, 0.4f, 0.25f);
    glPushMatrix();
    glTranslatef(0, 0, -0.4f);
    glScalef(0.3f, 0.3f, 1.5f);
    drawCube(0.4f);
    glPopMatrix();
    glPopMatrix();

    // Right leg
    glPushMatrix();
    glTranslatef(0.2f, 0, 0.4f);
    glRotatef(-legSwing, 1, 0, 0);
    glColor3f(0.3f, 0.4f, 0.25f);
    glPushMatrix();
    glTranslatef(0, 0, -0.4f);
    glScalef(0.3f, 0.3f, 1.5f);
    drawCube(0.4f);
    glPopMatrix();
    glPopMatrix();

    glPopMatrix();
}

// Draw building
void drawBuilding(BaseBuilding* building) {
    if (!building || !building->active) return;

    glPushMatrix();
    glTranslatef(building->position.x, building->position.y, 0);

    float healthRatio = building->health / building->maxHealth;
    glColor3f(0.5f * healthRatio, 0.5f * healthRatio, 0.5f * healthRatio);

    switch (building->type) {
    case BuildingType::WALL:
        glPushMatrix();
        glScalef(4.0f, 1.0f, 1.0f);
        drawCube(2.0f);
        glPopMatrix();
        break;

    case BuildingType::TURRET:
        // Base
        glColor3f(0.3f, 0.3f, 0.3f);
        glPushMatrix();
        glTranslatef(0, 0, 0.5f);
        drawCylinder(0.8f, 1.0f);
        glPopMatrix();

        // Turret head
        glPushMatrix();
        glTranslatef(0, 0, 1.5f);
        glRotatef(building->turretYaw, 0, 0, 1);
        glColor3f(0.4f, 0.4f, 0.4f);
        drawCube(1.0f);
        // Barrel
        glTranslatef(0, 0.8f, 0);
        glColor3f(0.2f, 0.2f, 0.2f);
        glRotatef(90, 1, 0, 0);
        drawCylinder(0.15f, 1.0f);
        glPopMatrix();
        break;

    case BuildingType::AMMO_STATION:
        glColor3f(0.8f, 0.6f, 0.2f);
        drawCube(1.5f);
        glTranslatef(0, 0, 1.0f);
        glColor3f(0.7f, 0.5f, 0.1f);
        drawCube(0.8f);
        break;

    case BuildingType::HEALTH_STATION:
        glColor3f(0.2f, 0.8f, 0.2f);
        drawCube(1.5f);
        glTranslatef(0, 0, 1.0f);
        glColor3f(1.0f, 0.0f, 0.0f);
        // Cross symbol
        glBegin(GL_QUADS);
        glVertex3f(-0.1f, -0.4f, 0.1f);
        glVertex3f(0.1f, -0.4f, 0.1f);
        glVertex3f(0.1f, 0.4f, 0.1f);
        glVertex3f(-0.1f, 0.4f, 0.1f);

        glVertex3f(-0.4f, -0.1f, 0.1f);
        glVertex3f(0.4f, -0.1f, 0.1f);
        glVertex3f(0.4f, 0.1f, 0.1f);
        glVertex3f(-0.4f, 0.1f, 0.1f);
        glEnd();
        break;

    case BuildingType::BARRICADE:
        glColor3f(0.6f, 0.4f, 0.2f);
        for (int i = 0; i < 5; i++) {
            glPushMatrix();
            glTranslatef((i - 2) * 0.5f, 0, 0.5f + i * 0.1f);
            glScalef(0.2f, 0.8f, 1.5f);
            drawCube(1.0f);
            glPopMatrix();
        }
        break;
    }

    glPopMatrix();
}

// Draw terrain
void drawTerrain() {
    glColor3f(0.3f, 0.4f, 0.2f); // Grass color

    int gridSize = 50;
    float cellSize = (float)GameWorld::WORLD_SIZE / gridSize;

    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);

    for (int x = 0; x < gridSize; x++) {
        for (int y = 0; y < gridSize; y++) {
            float x1 = x * cellSize;
            float y1 = y * cellSize;
            float x2 = (x + 1) * cellSize;
            float y2 = (y + 1) * cellSize;

            // Vary color slightly for visual interest
            float colorVar = ((x + y) % 2) * 0.05f;
            glColor3f(0.3f + colorVar, 0.4f + colorVar, 0.2f + colorVar);

            glVertex3f(x1, y1, 0);
            glVertex3f(x2, y1, 0);
            glVertex3f(x2, y2, 0);
            glVertex3f(x1, y2, 0);
        }
    }
    glEnd();

    // Grid lines
    glDisable(GL_LIGHTING);
    glColor3f(0.2f, 0.3f, 0.15f);
    glLineWidth(1.0f);

    glBegin(GL_LINES);
    for (int i = 0; i <= gridSize; i++) {
        float pos = i * cellSize;
        glVertex3f(pos, 0, 0.01f);
        glVertex3f(pos, GameWorld::WORLD_SIZE, 0.01f);
        glVertex3f(0, pos, 0.01f);
        glVertex3f(GameWorld::WORLD_SIZE, pos, 0.01f);
    }
    glEnd();

    glEnable(GL_LIGHTING);
}

// Draw particles
void drawParticles() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    for (const auto& p : g_world->particles) {
        glPointSize(p.size);
        glBegin(GL_POINTS);
        glColor4f(p.r, p.g, p.b, p.a);
        glVertex3f(p.position.x, p.position.y, p.position.z);
        glEnd();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// Draw UI
void drawUI() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, g_windowWidth, g_windowHeight, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    Player* player = g_world->player.get();
    Weapon* weapon = player->getCurrentWeapon();

    // Health bar
    glColor3f(0.8f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(20, g_windowHeight - 60);
    glVertex2f(20 + (player->health / player->maxHealth) * 300, g_windowHeight - 60);
    glVertex2f(20 + (player->health / player->maxHealth) * 300, g_windowHeight - 40);
    glVertex2f(20, g_windowHeight - 40);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(20, g_windowHeight - 60);
    glVertex2f(320, g_windowHeight - 60);
    glVertex2f(320, g_windowHeight - 40);
    glVertex2f(20, g_windowHeight - 40);
    glEnd();

    std::stringstream ss;
    ss << "Health: " << (int)player->health << "/" << (int)player->maxHealth;
    drawText(25, g_windowHeight - 45, ss.str(), 1, 1, 1);

    // Ammo display
    if (weapon) {
        ss.str("");
        ss << weapon->name << ": " << weapon->currentClip << " / " << weapon->ammo;
        drawText(g_windowWidth - 300, g_windowHeight - 80, ss.str(), 1, 1, 0);

        if (weapon->reloading) {
            float reloadProgress = weapon->currentReloadTime / weapon->reloadTime;
            glColor3f(1.0f, 1.0f, 0.0f);
            glBegin(GL_QUADS);
            glVertex2f(g_windowWidth - 300, g_windowHeight - 60);
            glVertex2f(g_windowWidth - 300 + reloadProgress * 200, g_windowHeight - 60);
            glVertex2f(g_windowWidth - 300 + reloadProgress * 200, g_windowHeight - 50);
            glVertex2f(g_windowWidth - 300, g_windowHeight - 50);
            glEnd();

            drawText(g_windowWidth - 280, g_windowHeight - 55, "RELOADING...", 1, 1, 0);
        }
    }

    // Wave info
    ss.str("");
    ss << "Wave: " << g_world->wave;
    drawText(g_windowWidth / 2 - 50, 30, ss.str(), 1, 1, 1);

    ss.str("");
    int aliveZombies = 0;
    for (auto& z : g_world->zombies) {
        if (!z->isDead) aliveZombies++;
    }
    ss << "Zombies: " << aliveZombies << " + " << g_world->zombiesRemaining << " incoming";
    drawText(g_windowWidth / 2 - 150, 50, ss.str(), 1, 0.5f, 0.5f);

    // Money
    ss.str("");
    ss << "Money: $" << player->money;
    drawText(20, 30, ss.str(), 0.2f, 1.0f, 0.2f);

    // Kills
    ss.str("");
    ss << "Kills: " << player->kills;
    drawText(20, 50, ss.str(), 1, 1, 1);

    // FPS
    if (g_showFPS) {
        ss.str("");
        ss << "FPS: " << g_fps;
        drawText(g_windowWidth - 100, 30, ss.str(), 1, 1, 0);
    }

    // Wave complete message
    if (!g_world->waveActive && aliveZombies == 0) {
        drawText(g_windowWidth / 2 - 200, g_windowHeight / 2, "WAVE COMPLETE! Press N for next wave", 0, 1, 0);
    }

    // Build mode
    if (g_buildMode) {
        drawText(g_windowWidth / 2 - 150, g_windowHeight - 100, "BUILD MODE - Click to place", 1, 1, 0);

        const char* buildingName = "Unknown";
        int cost = 0;

        auto tempBuilding = std::make_unique<BaseBuilding>(g_selectedBuilding, Vector3());
        buildingName = tempBuilding->getName();
        cost = tempBuilding->cost;

        ss.str("");
        ss << buildingName << " - Cost: $" << cost;
        drawText(g_windowWidth / 2 - 100, g_windowHeight - 80, ss.str(), 1, 1, 1);

        drawText(g_windowWidth / 2 - 200, g_windowHeight - 60,
            "[1-5] Select Building | [B] Exit Build Mode", 0.8f, 0.8f, 0.8f);
    }

    // Controls
    drawText(20, 70, "WASD: Move | Mouse: Look | LMB: Shoot | R: Reload", 0.7f, 0.7f, 0.7f);
    drawText(20, 90, "1-2: Switch Weapon | B: Build Mode | N: Next Wave | ESC: Quit", 0.7f, 0.7f, 0.7f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// Render function
void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    Player* player = g_world->player.get();

    // Set up first-person camera
    Vector3 camPos = player->position + Vector3(0, 0, 1.6f); // Eye level
    Vector3 lookAt = camPos + player->forward;

    gluLookAt(
        camPos.x, camPos.y, camPos.z,
        lookAt.x, lookAt.y, lookAt.z,
        player->up.x, player->up.y, player->up.z
    );

    // Draw world
    drawTerrain();

    // Draw buildings
    for (auto& building : g_world->buildings) {
        drawBuilding(building.get());
    }

    // Draw zombies
    for (auto& zombie : g_world->zombies) {
        drawZombie(zombie.get());
    }

    // Draw bullets
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 0.0f);
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    for (const auto& bullet : g_world->bullets) {
        glVertex3f(bullet.position.x, bullet.position.y, bullet.position.z);
    }
    glEnd();
    glEnable(GL_LIGHTING);

    // Draw particles
    drawParticles();

    // Draw weapon (in camera space)
    glPushMatrix();
    glLoadIdentity();

    // Rotate based on player view
    glRotatef(-player->pitch, 1, 0, 0);
    glRotatef(-player->yaw + 180, 0, 0, 1);

    drawWeapon(player->getCurrentWeapon());
    glPopMatrix();

    // Draw UI
    drawCrosshair();
    drawUI();

    SwapBuffers(g_hDC);

    // FPS counter
    g_frameCount++;
}

// Initialize OpenGL with enhanced lighting
void initOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Ambient light
    GLfloat globalAmbient[] = { 0.3f, 0.3f, 0.4f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    // Directional light (sun)
    glEnable(GL_LIGHT0);
    GLfloat light0Pos[] = { 1.0f, 1.0f, 2.0f, 0.0f }; // Directional
    GLfloat light0Dif[] = { 0.9f, 0.9f, 0.8f, 1.0f };
    GLfloat light0Spec[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Dif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0Spec);

    // Point light (player flashlight - dynamic)
    glEnable(GL_LIGHT1);

    // Fog for atmosphere
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    GLfloat fogColor[] = { 0.2f, 0.2f, 0.3f, 1.0f };
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.02f);
    glFogf(GL_FOG_START, 50.0f);
    glFogf(GL_FOG_END, 150.0f);

    glClearColor(0.2f, 0.2f, 0.3f, 1.0f); // Dark sky
}

void resize(int width, int height) {
    if (height == 0) height = 1;

    g_windowWidth = width;
    g_windowHeight = height;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(75.0, (double)width / (double)height, 0.1, 500.0);
    glMatrixMode(GL_MODELVIEW);
}

// Update game
void update() {
    DWORD currentTime = GetTickCount();
    g_deltaTime = (currentTime - g_lastTime) / 1000.0f;
    if (g_deltaTime > 0.1f) g_deltaTime = 0.1f;

    if (!g_paused) {
        // Update player movement
        Player* player = g_world->player.get();
        Vector3 moveDir(0, 0, 0);

        if (g_keys['W']) moveDir = moveDir + player->forward;
        if (g_keys['S']) moveDir = moveDir - player->forward;
        if (g_keys['A']) moveDir = moveDir - player->right;
        if (g_keys['D']) moveDir = moveDir + player->right;

        // Project movement to ground plane
        moveDir.z = 0;
        if (moveDir.length() > 0) {
            player->sprinting = g_keys[VK_SHIFT];
            player->move(moveDir, g_deltaTime);
        }

        // Update world
        g_world->update(g_deltaTime);

        // Update player flashlight
        Vector3 lightPos = player->position + Vector3(0, 0, 1.6f);
        GLfloat light1Pos[] = { lightPos.x, lightPos.y, lightPos.z, 1.0f };
        GLfloat light1Dir[] = { player->forward.x, player->forward.y, player->forward.z };
        GLfloat light1Dif[] = { 0.8f, 0.8f, 0.7f, 1.0f };

        glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light1Dir);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Dif);
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 45.0f);
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 2.0f);
    }

    // FPS calculation
    g_fpsTimer += g_deltaTime;
    if (g_fpsTimer >= 1.0f) {
        g_fps = g_frameCount;
        g_frameCount = 0;
        g_fpsTimer = 0;
    }

    g_lastTime = currentTime;
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_SIZE:
        resize(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_KEYDOWN:
        g_keys[wParam] = true;

        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
        }
        else if (wParam == 'B') {
            g_buildMode = !g_buildMode;
        }
        else if (wParam == 'N' && !g_world->waveActive) {
            g_world->startNextWave();
        }
        else if (wParam == 'R') {
            // Reload
            if (g_world->player->getCurrentWeapon()) {
                g_world->player->getCurrentWeapon()->reload();
            }
        }
        else if (wParam >= '1' && wParam <= '5') {
            if (g_buildMode) {
                g_selectedBuilding = (BuildingType)(wParam - '1');
            }
            else {
                g_world->player->switchWeapon(wParam - '1');
            }
        }
        return 0;

    case WM_KEYUP:
        g_keys[wParam] = false;
        return 0;

    case WM_LBUTTONDOWN:
        g_mouseButtons[0] = true;

        if (g_buildMode) {
            // Place building
            Player* player = g_world->player.get();
            Vector3 buildPos = player->position + player->forward * 5.0f;
            buildPos.z = 0;

            auto building = std::make_unique<BaseBuilding>(g_selectedBuilding, buildPos);
            if (player->money >= building->cost) {
                player->money -= building->cost;
                g_world->buildings.push_back(std::move(building));
            }
        }
        return 0;

    case WM_LBUTTONUP:
        g_mouseButtons[0] = false;
        return 0;

    case WM_MOUSEMOVE:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        if (g_mouseCaptured) {
            float dx = (float)(x - g_lastMouseX);
            float dy = (float)(y - g_lastMouseY);

            g_world->player->rotate(dx * g_mouseSensitivity, -dy * g_mouseSensitivity);

            // Re-center cursor
            POINT pt = { g_windowWidth / 2, g_windowHeight / 2 };
            ClientToScreen(hWnd, &pt);
            SetCursorPos(pt.x, pt.y);

            g_lastMouseX = g_windowWidth / 2;
            g_lastMouseY = g_windowHeight / 2;
        }
        else {
            g_lastMouseX = x;
            g_lastMouseY = y;
        }
    }
    return 0;

    case WM_SETFOCUS:
        g_mouseCaptured = true;
        ShowCursor(FALSE);
        return 0;

    case WM_KILLFOCUS:
        g_mouseCaptured = false;
        ShowCursor(TRUE);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

// Main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "========================================" << std::endl;
    std::cout << " FIRST-PERSON ZOMBIE SURVIVAL GAME" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Features:" << std::endl;
    std::cout << "- FPS controls with mouse look" << std::endl;
    std::cout << "- Multiple weapons with realistic models" << std::endl;
    std::cout << "- Zombie AI with NavMesh pathfinding" << std::endl;
    std::cout << "- Base building and upgrades" << std::endl;
    std::cout << "- Wave-based survival gameplay" << std::endl;
    std::cout << "- Advanced lighting (directional + point lights)" << std::endl;
    std::cout << "- Full 3D rendering with proper vertices" << std::endl;
    std::cout << "========================================" << std::endl;

    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = L"ZombieGame";

    RegisterClassEx(&wcex);

    RECT rect = { 0, 0, g_windowWidth, g_windowHeight };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    g_hWnd = CreateWindowEx(
        0, L"ZombieGame", L"Zombie Survival - First Person Shooter",
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

    // Initialize game
    g_world = new GameWorld();
    g_world->initialize();

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    // Capture mouse
    g_mouseCaptured = true;
    ShowCursor(FALSE);

    MSG msg = {};
    g_lastTime = GetTickCount();

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // Shooting
            if (g_mouseButtons[0] && !g_buildMode && !g_paused) {
                Weapon* weapon = g_world->player->getCurrentWeapon();
                if (weapon && weapon->canFire() && weapon->lastFireTime >= weapon->fireRate) {
                    weapon->fire();

                    // Create bullet
                    Vector3 shootPos = g_world->player->position + Vector3(0, 0, 1.6f);
                    Vector3 shootDir = g_world->player->forward;

                    // Add accuracy spread
                    float spread = (1.0f - weapon->accuracy) * 0.1f;
                    shootDir.x += ((rand() % 100 - 50) / 50.0f) * spread;
                    shootDir.y += ((rand() % 100 - 50) / 50.0f) * spread;
                    shootDir.z += ((rand() % 100 - 50) / 50.0f) * spread;
                    shootDir = shootDir.normalize();

                    Bullet bullet(shootPos, shootDir * 100.0f, weapon->damage, true);
                    g_world->bullets.push_back(bullet);

                    // Muzzle flash
                    g_world->addMuzzleFlash(shootPos, shootDir);
                }
            }

            update();
            render();
            Sleep(8); // ~120 FPS cap
        }
    }

    delete g_world;
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);

    return 0;
}
