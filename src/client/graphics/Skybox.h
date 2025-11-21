// Skybox Rendering System
// Provides immersive environment with day/night cycle

#pragma once

#include "../RaylibPlatform.h"
#include <string>

class Skybox {
private:
    Model m_cubeModel;
    Shader m_skyboxShader;
    Texture2D m_skyboxTexture;
    bool m_loaded;

    // Day/night cycle
    float m_timeOfDay;  // 0.0 = midnight, 0.5 = noon, 1.0 = midnight
    float m_cycleSpeed;

    // Sky colors for different times
    Color m_skyColorDay;
    Color m_skyColorNight;
    Color m_skyColorSunrise;
    Color m_skyColorSunset;

    // Sun/Moon
    Vector3 m_sunPosition;
    Vector3 m_moonPosition;
    Color m_sunColor;
    Color m_moonColor;

public:
    Skybox() : m_loaded(false), m_timeOfDay(0.5f), m_cycleSpeed(0.01f) {
        // Default colors
        m_skyColorDay = {135, 206, 235, 255};      // Sky blue
        m_skyColorNight = {25, 25, 112, 255};       // Midnight blue
        m_skyColorSunrise = {255, 140, 0, 255};     // Dark orange
        m_skyColorSunset = {255, 69, 0, 255};       // Orange red

        m_sunColor = {255, 255, 200, 255};
        m_moonColor = {200, 200, 255, 255};
    }

    ~Skybox() {
        if (m_loaded) {
            unload();
        }
    }

    // Load skybox
    bool load(const std::string& texturePath = "") {
        // Create cube model for skybox
        m_cubeModel = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));

        // Load texture if provided, otherwise use procedural
        if (!texturePath.empty() && FileExists(texturePath.c_str())) {
            m_skyboxTexture = LoadTexture(texturePath.c_str());
        } else {
            // Create procedural gradient skybox
            m_skyboxTexture = createProceduralSkybox();
        }

        // Set texture to model
        m_cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m_skyboxTexture;

        m_loaded = true;
        return true;
    }

    // Unload skybox resources
    void unload() {
        if (!m_loaded) return;

        UnloadTexture(m_skyboxTexture);
        UnloadModel(m_cubeModel);
        m_loaded = false;
    }

    // Update time of day
    void update(float deltaTime) {
        if (!m_loaded) return;

        m_timeOfDay += m_cycleSpeed * deltaTime;
        if (m_timeOfDay > 1.0f) m_timeOfDay -= 1.0f;

        // Update sun/moon positions
        float angle = m_timeOfDay * 2.0f * PI;
        m_sunPosition = {
            cosf(angle) * 1000.0f,
            sinf(angle) * 1000.0f,
            0.0f
        };
        m_moonPosition = {
            -m_sunPosition.x,
            -m_sunPosition.y,
            m_sunPosition.z
        };
    }

    // Render skybox (should be called first in render loop)
    void render(Camera3D camera) {
        if (!m_loaded) return;

        rlDisableBackfaceCulling();
        rlDisableDepthMask();

        // Scale up the cube and center on camera
        Matrix matScale = MatrixScale(1000.0f, 1000.0f, 1000.0f);
        Matrix matTranslate = MatrixTranslate(camera.position.x, camera.position.y, camera.position.z);
        Matrix matTransform = MatrixMultiply(matScale, matTranslate);

        // Draw the skybox
        DrawMesh(m_cubeModel.meshes[0], m_cubeModel.materials[0], matTransform);

        rlEnableBackfaceCulling();
        rlEnableDepthMask();
    }

    // Get current sky color based on time of day
    Color getCurrentSkyColor() const {
        // Interpolate between colors based on time
        if (m_timeOfDay < 0.25f) {
            // Night to sunrise
            float t = m_timeOfDay / 0.25f;
            return ColorLerp(m_skyColorNight, m_skyColorSunrise, t);
        } else if (m_timeOfDay < 0.5f) {
            // Sunrise to day
            float t = (m_timeOfDay - 0.25f) / 0.25f;
            return ColorLerp(m_skyColorSunrise, m_skyColorDay, t);
        } else if (m_timeOfDay < 0.75f) {
            // Day to sunset
            float t = (m_timeOfDay - 0.5f) / 0.25f;
            return ColorLerp(m_skyColorDay, m_skyColorSunset, t);
        } else {
            // Sunset to night
            float t = (m_timeOfDay - 0.75f) / 0.25f;
            return ColorLerp(m_skyColorSunset, m_skyColorNight, t);
        }
    }

    // Get sun direction (for directional lighting)
    Vector3 getSunDirection() const {
        return Vector3Normalize({-m_sunPosition.x, -m_sunPosition.y, -m_sunPosition.z});
    }

    // Get sun color
    Color getSunColor() const {
        float intensity = sinf(m_timeOfDay * PI);  // Brighter at noon
        return ColorBrightness(m_sunColor, intensity);
    }

    // Time of day controls
    void setTimeOfDay(float time) { m_timeOfDay = fmodf(time, 1.0f); }
    float getTimeOfDay() const { return m_timeOfDay; }
    void setCycleSpeed(float speed) { m_cycleSpeed = speed; }
    bool isDay() const { return m_timeOfDay > 0.25f && m_timeOfDay < 0.75f; }
    bool isNight() const { return !isDay(); }

private:
    // Create procedural gradient skybox
    Texture2D createProceduralSkybox() {
        const int size = 512;
        Image img = GenImageColor(size, size, m_skyColorDay);

        // Create simple vertical gradient
        Color* pixels = LoadImageColors(img);
        for (int y = 0; y < size; y++) {
            float t = (float)y / size;
            Color skyColor = ColorLerp(m_skyColorDay, {255, 255, 255, 255}, t * 0.3f);

            for (int x = 0; x < size; x++) {
                pixels[y * size + x] = skyColor;
            }
        }

        UpdateImageColors(&img, pixels);
        UnloadImageColors(pixels);

        Texture2D tex = LoadTextureFromImage(img);
        UnloadImage(img);
        return tex;
    }

    // Helper: Lerp between colors
    Color ColorLerp(Color c1, Color c2, float t) const {
        t = Clamp(t, 0.0f, 1.0f);
        return {
            (unsigned char)(c1.r + (c2.r - c1.r) * t),
            (unsigned char)(c1.g + (c2.g - c1.g) * t),
            (unsigned char)(c1.b + (c2.b - c1.b) * t),
            (unsigned char)(c1.a + (c2.a - c1.a) * t)
        };
    }

    // Helper: Adjust color brightness
    Color ColorBrightness(Color c, float factor) const {
        factor = Clamp(factor, -1.0f, 1.0f);
        return {
            (unsigned char)Clamp(c.r * (1.0f + factor), 0.0f, 255.0f),
            (unsigned char)Clamp(c.g * (1.0f + factor), 0.0f, 255.0f),
            (unsigned char)Clamp(c.b * (1.0f + factor), 0.0f, 255.0f),
            c.a
        };
    }
};
