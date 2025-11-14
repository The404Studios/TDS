#include "AudioManager.h"
#include "../Game.h"
#include "raylib.h"
#include <iostream>

namespace TDS {

AudioManager::AudioManager(Game* game) : game(game) {
    loadDefaultSounds();
}

AudioManager::~AudioManager() {
    unloadAll();
}

void AudioManager::loadDefaultSounds() {
    std::cout << "[AudioManager] Loading sounds..." << std::endl;

    // Load weapon sounds
    loadSound("gunshot", "assets/sounds/weapons/ak47_fire.ogg");
    loadSound("reload", "assets/sounds/weapons/reload.ogg");
    loadSound("empty", "assets/sounds/weapons/empty_click.ogg");

    // Load footsteps
    loadSound("footstep_concrete", "assets/sounds/footsteps/concrete_1.ogg");
    loadSound("footstep_grass", "assets/sounds/footsteps/grass_1.ogg");

    // Load ambient
    loadSound("ambient_wind", "assets/sounds/ambient/wind.ogg");

    // Load music
    loadMusic("menu_music", "assets/music/menu_theme.ogg");
    loadMusic("combat_music", "assets/music/combat_music.ogg");

    std::cout << "[AudioManager] Loaded " << sounds.size() << " sounds, " 
              << music.size() << " music tracks" << std::endl;
}

void AudioManager::loadSound(const std::string& name, const char* path) {
    if (FileExists(path)) {
        Sound sound = LoadSound(path);
        sounds[name] = sound;
        std::cout << "[AudioManager] Loaded sound: " << name << std::endl;
    } else {
        std::cout << "[AudioManager] Sound not found: " << path << std::endl;
    }
}

void AudioManager::loadMusic(const std::string& name, const char* path) {
    if (FileExists(path)) {
        Music musicTrack = LoadMusicStream(path);
        music[name] = musicTrack;
        std::cout << "[AudioManager] Loaded music: " << name << std::endl;
    } else {
        std::cout << "[AudioManager] Music not found: " << path << std::endl;
    }
}

void AudioManager::playSound(const std::string& name, float volume) {
    auto it = sounds.find(name);
    if (it != sounds.end()) {
        SetSoundVolume(it->second, volume);
        PlaySound(it->second);
    }
}

void AudioManager::playMusic(const std::string& name, float volume) {
    // Stop current music
    stopMusic();

    auto it = music.find(name);
    if (it != music.end()) {
        SetMusicVolume(it->second, volume);
        PlayMusicStream(it->second);
        currentMusic = name;
    }
}

void AudioManager::stopMusic() {
    if (!currentMusic.empty()) {
        auto it = music.find(currentMusic);
        if (it != music.end()) {
            StopMusicStream(it->second);
        }
        currentMusic.clear();
    }
}

void AudioManager::updateMusic() {
    if (!currentMusic.empty()) {
        auto it = music.find(currentMusic);
        if (it != music.end()) {
            UpdateMusicStream(it->second);
        }
    }
}

void AudioManager::unloadAll() {
    for (auto& pair : sounds) {
        UnloadSound(pair.second);
    }
    sounds.clear();

    for (auto& pair : music) {
        UnloadMusicStream(pair.second);
    }
    music.clear();

    std::cout << "[AudioManager] All audio unloaded" << std::endl;
}

} // namespace TDS
