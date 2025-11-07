#pragma once

// Platform-specific includes
// IMPORTANT: winsock2.h MUST be included before windows.h to avoid redefinition errors

#ifdef _WIN32
    // Include winsock2 FIRST
    #ifndef _WINSOCKAPI_
        #include <winsock2.h>
        #include <ws2tcpip.h>
    #endif

    // Now include windows
    #ifndef _WINDOWS_
        #define WIN32_LEAN_AND_MEAN
        #define NOMINMAX  // Prevent min/max macros
        #include <windows.h>
    #endif

    // OpenGL
    #include <gl/GL.h>
    #include <gl/GLU.h>

    // Link libraries
    #pragma comment(lib, "ws2_32.lib")
    #pragma comment(lib, "opengl32.lib")
    #pragma comment(lib, "glu32.lib")
#endif

// Platform detection macros
#ifdef _WIN32
    #define PLATFORM_WINDOWS 1
#elif defined(__linux__)
    #define PLATFORM_LINUX 1
#elif defined(__APPLE__)
    #define PLATFORM_MAC 1
#endif

// Common types
#include <cstdint>
#include <cstddef>
