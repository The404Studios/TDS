cmake_minimum_required(VERSION 3.15)
project(ExtractionShooter VERSION 1.0.0 LANGUAGES CXX)

# C++17 standard required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Platform detection
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    add_definitions(-DPLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX TRUE)
    add_definitions(-DPLATFORM_LINUX)
endif()

# Find packages
find_package(Threads REQUIRED)

# raylib
if(PLATFORM_LINUX)
    # Try system raylib first
    find_package(raylib QUIET)
    if(NOT raylib_FOUND)
        # Use local build
        set(raylib_DIR "${CMAKE_SOURCE_DIR}/dependencies/raylib")
        if(EXISTS "${raylib_DIR}")
            include_directories(${raylib_DIR}/include)
            link_directories(${raylib_DIR}/lib)
        else()
            message(WARNING "raylib not found. Run ./download_dependencies.sh first!")
        endif()
    endif()
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/client
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/natpunch
    ${CMAKE_SOURCE_DIR}/dependencies/raygui
)

if(PLATFORM_LINUX)
    include_directories(
        ${CMAKE_SOURCE_DIR}/dependencies/raylib/include
    )
endif()

# Common source files
set(COMMON_SOURCES
    src/common/NetworkProtocol.h
    src/common/DataStructures.h
    src/common/ItemDatabase.h
    src/common/CorpseSystem.h
    src/common/Utils.h
)

# Server executable
set(SERVER_SOURCES
    src/server/ServerMain.cpp
    src/server/NetworkServer.h
    src/server/managers/AuthManager.h
    src/server/managers/LobbyManager.h
    src/server/managers/FriendManager.h
    src/server/managers/MatchManager.h
    src/server/managers/MatchManager.cpp
    src/server/managers/MerchantManager.h
    src/server/managers/PersistenceManager.h
    ${COMMON_SOURCES}
)

add_executable(ExtractionShooterServer ${SERVER_SOURCES})

if(PLATFORM_LINUX)
    target_link_libraries(ExtractionShooterServer PRIVATE
        Threads::Threads
        pthread
    )
elseif(PLATFORM_WINDOWS)
    target_link_libraries(ExtractionShooterServer PRIVATE
        ws2_32
        Threads::Threads
    )
endif()

# Raylib Client executable
set(CLIENT_SOURCES
    src/client/RaylibMain.cpp
    src/client/RaylibGameClient.h
    src/client/PlaceholderModels.h
    src/client/animation/AnimationController.h
    src/client/audio/SoundSystem.h
    src/client/effects/ParticleSystem.h
    src/client/NetworkClient.h
    ${COMMON_SOURCES}
)

add_executable(ExtractionShooterRaylib ${CLIENT_SOURCES})

if(PLATFORM_LINUX)
    target_link_libraries(ExtractionShooterRaylib PRIVATE
        raylib
        m
        pthread
        dl
        rt
        X11
    )
elseif(PLATFORM_WINDOWS)
    target_link_libraries(ExtractionShooterRaylib PRIVATE
        raylib
        ws2_32
        winmm
        gdi32
        opengl32
    )
endif()

# NAT Punchthrough Server executable
set(NAT_SERVER_SOURCES
    src/natpunch/NatPunchServerMain.cpp
    src/natpunch/NatPunchServer.h
)

add_executable(NatPunchServer ${NAT_SERVER_SOURCES})

if(PLATFORM_LINUX)
    target_link_libraries(NatPunchServer PRIVATE
        Threads::Threads
        pthread
    )
elseif(PLATFORM_WINDOWS)
    target_link_libraries(NatPunchServer PRIVATE
        ws2_32
    )
endif()

# Installation
install(TARGETS ExtractionShooterServer ExtractionShooterRaylib NatPunchServer
    RUNTIME DESTINATION bin
)

# Copy server config
install(FILES server_config.ini
    DESTINATION bin
)

# Print build info
message(STATUS "")
message(STATUS "=== Extraction Shooter Build Configuration ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
if(PLATFORM_LINUX)
    message(STATUS "raylib: System or dependencies/raylib")
endif()
message(STATUS "==============================================")
message(STATUS "")
