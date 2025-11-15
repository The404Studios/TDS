// Sound System for Extraction Shooter
// Handles all game audio with raylib

#pragma once

#include "raylib.h"
#include <unordered_map>
#include <string>

enum class SoundType {
    // Weapons
    GUNSHOT_AK47,
    GUNSHOT_M4A1,
    GUNSHOT_PISTOL,
    GUNSHOT_SNIPER,
    RELOAD,
    WEAPON_SWITCH,

    // Combat
    BULLET_IMPACT_FLESH,
    BULLET_IMPACT_METAL,
    BULLET_IMPACT_CONCRETE,
    PLAYER_HURT,
    PLAYER_DEATH,

    // Movement
    FOOTSTEP_CONCRETE,
    FOOTSTEP_GRASS,
    FOOTSTEP_METAL,
    JUMP,
    LAND,

    // Interaction
    LOOT_PICKUP,
    LOOT_OPEN,
    DOOR_OPEN,
    DOOR_CLOSE,

    // UI
    MENU_CLICK,
    MENU_HOVER,
    EQUIP_ITEM,
    DROP_ITEM,

    // Environment
    EXTRACTION_ARRIVED,
    EXTRACTION_COUNTDOWN,
    RAID_TIMER_WARNING,

    // Special
    AMBIENT_WIND,
    AMBIENT_RAIN
};

class SoundSystem {
private:
    std::unordered_map<SoundType, Sound> sounds;
    std::unordered_map<SoundType, Music> music;

    bool initialized;
    float masterVolume;
    float sfxVolume;
    float musicVolume;

    // For 3D sound positioning
    Vector3 listenerPosition;
    Vector3 listenerForward;

public:
    SoundSystem()
        : initialized(false)
        , masterVolume(1.0f)
        , sfxVolume(0.7f)
        , musicVolume(0.5f)
        , listenerPosition({0, 0, 0})
        , listenerForward({0, 0, 1})
    {
    }

    ~SoundSystem() {
        cleanup();
    }

    // Initialize sound system
    bool initialize() {
        if (initialized) return true;

        // Audio device is initialized in main
        loadSounds();

        initialized = true;
        TraceLog(LOG_INFO, "[SoundSystem] Sound system initialized");
        return true;
    }

    // Load all sounds
    void loadSounds() {
        TraceLog(LOG_INFO, "[SoundSystem] Loading sounds...");

        // Try to load custom sounds, fallback to generated sounds

        // Gunshots
        loadSound(SoundType::GUNSHOT_AK47, "resources/sounds/ak47_shot.wav", 440.0f, 0.1f);
        loadSound(SoundType::GUNSHOT_M4A1, "resources/sounds/m4a1_shot.wav", 480.0f, 0.1f);
        loadSound(SoundType::GUNSHOT_PISTOL, "resources/sounds/pistol_shot.wav", 400.0f, 0.08f);
        loadSound(SoundType::GUNSHOT_SNIPER, "resources/sounds/sniper_shot.wav", 350.0f, 0.15f);
        loadSound(SoundType::RELOAD, "resources/sounds/reload.wav", 300.0f, 0.5f);

        // Combat
        loadSound(SoundType::BULLET_IMPACT_FLESH, "resources/sounds/impact_flesh.wav", 250.0f, 0.1f);
        loadSound(SoundType::BULLET_IMPACT_METAL, "resources/sounds/impact_metal.wav", 600.0f, 0.08f);
        loadSound(SoundType::PLAYER_HURT, "resources/sounds/hurt.wav", 200.0f, 0.3f);
        loadSound(SoundType::PLAYER_DEATH, "resources/sounds/death.wav", 150.0f, 1.0f);

        // Movement
        loadSound(SoundType::FOOTSTEP_CONCRETE, "resources/sounds/footstep.wav", 300.0f, 0.2f);
        loadSound(SoundType::JUMP, "resources/sounds/jump.wav", 350.0f, 0.15f);
        loadSound(SoundType::LAND, "resources/sounds/land.wav", 280.0f, 0.2f);

        // Interaction
        loadSound(SoundType::LOOT_PICKUP, "resources/sounds/loot_pickup.wav", 500.0f, 0.3f);
        loadSound(SoundType::LOOT_OPEN, "resources/sounds/loot_open.wav", 400.0f, 0.4f);

        // UI
        loadSound(SoundType::MENU_CLICK, "resources/sounds/menu_click.wav", 800.0f, 0.1f);
        loadSound(SoundType::EQUIP_ITEM, "resources/sounds/equip.wav", 450.0f, 0.2f);

        // Special
        loadSound(SoundType::EXTRACTION_ARRIVED, "resources/sounds/extraction.wav", 600.0f, 0.5f);

        TraceLog(LOG_INFO, "[SoundSystem] Sounds loaded");
    }

