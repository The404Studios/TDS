@echo off
echo ========================================
echo   Extraction Shooter - Dependency Setup
echo ========================================
echo.

:: Create dependencies folder
if not exist "dependencies" mkdir dependencies
cd dependencies

:: Download raylib
echo [1/5] Downloading raylib...
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
echo [2/5] Downloading raygui...
if not exist "raygui" (
    mkdir raygui
    powershell -Command "& {Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/raysan5/raygui/master/src/raygui.h' -OutFile 'raygui/raygui.h'}"
    echo [OK] raygui downloaded
) else (
    echo [SKIP] raygui already exists
)

:: Download ENet for NAT punchthrough
echo.
echo [3/5] Downloading ENet (NAT Punchthrough)...
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

:: Download glTF model loader (for animations)
echo.
echo [4/5] Setting up raylib model support...
echo [OK] raylib includes built-in model loading (glTF, OBJ, IQM)

:: Create resource directories
echo.
echo [5/5] Creating resource directories...
cd ..
if not exist "resources" mkdir resources
if not exist "resources\models" mkdir resources\models
if not exist "resources\animations" mkdir resources\animations
if not exist "resources\textures" mkdir resources\textures
if not exist "resources\sounds" mkdir resources\sounds
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
echo.
echo Resource folders created: resources\
echo   - models\    (3D character models)
echo   - animations\ (animation files)
echo   - textures\  (textures and sprites)
echo   - sounds\    (audio files)
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
