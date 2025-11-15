cmake_minimum_required(VERSION 3.15)
project(TDS_RaylibShooter VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_CLIENT "Build game client" ON)
option(BUILD_SERVER "Build game server" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Platform detection
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)  # Windows 7+
    set(PLATFORM_LIBS ws2_32)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LIBS pthread m dl)
elseif(APPLE)
    set(PLATFORM_LIBS pthread m)
endif()

# Dependencies directory
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

# ============================================================================
# Raylib
# ============================================================================
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

# We'll use Raylib from system or download it
find_package(raylib 5.0 QUIET)
if(NOT raylib_FOUND)
    message(STATUS "Raylib not found, will download from GitHub...")
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(raylib)
endif()

# ============================================================================
# ENet (UDP networking library)
# ============================================================================
add_subdirectory(${EXTERNAL_DIR}/enet)

# ============================================================================
# SQLite3
# ============================================================================
add_subdirectory(${EXTERNAL_DIR}/sqlite)

# ============================================================================
# Common library (shared between client and server)
# ============================================================================
add_library(tds_common STATIC
    src/common/Protocol.h
    src/common/Protocol.cpp
    src/common/Items.h
    src/common/Items.cpp
    src/common/Math.h
    src/common/Math.cpp
    src/common/Compression.h
    src/common/Compression.cpp
)

target_include_directories(tds_common PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/common
)

target_link_libraries(tds_common PUBLIC
    enet
)

# ============================================================================
# Client executable
# ============================================================================
if(BUILD_CLIENT)
    add_executable(TDS_Client
        src/client/main.cpp
        src/client/Game.cpp
        src/client/Game.h

        # Network
        src/client/Network/NetworkClient.cpp
        src/client/Network/NetworkClient.h
        src/client/Network/STUNClient.cpp
        src/client/Network/STUNClient.h

        # Rendering
        src/client/Rendering/Renderer.cpp
        src/client/Rendering/Renderer.h
        src/client/Rendering/Camera.cpp
        src/client/Rendering/Camera.h
        src/client/Rendering/ModelManager.cpp
        src/client/Rendering/ModelManager.h
        src/client/Rendering/ParticleSystem.cpp
        src/client/Rendering/ParticleSystem.h

        # Gameplay
        src/client/Gameplay/Player.cpp
        src/client/Gameplay/Player.h
        src/client/Gameplay/RemotePlayer.cpp
        src/client/Gameplay/RemotePlayer.h
        src/client/Gameplay/Weapon.cpp
        src/client/Gameplay/Weapon.h
        src/client/Gameplay/Inventory.cpp
        src/client/Gameplay/Inventory.h
        src/client/Gameplay/LootSpawn.cpp
        src/client/Gameplay/LootSpawn.h

        # UI
        src/client/UI/UIManager.cpp
        src/client/UI/UIManager.h
        src/client/UI/MainMenu.cpp
        src/client/UI/MainMenu.h
        src/client/UI/HUD.cpp
        src/client/UI/HUD.h
        src/client/UI/InventoryUI.cpp
        src/client/UI/InventoryUI.h
        src/client/UI/LoginUI.cpp
        src/client/UI/LoginUI.h

        # Audio
        src/client/Audio/AudioManager.cpp
        src/client/Audio/AudioManager.h
    )

    target_include_directories(TDS_Client PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/client
        ${CMAKE_SOURCE_DIR}/external/enet/include
        ${CMAKE_BINARY_DIR}/_deps/raylib-src/examples/shapes  # For raygui.h
    )

    target_link_libraries(TDS_Client PRIVATE
        tds_common
        raylib
        enet
        ${PLATFORM_LIBS}
    )

    # Copy assets to build directory
    add_custom_command(TARGET TDS_Client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:TDS_Client>/assets
    )
endif()

# ============================================================================
# Server executable
# ============================================================================
if(BUILD_SERVER)
    add_executable(TDS_Server
        src/server/main.cpp
        src/server/Server.cpp
        src/server/Server.h

        # Network
        src/server/Network/NetworkServer.cpp
        src/server/Network/NetworkServer.h
        src/server/Network/STUNServer.cpp
        src/server/Network/STUNServer.h
        src/server/Network/PacketHandler.cpp
        src/server/Network/PacketHandler.h

        # Database
        src/server/Database/Database.cpp
        src/server/Database/Database.h

        # Managers
        src/server/Managers/AuthManager.cpp
        src/server/Managers/AuthManager.h
        src/server/Managers/MatchManager.cpp
        src/server/Managers/MatchManager.h
        src/server/Managers/PlayerManager.cpp
        src/server/Managers/PlayerManager.h
        src/server/Managers/LootManager.cpp
        src/server/Managers/LootManager.h
        src/server/Managers/MerchantManager.cpp
        src/server/Managers/MerchantManager.h

        # Gameplay
        src/server/Gameplay/GameWorld.cpp
        src/server/Gameplay/GameWorld.h
        src/server/Gameplay/AIController.cpp
        src/server/Gameplay/AIController.h
        src/server/Gameplay/PhysicsWorld.cpp
        src/server/Gameplay/PhysicsWorld.h
    )

    target_include_directories(TDS_Server PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/server
        ${CMAKE_SOURCE_DIR}/external/enet/include
        ${CMAKE_SOURCE_DIR}/external/sqlite
    )

    target_link_libraries(TDS_Server PRIVATE
        tds_common
        enet
        sqlite3
        ${PLATFORM_LIBS}
    )
endif()

# ============================================================================
# Installation
# ============================================================================
if(BUILD_CLIENT)
    install(TARGETS TDS_Client DESTINATION bin)
    install(DIRECTORY assets DESTINATION share/tds)
endif()

if(BUILD_SERVER)
    install(TARGETS TDS_Server DESTINATION bin)
endif()

# Print configuration
message(STATUS "==============================================")
message(STATUS "TDS Raylib Shooter Configuration")
message(STATUS "==============================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build client: ${BUILD_CLIENT}")
message(STATUS "Build server: ${BUILD_SERVER}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "==============================================")