    // Load a single sound (with fallback to generated sound)
    void loadSound(SoundType type, const std::string& path, float frequency, float duration) {
        if (FileExists(path.c_str())) {
            sounds[type] = LoadSound(path.c_str());
            TraceLog(LOG_INFO, "[SoundSystem] Loaded sound: %s", path.c_str());
        } else {
            // Generate placeholder sound using raylib's wave generation
            Wave wave = GenWaveNoise(frequency, 1, duration);
            sounds[type] = LoadSoundFromWave(wave);
            UnloadWave(wave);
            TraceLog(LOG_INFO, "[SoundSystem] Generated placeholder for: %s", path.c_str());
        }
    }

    // Play a sound
    void play(SoundType type, float volume = 1.0f) {
        auto it = sounds.find(type);
        if (it != sounds.end()) {
            SetSoundVolume(it->second, volume * sfxVolume * masterVolume);
            PlaySound(it->second);
        }
    }

    // Play sound at 3D position (volume based on distance)
    void play3D(SoundType type, Vector3 position, float maxDistance = 50.0f, float volume = 1.0f) {
        auto it = sounds.find(type);
        if (it != sounds.end()) {
            // Calculate distance from listener
            float distance = Vector3Distance(listenerPosition, position);

            // Calculate volume based on distance (inverse square law approximation)
            float distanceAttenuation = 1.0f - (distance / maxDistance);
            distanceAttenuation = distanceAttenuation < 0 ? 0 : distanceAttenuation;

            float finalVolume = volume * sfxVolume * masterVolume * distanceAttenuation;

            SetSoundVolume(it->second, finalVolume);
            PlaySound(it->second);
        }
    }

    // Play looping sound
    void playLooping(SoundType type, float volume = 1.0f) {
        auto it = sounds.find(type);
        if (it != sounds.end()) {
            SetSoundVolume(it->second, volume * sfxVolume * masterVolume);
            PlaySound(it->second);
            // Note: raylib doesn't have built-in looping for Sound, use Music for that
        }
    }

    // Stop a sound
    void stop(SoundType type) {
        auto it = sounds.find(type);
        if (it != sounds.end()) {
            StopSound(it->second);
        }
    }

    // Update listener position (for 3D audio)
    void setListenerPosition(Vector3 position, Vector3 forward) {
        listenerPosition = position;
        listenerForward = forward;
    }

    // Volume controls
    void setMasterVolume(float volume) {
        masterVolume = volume < 0 ? 0 : (volume > 1 ? 1 : volume);
    }

    void setSFXVolume(float volume) {
        sfxVolume = volume < 0 ? 0 : (volume > 1 ? 1 : volume);
    }

    void setMusicVolume(float volume) {
        musicVolume = volume < 0 ? 0 : (volume > 1 ? 1 : volume);
    }

    float getMasterVolume() const { return masterVolume; }
    float getSFXVolume() const { return sfxVolume; }
    float getMusicVolume() const { return musicVolume; }

    // Cleanup
    void cleanup() {
        if (!initialized) return;

        for (auto& pair : sounds) {
            UnloadSound(pair.second);
        }
        sounds.clear();

        for (auto& pair : music) {
            UnloadMusicStream(pair.second);
        }
        music.clear();

        initialized = false;
        TraceLog(LOG_INFO, "[SoundSystem] Sound system cleaned up");
    }
};

// Helper: Play footstep sound with randomized pitch for variation
inline void playFootstep(SoundSystem& soundSystem, SoundType footstepType) {
    static int footstepCounter = 0;

    // Only play every other footstep (left/right alternation)
    if (footstepCounter++ % 2 == 0) {
        soundSystem.play(footstepType, 0.5f);
    }
}

// Helper: Play gunshot with muzzle flash effect callback
inline void playGunshot(SoundSystem& soundSystem, SoundType weaponType, Vector3 position) {
    soundSystem.play3D(weaponType, position, 100.0f, 0.9f);
}
