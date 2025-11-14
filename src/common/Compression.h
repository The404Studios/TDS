#pragma once

#include <vector>
#include <cstdint>
#include <cstring>

namespace TDS {
namespace Compression {

// Simple run-length encoding for repeated values
inline std::vector<uint8_t> compressRLE(const uint8_t* data, size_t size) {
    std::vector<uint8_t> compressed;
    if (size == 0) return compressed;

    size_t i = 0;
    while (i < size) {
        uint8_t value = data[i];
        uint8_t count = 1;

        // Count repeating bytes
        while (i + count < size && data[i + count] == value && count < 255) {
            count++;
        }

        if (count > 3 || value == 0xFF) {
            // Use RLE marker (0xFF) for runs
            compressed.push_back(0xFF);
            compressed.push_back(count);
            compressed.push_back(value);
        } else {
            // Raw bytes for short runs
            for (uint8_t j = 0; j < count; j++) {
                compressed.push_back(value);
            }
        }

        i += count;
    }

    return compressed;
}

inline std::vector<uint8_t> decompressRLE(const uint8_t* data, size_t size) {
    std::vector<uint8_t> decompressed;

    size_t i = 0;
    while (i < size) {
        if (data[i] == 0xFF && i + 2 < size) {
            // RLE marker
            uint8_t count = data[i + 1];
            uint8_t value = data[i + 2];
            for (uint8_t j = 0; j < count; j++) {
                decompressed.push_back(value);
            }
            i += 3;
        } else {
            // Raw byte
            decompressed.push_back(data[i]);
            i++;
        }
    }

    return decompressed;
}

// Delta encoding for positions (compress consecutive similar values)
inline std::vector<int16_t> deltaEncode(const float* values, size_t count, float scale = 100.0f) {
    std::vector<int16_t> encoded;
    if (count == 0) return encoded;

    // First value is full
    int16_t prev = static_cast<int16_t>(values[0] * scale);
    encoded.push_back(prev);

    // Rest are deltas
    for (size_t i = 1; i < count; i++) {
        int16_t current = static_cast<int16_t>(values[i] * scale);
        int16_t delta = current - prev;
        encoded.push_back(delta);
        prev = current;
    }

    return encoded;
}

inline std::vector<float> deltaDecode(const int16_t* encoded, size_t count, float scale = 100.0f) {
    std::vector<float> decoded;
    if (count == 0) return decoded;

    // First value is full
    int16_t prev = encoded[0];
    decoded.push_back(static_cast<float>(prev) / scale);

    // Rest are deltas
    for (size_t i = 1; i < count; i++) {
        int16_t current = prev + encoded[i];
        decoded.push_back(static_cast<float>(current) / scale);
        prev = current;
    }

    return decoded;
}

// Quantize float to reduced precision
inline uint16_t quantizeFloat(float value, float min, float max, uint16_t levels = 65535) {
    float normalized = (value - min) / (max - min);
    normalized = std::max(0.0f, std::min(1.0f, normalized));
    return static_cast<uint16_t>(normalized * levels);
}

inline float dequantizeFloat(uint16_t quantized, float min, float max, uint16_t levels = 65535) {
    float normalized = static_cast<float>(quantized) / levels;
    return min + normalized * (max - min);
}

// Pack 3 floats (angles) into 32 bits (10-11-11 bits each)
inline uint32_t packAngles(float yaw, float pitch, float roll) {
    // Normalize to [0, 1]
    float nyaw = (yaw + 180.0f) / 360.0f;
    float npitch = (pitch + 90.0f) / 180.0f;
    float nroll = (roll + 180.0f) / 360.0f;

    // Pack into bits
    uint32_t y = static_cast<uint32_t>(nyaw * 1023.0f) & 0x3FF;      // 10 bits
    uint32_t p = static_cast<uint32_t>(npitch * 2047.0f) & 0x7FF;   // 11 bits
    uint32_t r = static_cast<uint32_t>(nroll * 2047.0f) & 0x7FF;    // 11 bits

    return (y << 22) | (p << 11) | r;
}

inline void unpackAngles(uint32_t packed, float& yaw, float& pitch, float& roll) {
    uint32_t y = (packed >> 22) & 0x3FF;
    uint32_t p = (packed >> 11) & 0x7FF;
    uint32_t r = packed & 0x7FF;

    yaw = (static_cast<float>(y) / 1023.0f) * 360.0f - 180.0f;
    pitch = (static_cast<float>(p) / 2047.0f) * 180.0f - 90.0f;
    roll = (static_cast<float>(r) / 2047.0f) * 360.0f - 180.0f;
}

} // namespace Compression
} // namespace TDS
