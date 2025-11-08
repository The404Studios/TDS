#pragma once

#include "../rendering/RenderEngine.h"
#include "../particles/ParticleSystem.h"
#include <memory>

// Weather type
enum class WeatherType {
    CLEAR,
    RAIN,
    SNOW,
    FOG,
    STORM,
    SANDSTORM,
    BLIZZARD
};

// Time of day
struct TimeOfDay {
    float hour;           // 0-24
    float timeScale;      // 1.0 = realtime, higher = faster

    Color sunColor;
    Color moonColor;
    Color skyColor;
    Color horizonColor;
    Color ambientColor;

    Vec3 sunDirection;
    Vec3 moonDirection;
    float sunIntensity;
    float moonIntensity;

    TimeOfDay() : hour(12.0f), timeScale(1.0f),
                  sunColor(1.0f, 0.95f, 0.8f, 1.0f),
                  moonColor(0.8f, 0.8f, 1.0f, 1.0f),
                  skyColor(0.5f, 0.7f, 1.0f, 1.0f),
                  horizonColor(0.8f, 0.85f, 1.0f, 1.0f),
                  ambientColor(0.4f, 0.4f, 0.45f, 1.0f),
                  sunIntensity(1.0f), moonIntensity(0.3f) {}

    void updateCelestialBodies();
};

// Wind
struct Wind {
    Vec3 direction;
    float strength;
    float turbulence;
    float gustFrequency;

    bool enabled;

    Wind() : direction(1, 0, 0), strength(5.0f),
             turbulence(0.5f), gustFrequency(2.0f),
             enabled(true) {}

    Vec3 getWindForce(const Vec3& position, float time) const;
    Vec3 getWindVelocity(float time) const;
};

// Rain configuration
struct RainConfig {
    float intensity;      // 0-1 (light to heavy)
    float dropSize;
    float dropSpeed;
    int particlesPerSecond;
    Vec3 spawnArea;       // Area around camera
    Color rainColor;

    bool enableSplashes;
    bool enableRipples;
    bool enableWetSurfaces;

    RainConfig() : intensity(0.5f), dropSize(0.05f), dropSpeed(10.0f),
                   particlesPerSecond(500), spawnArea(50, 30, 50),
                   rainColor(0.7f, 0.7f, 0.8f, 0.6f),
                   enableSplashes(true), enableRipples(true),
                   enableWetSurfaces(true) {}
};

// Snow configuration
struct SnowConfig {
    float intensity;      // 0-1
    float flakeSize;
    float fallSpeed;
    int particlesPerSecond;
    Vec3 spawnArea;
    Color snowColor;

    bool enableAccumulation;
    float accumulationRate;

    SnowConfig() : intensity(0.5f), flakeSize(0.1f), fallSpeed(2.0f),
                   particlesPerSecond(300), spawnArea(50, 30, 50),
                   snowColor(1.0f, 1.0f, 1.0f, 0.9f),
                   enableAccumulation(true), accumulationRate(0.01f) {}
};

// Fog configuration
struct FogConfig {
    float density;
    float start;
    float end;
    Color color;
    bool volumetric;

    FogConfig() : density(0.02f), start(10.0f), end(100.0f),
                  color(0.7f, 0.7f, 0.75f, 1.0f), volumetric(false) {}
};

// Lightning
struct Lightning {
    Vec3 position;
    Vec3 direction;
    float duration;
    float age;
    float intensity;
    Color color;
    bool active;

    std::vector<Vec3> bolts;  // Lightning path

    Lightning() : duration(0.2f), age(0.0f), intensity(1.0f),
                  color(0.8f, 0.9f, 1.0f, 1.0f), active(false) {}

    void generate(const Vec3& start, const Vec3& end, int segments = 10);
    void update(float deltaTime);
    void render();
};

// Cloud system
struct CloudLayer {
    float altitude;
    float speed;
    float coverage;
    float density;
    Color color;
    GLuint texture;

    CloudLayer() : altitude(500.0f), speed(1.0f), coverage(0.5f),
                   density(0.8f), color(1.0f, 1.0f, 1.0f, 0.8f),
                   texture(0) {}
};

// Weather preset
struct WeatherPreset {
    WeatherType type;
    TimeOfDay timeOfDay;
    Wind wind;
    RainConfig rain;
    SnowConfig snow;
    FogConfig fog;
    std::vector<CloudLayer> clouds;

