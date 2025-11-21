// Minimap System with Fog of War
// Displays tactical overview with explored areas

#pragma once

#include "../RaylibPlatform.h"
#include <vector>
#include <unordered_map>
#include <cmath>

struct MinimapIcon {
    Vector2 worldPosition;
    Color color;
    float size;
    bool visible;
    int iconType; // 0=player, 1=enemy, 2=corpse, 3=extract, 4=objective

    MinimapIcon() : worldPosition({0, 0}), color(WHITE), size(5.0f), visible(true), iconType(0) {}
};

class Minimap {
private:
    // Minimap properties
    Vector2 m_position;      // Screen position (top-right corner)
    float m_size;            // Minimap diameter
    float m_zoom;            // Zoom level (world units per pixel)
    float m_rotation;        // Map rotation (for north-up mode)
    bool m_rotateWithPlayer; // Rotate map with player view

    // Fog of war
    RenderTexture2D m_fogTexture;
    bool m_fogOfWarEnabled;
    float m_visionRadius;    // Player vision radius in world units
    std::vector<Vector2> m_exploredAreas; // Explored positions

    // Map bounds
    Rectangle m_worldBounds;

    // Icons to display
    std::vector<MinimapIcon> m_icons;

    // Colors
    Color m_backgroundColor;
    Color m_borderColor;
    Color m_exploredColor;
    Color m_unexploredColor;
    Color m_playerColor;

    bool m_initialized;

public:
    Minimap() : m_position({0, 0}), m_size(200.0f), m_zoom(10.0f), m_rotation(0.0f),
                m_rotateWithPlayer(true), m_fogOfWarEnabled(true), m_visionRadius(50.0f),
                m_backgroundColor({30, 30, 30, 200}), m_borderColor({100, 100, 100, 255}),
                m_exploredColor({80, 80, 80, 255}), m_unexploredColor({20, 20, 20, 255}),
                m_playerColor({0, 255, 0, 255}), m_initialized(false) {

        m_worldBounds = {-500, -500, 1000, 1000}; // Default world bounds
    }

    ~Minimap() {
        if (m_initialized) {
            UnloadRenderTexture(m_fogTexture);
        }
    }

    // Initialize minimap
    void initialize(Vector2 screenPos, float size) {
        m_position = screenPos;
        m_size = size;

        // Create fog of war texture
        m_fogTexture = LoadRenderTexture((int)size, (int)size);

        // Initialize fog of war (all unexplored)
        BeginTextureMode(m_fogTexture);
        ClearBackground(m_unexploredColor);
        EndTextureMode();

        m_initialized = true;
    }

    // Update minimap (call every frame)
    void update(Vector2 playerWorldPos, float playerRotation) {
        if (!m_initialized) return;

        m_rotation = m_rotateWithPlayer ? playerRotation : 0.0f;

        // Update fog of war
        if (m_fogOfWarEnabled) {
            revealArea(playerWorldPos, m_visionRadius);
        }
    }

    // Render minimap to screen
    void render(Vector2 playerWorldPos, float playerRotation) {
        if (!m_initialized) return;

        // Draw background circle
        DrawCircleV(m_position, m_size / 2.0f, m_backgroundColor);

        // Set scissor test for circular clipping
        BeginScissorMode(
            (int)(m_position.x - m_size / 2.0f),
            (int)(m_position.y - m_size / 2.0f),
            (int)m_size,
            (int)m_size
        );

        // Draw fog of war
        if (m_fogOfWarEnabled) {
            Rectangle source = {0, 0, (float)m_fogTexture.texture.width, (float)-m_fogTexture.texture.height};
            Rectangle dest = {m_position.x - m_size/2, m_position.y - m_size/2, m_size, m_size};
            DrawTexturePro(m_fogTexture.texture, source, dest, {0, 0}, 0.0f, WHITE);
        }

        // Draw icons (enemies, corpses, objectives, etc.)
        for (const auto& icon : m_icons) {
            if (!icon.visible) continue;

            Vector2 screenPos = worldToMinimapPos(icon.worldPosition, playerWorldPos);

            // Check if in bounds
            if (isPointInCircle(screenPos, m_position, m_size / 2.0f)) {
                if (icon.iconType == 0) {
                    // Player - triangle pointing forward
                    DrawIconPlayer(screenPos, icon.size, icon.color, playerRotation);
                } else if (icon.iconType == 1) {
                    // Enemy - red circle
                    DrawCircleV(screenPos, icon.size, icon.color);
                } else if (icon.iconType == 2) {
                    // Corpse - X mark
                    DrawIconCorpse(screenPos, icon.size, icon.color);
                } else if (icon.iconType == 3) {
                    // Extraction - pulsing circle
                    float pulse = (sinf(GetTime() * 3.0f) + 1.0f) / 2.0f;
                    DrawCircleV(screenPos, icon.size * (0.8f + pulse * 0.4f), Fade(icon.color, 0.7f));
                } else {
                    // Default - circle
                    DrawCircleV(screenPos, icon.size, icon.color);
                }
            }
        }

        // Draw player at center
        Vector2 centerPos = m_position;
        DrawIconPlayer(centerPos, 8.0f, m_playerColor, playerRotation);

        EndScissorMode();

        // Draw border
        DrawCircleLines((int)m_position.x, (int)m_position.y, m_size / 2.0f, m_borderColor);

        // Draw compass
        drawCompass();
    }

