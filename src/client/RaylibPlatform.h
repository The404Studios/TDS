// Platform compatibility header for raylib on Windows
// Resolves conflicts between raylib and Windows API
#pragma once

// CRITICAL: Include raylib BEFORE any Windows headers!
#include "raylib.h"

// Now we can safely include Windows headers
#if defined(_WIN32)
    // Undefine conflicting Windows macros that clash with raylib
    #ifdef near
        #undef near
    #endif
    #ifdef far
        #undef far
    #endif
    #ifdef DrawText
        #undef DrawText
    #endif
    #ifdef CloseWindow
        #undef CloseWindow
    #endif
    #ifdef ShowCursor
        #undef ShowCursor
    #endif
    #ifdef LoadImage
        #undef LoadImage
    #endif
    #ifdef PlaySound
        #undef PlaySound
    #endif

    // Map raylib functions that were undefined
    #define rDrawText DrawTextEx
    #define rCloseWindow CloseWindow
    #define rShowCursor ShowCursor
#endif

// raylib color constants (in case they're not defined)
#ifndef DARKRED
    #define DARKRED CLITERAL(Color){ 109, 7, 26, 255 }
#endif
#ifndef DARKGREEN
    #define DARKGREEN CLITERAL(Color){ 0, 117, 44, 255 }
#endif
#ifndef DARKBLUE
    #define DARKBLUE CLITERAL(Color){ 0, 82, 172, 255 }
#endif
#ifndef DARKGRAY
    #define DARKGRAY CLITERAL(Color){ 80, 80, 80, 255 }
#endif

// raylib math helper functions that may be missing
#ifndef TextToFloat
    inline float TextToFloat(const char* text) {
        return static_cast<float>(atof(text));
    }
#endif

#ifndef Vector3Distance
    inline float Vector3Distance(Vector3 v1, Vector3 v2) {
        float dx = v2.x - v1.x;
        float dy = v2.y - v1.y;
        float dz = v2.z - v1.z;
        return sqrtf(dx*dx + dy*dy + dz*dz);
    }
#endif

// Use this header instead of including raylib.h directly in Windows projects
