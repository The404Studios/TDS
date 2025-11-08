#pragma once

#include "../rendering/RenderEngine.h"
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <functional>

// Resource types
enum class ResourceType {
    TEXTURE,
    MODEL,
    AUDIO,
    SHADER,
    MATERIAL,
    ANIMATION,
    FONT
};

// Texture data
struct Texture {
    GLuint id;
    int width;
    int height;
    int channels;
    std::string path;

    Texture() : id(0), width(0), height(0), channels(0) {}
};

// Model data (OBJ/FBX)
struct Model {
    std::vector<Mesh> meshes;
    std::string name;
    std::string path;

    Vec3 boundingBoxMin;
    Vec3 boundingBoxMax;

    Model() {}

    void calculateBounds();
    void render();
    void cleanup();
};

// Audio clip
struct AudioClip {
    std::string path;
    std::vector<int16_t> samples;
    int sampleRate;
    int channels;
    float duration;

    AudioClip() : sampleRate(44100), channels(2), duration(0.0f) {}
};

// Shader program
struct ShaderProgram {
    GLuint program;
    GLuint vertexShader;
    GLuint fragmentShader;
    std::string name;

    std::map<std::string, GLint> uniforms;

    ShaderProgram() : program(0), vertexShader(0), fragmentShader(0) {}

    void use();
    void setUniform(const std::string& name, float value);
    void setUniform(const std::string& name, const Vec3& value);
    void setUniform(const std::string& name, const Mat4& value);
    void setUniform(const std::string& name, int value);
};

// Font data
struct Font {
    std::string path;
    int size;
    GLuint textureAtlas;
    struct Glyph {
        int x, y, width, height;
        int xOffset, yOffset;
        int xAdvance;
    };
    std::map<char, Glyph> glyphs;

    Font() : size(16), textureAtlas(0) {}
};

// Animation data
struct AnimationClip {
    std::string name;
    float duration;
    float ticksPerSecond;

    struct BoneAnimation {
        std::string boneName;
        std::vector<Vec3> positions;
        std::vector<Vec4> rotations;  // Quaternions
        std::vector<Vec3> scales;
        std::vector<float> positionTimestamps;
        std::vector<float> rotationTimestamps;
        std::vector<float> scaleTimestamps;
    };

    std::vector<BoneAnimation> boneAnimations;

    AnimationClip() : duration(0.0f), ticksPerSecond(25.0f) {}
};

// Resource handle (reference counted)
template<typename T>
class ResourceHandle {
public:
    ResourceHandle() : resource(nullptr), refCount(new int(0)) {}
    ResourceHandle(T* res) : resource(res), refCount(new int(1)) {}

    ResourceHandle(const ResourceHandle& other)
        : resource(other.resource), refCount(other.refCount) {
        if (refCount) (*refCount)++;
    }

    ~ResourceHandle() {
        if (refCount && --(*refCount) == 0) {
            delete resource;
            delete refCount;
        }
    }

    ResourceHandle& operator=(const ResourceHandle& other) {
        if (this != &other) {
            if (refCount && --(*refCount) == 0) {
                delete resource;
                delete refCount;
            }
            resource = other.resource;
            refCount = other.refCount;
            if (refCount) (*refCount)++;
        }
        return *this;
    }

    T* get() { return resource; }
    const T* get() const { return resource; }
    T* operator->() { return resource; }
    const T* operator->() const { return resource; }
    bool isValid() const { return resource != nullptr; }

private:
    T* resource;
    int* refCount;
};

