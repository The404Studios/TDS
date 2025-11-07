#pragma once

#include "../core/Platform.h"
#include <vector>
#include <memory>
#include <cmath>

// Math structures
struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    float dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3 cross(const Vec3& v) const {
        return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }
    float length() const { return std::sqrtf(x * x + y * y + z * z); }
    Vec3 normalized() const {
        float len = length();
        return len > 0.0f ? Vec3(x / len, y / len, z / len) : Vec3(0.0f, 0.0f, 0.0f);
    }
};

struct Vec4 {
    float x, y, z, w;
    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vec4(const Vec3& v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}
};

struct Mat4 {
    float m[16];

    Mat4() { identity(); }

    void identity() {
        for (int i = 0; i < 16; i++) m[i] = 0;
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    static Mat4 perspective(float fov, float aspect, float near, float far);
    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up);
    static Mat4 translate(const Vec3& v);
    static Mat4 rotate(float angle, const Vec3& axis);
    static Mat4 scale(const Vec3& s);

    Mat4 operator*(const Mat4& other) const;
    Vec4 operator*(const Vec4& v) const;
};

// Color structure
struct Color {
    float r, g, b, a;
    Color() : r(1), g(1), b(1), a(1) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
};

// Light types
enum class LightType {
    DIRECTIONAL,
    POINT,
    SPOT,
    AREA
};

// Light structure
struct Light {
    LightType type;
    Vec3 position;
    Vec3 direction;
    Color color;
    float intensity;

    // Attenuation (for point/spot lights)
    float constant;
    float linear;
    float quadratic;

    // Spot light specific
    float innerCone;  // In radians
    float outerCone;  // In radians

    // Shadow casting
    bool castShadows;
    int shadowMapSize;
    GLuint shadowMapTexture;
    Mat4 shadowMatrix;

    Light() : type(LightType::DIRECTIONAL), position(), direction(), color(),
              intensity(1.0f), constant(1.0f), linear(0.09f), quadratic(0.032f),
              innerCone(0.9f), outerCone(0.95f), castShadows(true),
              shadowMapSize(1024), shadowMapTexture(0), shadowMatrix() {}
};

// Material properties
struct Material {
    Color albedo;
    Color emissive;
    float metallic;
    float roughness;
    float ao;  // Ambient occlusion

    // Textures
    GLuint albedoMap;
    GLuint normalMap;
    GLuint metallicMap;
    GLuint roughnessMap;
    GLuint aoMap;

    Material() : metallic(0.0f), roughness(0.5f), ao(1.0f),
                 albedoMap(0), normalMap(0), metallicMap(0),
                 roughnessMap(0), aoMap(0) {}
};

// Mesh data
struct Mesh {
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<Vec4> tangents;
    std::vector<Vec3> texCoords;
    std::vector<unsigned int> indices;

    // OpenGL buffers
    GLuint vao;
    GLuint vbo;
    GLuint nbo;
    GLuint tbo;
    GLuint ebo;

    // Material
    Material material;

    Mesh() : vao(0), vbo(0), nbo(0), tbo(0), ebo(0) {}

    void uploadToGPU();
    void render();
    void cleanup();
};

// Camera
struct Camera {
    Vec3 position;
    Vec3 target;
    Vec3 up;

    float fov;
    float nearPlane;
    float farPlane;
    float aspectRatio;

    Mat4 viewMatrix;
    Mat4 projectionMatrix;
    Mat4 viewProjectionMatrix;

    Camera() : position(0, 0, 5), target(0, 0, 0), up(0, 1, 0),
               fov(60.0f), nearPlane(0.1f), farPlane(1000.0f),
               aspectRatio(16.0f / 9.0f) {}

    void updateMatrices();
    void lookAt(const Vec3& eye, const Vec3& center, const Vec3& upVec);
    void setPerspective(float fovDeg, float aspect, float near, float far);
};

// Skybox
struct Skybox {
    GLuint cubemapTexture;
    Mesh mesh;
    Color tint;
    float brightness;

    Skybox() : cubemapTexture(0), brightness(1.0f) {}

    void render(const Camera& camera);
};

// Render queue entry
struct RenderQueueEntry {
    Mesh* mesh;
    Mat4 transform;
    float distanceToCamera;
    bool transparent;

    bool operator<(const RenderQueueEntry& other) const {
        if (transparent != other.transparent) {
            return !transparent;  // Opaque first
        }
        if (transparent) {
            return distanceToCamera > other.distanceToCamera;  // Back to front for transparent
        }
        return distanceToCamera < other.distanceToCamera;  // Front to back for opaque
    }
};

// Rendering engine
class RenderEngine {
public:
    RenderEngine();
    ~RenderEngine();

    bool initialize(HWND hwnd, int width, int height, int samples);
    void shutdown();

    // Frame rendering
    void beginFrame();
    void endFrame();
    void swapBuffers();

    // Rendering
    void renderScene();
    void renderMesh(Mesh* mesh, const Mat4& transform);
    void submitToRenderQueue(Mesh* mesh, const Mat4& transform, bool transparent = false);

    // Lighting
    Light* createLight(LightType type);
    void removeLight(Light* light);
    void updateLighting();
    void renderShadows();

    // Camera
    Camera& getCamera() { return camera; }
    void setCamera(const Camera& cam) { camera = cam; }

    // Skybox
    void setSkybox(const Skybox& sky) { skybox = sky; }
    Skybox& getSkybox() { return skybox; }

    // Settings
    void setAmbientLight(const Color& color, float intensity);
    void setFog(bool enabled, const Color& color, float density, float start, float end);
    void setWireframe(bool enabled);
    void setBackfaceCulling(bool enabled);

    // Post-processing
    void enableBloom(bool enabled);
    void enableSSAO(bool enabled);
    void enableMotionBlur(bool enabled);
    void setExposure(float exposure);
    void setGamma(float gamma);

    // Debug
    void renderDebugInfo();
    void setDebugMode(bool enabled) { debugMode = enabled; }

    // Stats
    int getDrawCalls() const { return drawCalls; }
    int getTrianglesRendered() const { return trianglesRendered; }
    void resetStats();

private:
    void initializeOpenGL();
    void createFramebuffers();
    void renderToGBuffer();
    void renderLightingPass();
    void renderPostProcessing();
    void sortRenderQueue();

    // OpenGL context
    HDC hdc;
    HGLRC hglrc;
    HWND hwnd;
    int width, height;

    // Camera
    Camera camera;

    // Lights
    std::vector<std::unique_ptr<Light>> lights;
    Color ambientColor;
    float ambientIntensity;

    // Skybox
    Skybox skybox;

    // Fog
    bool fogEnabled;
    Color fogColor;
    float fogDensity;
    float fogStart;
    float fogEnd;

    // Render queue
    std::vector<RenderQueueEntry> renderQueue;

    // Framebuffers
    GLuint gBuffer;  // G-Buffer for deferred rendering
    GLuint gPosition, gNormal, gAlbedo, gMaterial;
    GLuint depthBuffer;
    GLuint hdrFBO;
    GLuint hdrColorBuffer;
    GLuint postProcessFBO;
    GLuint postProcessTexture;

    // Settings
    bool wireframeMode;
    bool backfaceCulling;
    bool bloomEnabled;
    bool ssaoEnabled;
    bool motionBlurEnabled;
    float exposure;
    float gamma;
    bool debugMode;

    // Stats
    int drawCalls;
    int trianglesRendered;
};
