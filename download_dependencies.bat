@echo off
echo ========================================
echo   Extraction Shooter - Dependency Setup
echo ========================================
echo.

:: Create dependencies folder
if not exist "dependencies" mkdir dependencies
cd dependencies

:: Download raylib
echo [1/9] Downloading raylib...
if not exist "raylib" (
    powershell -Command "& {Invoke-WebRequest -Uri 'https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_win64_msvc16.zip' -OutFile 'raylib.zip'}"
    echo Extracting raylib...
    powershell -Command "& {Expand-Archive -Path 'raylib.zip' -DestinationPath '.' -Force}"
    ren "raylib-5.0_win64_msvc16" "raylib"
    del raylib.zip
    echo [OK] raylib downloaded and extracted
) else (
    echo [SKIP] raylib already exists
)

:: Download raygui
echo.
echo [2/9] Downloading raygui...
if not exist "raygui" (
    mkdir raygui
    powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/raysan5/raygui/master/src/raygui.h' -OutFile 'raygui/raygui.h'}"
    echo [OK] raygui downloaded
) else (
    echo [SKIP] raygui already exists
)

:: Download ENet for NAT punchthrough
echo.
echo [3/9] Downloading ENet (NAT Punchthrough)...
if not exist "enet" (
    powershell -Command "& {Invoke-WebRequest -Uri 'https://github.com/lsalzman/enet/archive/refs/tags/v1.3.17.zip' -OutFile 'enet.zip'}"
    echo Extracting ENet...
    powershell -Command "& {Expand-Archive -Path 'enet.zip' -DestinationPath '.' -Force}"
    ren "enet-1.3.17" "enet"
    del enet.zip
    echo [OK] ENet downloaded and extracted
) else (
    echo [SKIP] ENet already exists
)

:: Download ImGui
echo.
echo [4/9] Downloading ImGui (Advanced UI)...
if not exist "imgui" (
    powershell -Command "& {Invoke-WebRequest -Uri 'https://github.com/ocornut/imgui/archive/refs/heads/docking.zip' -OutFile 'imgui.zip'}"
    echo Extracting ImGui...
    powershell -Command "& {Expand-Archive -Path 'imgui.zip' -DestinationPath '.' -Force}"
    ren "imgui-docking" "imgui"
    del imgui.zip
    echo [OK] ImGui downloaded and extracted
) else (
    echo [SKIP] ImGui already exists
)

:: Download nlohmann/json
echo.
echo [5/9] Downloading nlohmann/json (JSON library)...
if not exist "json" (
    mkdir json
    mkdir json\nlohmann
    powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp' -OutFile 'json/nlohmann/json.hpp'}"
    echo [OK] nlohmann/json downloaded
) else (
    echo [SKIP] nlohmann/json already exists
)

:: Download spdlog
echo.
echo [6/9] Downloading spdlog (Logging library)...
if not exist "spdlog" (
    powershell -Command "& {Invoke-WebRequest -Uri 'https://github.com/gabime/spdlog/archive/refs/heads/v1.x.zip' -OutFile 'spdlog.zip'}"
    echo Extracting spdlog...
    powershell -Command "& {Expand-Archive -Path 'spdlog.zip' -DestinationPath '.' -Force}"
    ren "spdlog-1.x" "spdlog"
    del spdlog.zip
    echo [OK] spdlog downloaded and extracted
) else (
    echo [SKIP] spdlog already exists
)

:: Download LZ4
echo.
echo [7/9] Downloading LZ4 (Compression library)...
if not exist "lz4" (
    powershell -Command "& {Invoke-WebRequest -Uri 'https://github.com/lz4/lz4/archive/refs/heads/dev.zip' -OutFile 'lz4.zip'}"
    echo Extracting LZ4...
    powershell -Command "& {Expand-Archive -Path 'lz4.zip' -DestinationPath '.' -Force}"
    ren "lz4-dev" "lz4"
    del lz4.zip
    echo [OK] LZ4 downloaded and extracted
) else (
    echo [SKIP] LZ4 already exists
)

:: Download glTF model loader (for animations)
echo.
echo [8/9] Setting up raylib model support...
echo [OK] raylib includes built-in model loading (glTF, OBJ, IQM)

:: Create resource directories
echo.
echo [9/9] Creating resource directories...
cd ..
if not exist "resources" mkdir resources
if not exist "resources\models" mkdir resources\models
if not exist "resources\animations" mkdir resources\animations
if not exist "resources\textures" mkdir resources\textures
if not exist "resources\sounds" mkdir resources\sounds
if not exist "resources\shaders" mkdir resources\shaders
echo [OK] Resource directories created

:: Summary
echo.
echo ========================================
echo   Setup Complete!
echo ========================================
echo.
echo Dependencies installed in: dependencies\
echo   - raylib (3D graphics engine)
echo   - raygui (UI library)
echo   - ENet (NAT punchthrough networking)
echo   - ImGui (Advanced debugging UI)
echo   - nlohmann/json (JSON configuration)
echo   - spdlog (Professional logging)
echo   - LZ4 (Fast compression)
echo.
echo Resource folders created: resources\
echo   - models\     (3D character models)
echo   - animations\ (animation files)
echo   - textures\   (textures and sprites)
echo   - sounds\     (audio files)
echo   - shaders\    (custom shaders)
echo.
echo Next steps:
echo   1. Open ExtractionShooter.sln in Visual Studio 2022
echo   2. Build the solution (Ctrl+Shift+B)
echo   3. Run the server (ExtractionShooterServer.exe)
echo   4. Run the client (ExtractionShooterClient.exe)
echo.
echo For NAT punchthrough to work:
echo   - Run the NAT punchthrough server first
echo   - Configure clients to connect through NAT server
echo.
pause