    // Add icon to minimap
    void addIcon(Vector2 worldPos, Color color, float size, int type) {
        MinimapIcon icon;
        icon.worldPosition = worldPos;
        icon.color = color;
        icon.size = size;
        icon.iconType = type;
        icon.visible = true;
        m_icons.push_back(icon);
    }

    // Clear all icons
    void clearIcons() {
        m_icons.clear();
    }

    // Reveal area on fog of war
    void revealArea(Vector2 worldPos, float radius) {
        if (!m_fogOfWarEnabled || !m_initialized) return;

        Vector2 texPos = worldToTexturePos(worldPos);
        float texRadius = (radius / m_zoom) * (m_size / m_worldBounds.width);

        BeginTextureMode(m_fogTexture);
        DrawCircleV(texPos, texRadius, m_exploredColor);
        EndTextureMode();

        // Store explored position
        m_exploredAreas.push_back(worldPos);
    }

    // Setters
    void setPosition(Vector2 pos) { m_position = pos; }
    void setSize(float size) { m_size = size; }
    void setZoom(float zoom) { m_zoom = zoom; }
    void setRotateWithPlayer(bool rotate) { m_rotateWithPlayer = rotate; }
    void setFogOfWar(bool enabled) { m_fogOfWarEnabled = enabled; }
    void setVisionRadius(float radius) { m_visionRadius = radius; }
    void setWorldBounds(Rectangle bounds) { m_worldBounds = bounds; }

private:
    // Convert world position to minimap screen position
    Vector2 worldToMinimapPos(Vector2 worldPos, Vector2 playerWorldPos) const {
        // Relative to player
        Vector2 relative = {
            worldPos.x - playerWorldPos.x,
            worldPos.y - playerWorldPos.y
        };

        // Apply rotation if enabled
        if (m_rotateWithPlayer) {
            float cosR = cosf(-m_rotation);
            float sinR = sinf(-m_rotation);
            float rx = relative.x * cosR - relative.y * sinR;
            float ry = relative.x * sinR + relative.y * cosR;
            relative = {rx, ry};
        }

        // Scale to minimap
        float scale = m_size / (m_zoom * 2.0f);
        Vector2 minimapPos = {
            m_position.x + relative.x * scale,
            m_position.y - relative.y * scale  // Flip Y
        };

        return minimapPos;
    }

    // Convert world position to fog texture position
    Vector2 worldToTexturePos(Vector2 worldPos) const {
        float x = ((worldPos.x - m_worldBounds.x) / m_worldBounds.width) * m_size;
        float y = ((worldPos.y - m_worldBounds.y) / m_worldBounds.height) * m_size;
        return {x, y};
    }

    // Check if point is in circle
    bool isPointInCircle(Vector2 point, Vector2 center, float radius) const {
        float dx = point.x - center.x;
        float dy = point.y - center.y;
        return (dx * dx + dy * dy) <= (radius * radius);
    }

    // Draw player icon (triangle)
    void DrawIconPlayer(Vector2 pos, float size, Color color, float rotation) const {
        Vector2 v1 = {pos.x, pos.y - size};
        Vector2 v2 = {pos.x - size * 0.7f, pos.y + size * 0.7f};
        Vector2 v3 = {pos.x + size * 0.7f, pos.y + size * 0.7f};

        // Rotate triangle
        float cosR = cosf(rotation);
        float sinR = sinf(rotation);

        auto rotatePoint = [&](Vector2 p) {
            float dx = p.x - pos.x;
            float dy = p.y - pos.y;
            return Vector2{
                pos.x + dx * cosR - dy * sinR,
                pos.y + dx * sinR + dy * cosR
            };
        };

        v1 = rotatePoint(v1);
        v2 = rotatePoint(v2);
        v3 = rotatePoint(v3);

        DrawTriangle(v1, v2, v3, color);
    }

    // Draw corpse icon (X mark)
    void DrawIconCorpse(Vector2 pos, float size, Color color) const {
        DrawLineEx({pos.x - size, pos.y - size}, {pos.x + size, pos.y + size}, 2.0f, color);
        DrawLineEx({pos.x + size, pos.y - size}, {pos.x - size, pos.y + size}, 2.0f, color);
    }

    // Draw compass rose
    void drawCompass() const {
        float compassSize = 20.0f;
        Vector2 compassPos = {m_position.x, m_position.y - m_size / 2.0f - 30.0f};

        // North indicator
        DrawText("N", (int)compassPos.x - 5, (int)compassPos.y - compassSize - 15, 20, RED);

        // Compass circle
        DrawCircleLines((int)compassPos.x, (int)compassPos.y, compassSize, GRAY);

        // North arrow
        float arrowRotation = m_rotateWithPlayer ? m_rotation : 0.0f;
        Vector2 arrowTip = {
            compassPos.x + sinf(arrowRotation) * compassSize,
            compassPos.y - cosf(arrowRotation) * compassSize
        };

        DrawLineEx(compassPos, arrowTip, 2.0f, RED);
    }
};