    float transitionDuration;

    WeatherPreset() : type(WeatherType::CLEAR), transitionDuration(5.0f) {}

    static WeatherPreset createClear();
    static WeatherPreset createRain();
    static WeatherPreset createSnow();
    static WeatherPreset createFog();
    static WeatherPreset createStorm();
    static WeatherPreset createSandstorm();
    static WeatherPreset createBlizzard();
};

// Weather system
class WeatherSystem {
public:
    WeatherSystem();
    ~WeatherSystem();

    bool initialize(ParticleSystem* particleSystem);
    void shutdown();

    // Update and render
    void update(float deltaTime);
    void render(const Camera& camera);

    // Weather control
    void setWeather(const WeatherPreset& preset);
    void setWeatherType(WeatherType type);
    void transitionToWeather(const WeatherPreset& preset, float duration);

    WeatherType getCurrentWeather() const { return currentPreset.type; }
    const WeatherPreset& getCurrentPreset() const { return currentPreset; }

    // Time of day
    void setTimeOfDay(float hour);
    float getTimeOfDay() const { return timeOfDay.hour; }
    void setTimeScale(float scale) { timeOfDay.timeScale = scale; }
    TimeOfDay& getTimeOfDayConfig() { return timeOfDay; }

    // Wind
    void setWind(const Vec3& direction, float strength);
    Vec3 getWindForce(const Vec3& position) const;
    Wind& getWind() { return wind; }

    // Rain
    void setRainIntensity(float intensity);
    float getRainIntensity() const { return rain.intensity; }
    bool isRaining() const { return rain.intensity > 0.0f; }
    RainConfig& getRainConfig() { return rain; }

    // Snow
    void setSnowIntensity(float intensity);
    float getSnowIntensity() const { return snow.intensity; }
    bool isSnowing() const { return snow.intensity > 0.0f; }
    SnowConfig& getSnowConfig() { return snow; }

    // Fog
    void setFogDensity(float density);
    float getFogDensity() const { return fog.density; }
    FogConfig& getFogConfig() { return fog; }

    // Lightning
    void triggerLightning(const Vec3& position);
    void setLightningFrequency(float frequency) { lightningFrequency = frequency; }

    // Clouds
    void addCloudLayer(const CloudLayer& layer);
    void removeCloudLayer(int index);
    void clearCloudLayers();
    std::vector<CloudLayer>& getCloudLayers() { return clouds; }

    // Environment effects
    void setTemperature(float temp) { temperature = temp; }
    float getTemperature() const { return temperature; }
    void setHumidity(float humid) { humidity = humid; }
    float getHumidity() const { return humidity; }

    // Audio
    void setAmbientSound(const std::string& soundPath);
    void setThunderSound(const std::string& soundPath);
    void setRainSound(const std::string& soundPath);
    void setWindSound(const std::string& soundPath);

    // Debug
    void setDebugDraw(bool enabled) { debugDraw = enabled; }
    void debugDrawInfo();

private:
    // Rendering
    void renderRain();
    void renderSnow();
    void renderClouds();
    void renderSkybox();

    // Updates
    void updateTimeOfDay(float deltaTime);
    void updateWind(float deltaTime);
    void updateRain(float deltaTime);
    void updateSnow(float deltaTime);
    void updateLightning(float deltaTime);

    // Transitions
    void updateTransition(float deltaTime);

    // Particle system
    ParticleSystem* particleSystem;
    ParticleEmitter* rainEmitter;
    ParticleEmitter* snowEmitter;

    // Current state
    WeatherPreset currentPreset;
    WeatherPreset targetPreset;
    float transitionProgress;
    bool transitioning;

    // Components
    TimeOfDay timeOfDay;
    Wind wind;
    RainConfig rain;
    SnowConfig snow;
    FogConfig fog;
    std::vector<CloudLayer> clouds;
    std::vector<std::unique_ptr<Lightning>> lightningBolts;

    // Lightning
    float lightningFrequency;
    float lightningTimer;

    // Environment
    float temperature;  // Celsius
    float humidity;     // 0-1

    // Audio
    std::string ambientSoundPath;
    std::string thunderSoundPath;
    std::string rainSoundPath;
    std::string windSoundPath;

    // Time
    float totalTime;

    // Debug
    bool debugDraw;
};
