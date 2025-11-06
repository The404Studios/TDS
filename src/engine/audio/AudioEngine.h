#pragma once

#include "../rendering/RenderEngine.h"
#include "../resources/ResourceManager.h"
#include <vector>
#include <memory>
#include <string>

// Audio source state
enum class AudioState {
    STOPPED,
    PLAYING,
    PAUSED
};

// Audio source
class AudioSource {
public:
    AudioSource();
    ~AudioSource();

    // Playback
    void play();
    void pause();
    void stop();
    void setLoop(bool loop) { looping = loop; }
    bool isPlaying() const { return state == AudioState::PLAYING; }
    bool isPaused() const { return state == AudioState::PAUSED; }

    // Clip
    void setClip(ResourceHandle<AudioClip> clip) { this->clip = clip; }
    ResourceHandle<AudioClip> getClip() const { return clip; }

    // Volume
    void setVolume(float vol) { volume = std::max(0.0f, std::min(1.0f, vol)); }
    float getVolume() const { return volume; }

    // Pitch
    void setPitch(float p) { pitch = std::max(0.1f, std::min(3.0f, p)); }
    float getPitch() const { return pitch; }

    // 3D Audio
    void set3D(bool enabled) { is3D = enabled; }
    bool is3DEnabled() const { return is3D; }
    void setPosition(const Vec3& pos) { position = pos; }
    Vec3 getPosition() const { return position; }
    void setVelocity(const Vec3& vel) { velocity = vel; }
    Vec3 getVelocity() const { return velocity; }

    // Distance attenuation
    void setMinDistance(float dist) { minDistance = dist; }
    float getMinDistance() const { return minDistance; }
    void setMaxDistance(float dist) { maxDistance = dist; }
    float getMaxDistance() const { return maxDistance; }
    void setRolloffFactor(float factor) { rolloffFactor = factor; }
    float getRolloffFactor() const { return rolloffFactor; }

    // Panning
    void setPan(float p) { pan = std::max(-1.0f, std::min(1.0f, p)); }
    float getPan() const { return pan; }

    // Priority
    void setPriority(int p) { priority = p; }
    int getPriority() const { return priority; }

    // Time
    float getCurrentTime() const { return currentTime; }
    void setCurrentTime(float time) { currentTime = time; }
    float getDuration() const;

    // Effects
    void setReverb(float amount) { reverbAmount = amount; }
    void setLowPassFilter(float cutoff) { lowPassCutoff = cutoff; }
    void setHighPassFilter(float cutoff) { highPassCutoff = cutoff; }

private:
    friend class AudioEngine;

    ResourceHandle<AudioClip> clip;
    AudioState state;

    float volume;
    float pitch;
    float pan;
    int priority;

    bool looping;
    bool is3D;
    Vec3 position;
    Vec3 velocity;

    float minDistance;
    float maxDistance;
    float rolloffFactor;

    float currentTime;
    int currentSample;

    // Effects
    float reverbAmount;
    float lowPassCutoff;
    float highPassCutoff;
};

// Audio listener (camera/player)
class AudioListener {
public:
    AudioListener();

    void setPosition(const Vec3& pos) { position = pos; }
    Vec3 getPosition() const { return position; }

    void setVelocity(const Vec3& vel) { velocity = vel; }
    Vec3 getVelocity() const { return velocity; }

    void setOrientation(const Vec3& forward, const Vec3& up) {
        this->forward = forward;
        this->up = up;
    }

    Vec3 getForward() const { return forward; }
    Vec3 getUp() const { return up; }

private:
    Vec3 position;
    Vec3 velocity;
    Vec3 forward;
    Vec3 up;
};

// Audio mixer channel
struct MixerChannel {
    std::string name;
    float volume;
    float pitch;
    bool muted;
    bool solo;

    MixerChannel() : volume(1.0f), pitch(1.0f), muted(false), solo(false) {}
};

// Reverb preset
enum class ReverbPreset {
    NONE,
    ROOM,
    HALL,
    CATHEDRAL,
    CAVE,
    ARENA,
    UNDERWATER,
    CUSTOM
};

// Audio engine
class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    bool initialize(int sampleRate = 44100, int channels = 2, int bufferSize = 1024);
    void shutdown();

    // Update (mix audio)
    void update(float deltaTime);

    // Source management
    AudioSource* createSource();
    void removeSource(AudioSource* source);
    void clearSources();

    // Quick play
    void playOneShot(ResourceHandle<AudioClip> clip, float volume = 1.0f);
    void playOneShotAt(ResourceHandle<AudioClip> clip, const Vec3& position, float volume = 1.0f);

    // Music
    void playMusic(ResourceHandle<AudioClip> clip, float volume = 0.5f, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    void setMusicVolume(float volume);

    // Listener
    AudioListener& getListener() { return listener; }
    void setListenerPosition(const Vec3& pos) { listener.setPosition(pos); }

    // Master controls
    void setMasterVolume(float volume);
    float getMasterVolume() const { return masterVolume; }
    void setMute(bool muted) { this->muted = muted; }
    bool isMuted() const { return muted; }

    // Mixer channels
    MixerChannel* createChannel(const std::string& name);
    MixerChannel* getChannel(const std::string& name);
    void removeChannel(const std::string& name);

    // Effects
    void setGlobalReverb(ReverbPreset preset);
    void setDopplerFactor(float factor) { dopplerFactor = factor; }
    void setSpeedOfSound(float speed) { speedOfSound = speed; }

    // Distance model
    enum class DistanceModel {
        INVERSE,
        LINEAR,
        EXPONENTIAL
    };
    void setDistanceModel(DistanceModel model) { distanceModel = model; }

    // Stats
    int getActiveSources() const { return (int)sources.size(); }
    int getPlayingSources() const;

private:
    // Mixing
    void mixAudio(int16_t* outputBuffer, int frames);
    void mixSource(AudioSource* source, int16_t* buffer, int frames);
    void apply3D(AudioSource* source, float& leftGain, float& rightGain);
    void applyEffects(AudioSource* source, int16_t* buffer, int frames);
    void applyReverb(int16_t* buffer, int frames, float amount);
    void applyFilter(int16_t* buffer, int frames, float cutoff, bool highPass);

    // Spatial audio
    float calculateDistance(const Vec3& sourcePos, const Vec3& listenerPos);
    float calculateAttenuation(AudioSource* source, float distance);
    Vec3 calculateRelativePosition(const Vec3& sourcePos);

    // Sources
    std::vector<std::unique_ptr<AudioSource>> sources;
    AudioSource* musicSource;

    // Listener
    AudioListener listener;

    // Audio device
    int sampleRate;
    int channels;
    int bufferSize;

    // Master controls
    float masterVolume;
    bool muted;

    // Mixer
    std::map<std::string, MixerChannel> mixerChannels;

    // Effects
    float dopplerFactor;
    float speedOfSound;
    DistanceModel distanceModel;
    ReverbPreset reverbPreset;

    // Reverb buffer
    std::vector<float> reverbBuffer;
    int reverbIndex;

    // Platform specific
#ifdef _WIN32
    void* deviceHandle;  // Windows audio device
#endif
};
