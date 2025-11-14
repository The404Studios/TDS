#pragma once

#include "raylib.h"
#include <map>
#include <string>

namespace TDS {

class Game;

class AudioManager {
public:
    AudioManager(Game* game);
    ~AudioManager();

    void loadDefaultSounds();
    void loadSound(const std::string& name, const char* path);
    void loadMusic(const std::string& name, const char* path);

    void playSound(const std::string& name, float volume = 1.0f);
    void playMusic(const std::string& name, float volume = 0.5f);
    void stopMusic();
    void updateMusic();

    void unloadAll();

private:
    Game* game;
    std::map<std::string, Sound> sounds;
    std::map<std::string, Music> music;
    std::string currentMusic;
};

} // namespace TDS