// Resource manager
class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    bool initialize();
    void shutdown();

    // Texture loading
    ResourceHandle<Texture> loadTexture(const std::string& path);
    ResourceHandle<Texture> loadTextureFromMemory(const uint8_t* data, int width, int height, int channels);
    ResourceHandle<Texture> createTexture(int width, int height, int channels, const void* data = nullptr);
    void unloadTexture(const std::string& path);

    // Model loading
    ResourceHandle<Model> loadModel(const std::string& path);
    ResourceHandle<Model> loadOBJ(const std::string& path);
    ResourceHandle<Model> loadFBX(const std::string& path);
    void unloadModel(const std::string& path);

    // Audio loading
    ResourceHandle<AudioClip> loadAudio(const std::string& path);
    ResourceHandle<AudioClip> loadWAV(const std::string& path);
    ResourceHandle<AudioClip> loadOGG(const std::string& path);
    void unloadAudio(const std::string& path);

    // Shader loading
    ResourceHandle<ShaderProgram> loadShader(const std::string& vertexPath, const std::string& fragmentPath);
    ResourceHandle<ShaderProgram> compileShader(const std::string& vertexSource, const std::string& fragmentSource);
    void unloadShader(const std::string& name);

    // Font loading
    ResourceHandle<Font> loadFont(const std::string& path, int size = 16);
    void unloadFont(const std::string& path);

    // Animation loading
    ResourceHandle<AnimationClip> loadAnimation(const std::string& path);
    void unloadAnimation(const std::string& path);

    // Resource queries
    bool isLoaded(const std::string& path, ResourceType type);
    int getLoadedResourceCount(ResourceType type);
    size_t getTotalMemoryUsage();

    // Hot reloading
    void enableHotReload(bool enabled) { hotReloadEnabled = enabled; }
    void checkForReloads();

    // Async loading
    void loadAsync(const std::string& path, ResourceType type,
                   std::function<void(bool success)> callback);
    bool isLoadingAsync() const { return asyncLoadingInProgress; }

    // Cache management
    void clearCache(ResourceType type);
    void clearAllCaches();
    void setMaxCacheSize(size_t bytes) { maxCacheSize = bytes; }

    // Built-in resources
    ResourceHandle<Texture> getWhiteTexture();
    ResourceHandle<Texture> getBlackTexture();
    ResourceHandle<Texture> getNormalMapDefault();
    ResourceHandle<Mesh> getCubeMesh();
    ResourceHandle<Mesh> getSphereMesh();
    ResourceHandle<Mesh> getPlaneMesh();
    ResourceHandle<ShaderProgram> getDefaultShader();

private:
    // Loaders
    Texture* loadTextureFile(const std::string& path);
    Texture* loadBMP(const std::string& path);
    Texture* loadTGA(const std::string& path);
    Texture* loadPNG(const std::string& path);

    Model* loadModelFile(const std::string& path);
    bool parseOBJ(const std::string& path, Model* model);
    bool parseFBX(const std::string& path, Model* model);

    AudioClip* loadAudioFile(const std::string& path);
    bool parseWAV(const std::string& path, AudioClip* clip);
    bool parseOGG(const std::string& path, AudioClip* clip);

    ShaderProgram* loadShaderFile(const std::string& vertPath, const std::string& fragPath);
    GLuint compileShaderSource(const std::string& source, GLenum type);
    bool linkShaderProgram(ShaderProgram* program);

    Font* loadFontFile(const std::string& path, int size);

    AnimationClip* loadAnimationFile(const std::string& path);

    // Utilities
    std::string readTextFile(const std::string& path);
    std::vector<uint8_t> readBinaryFile(const std::string& path);
    bool fileExists(const std::string& path);
    std::string getFileExtension(const std::string& path);
    uint64_t getFileModifiedTime(const std::string& path);

    // Caches
    std::map<std::string, Texture*> textureCache;
    std::map<std::string, Model*> modelCache;
    std::map<std::string, AudioClip*> audioCache;
    std::map<std::string, ShaderProgram*> shaderCache;
    std::map<std::string, Font*> fontCache;
    std::map<std::string, AnimationClip*> animationCache;

    // File watching for hot reload
    std::map<std::string, uint64_t> fileModTimes;
    bool hotReloadEnabled;

    // Async loading
    bool asyncLoadingInProgress;

    // Memory management
    size_t currentMemoryUsage;
    size_t maxCacheSize;

    // Built-in resources
    Texture* whiteTexture;
    Texture* blackTexture;
    Texture* normalMapDefault;
    Mesh* cubeMesh;
    Mesh* sphereMesh;
    Mesh* planeMesh;
    ShaderProgram* defaultShader;

    void createBuiltInResources();
    void cleanupBuiltInResources();
};
