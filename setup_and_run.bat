@echo off
echo ========================================
echo   Extraction Shooter - Complete Setup
echo   One-Click Build and Run
echo ========================================
echo.

:: Check if running from correct directory
if not exist "ExtractionShooter.sln" (
    echo ERROR: Please run this script from the TDS directory
    echo Current directory: %CD%
    pause
    exit /b 1
)

:: Step 1: Download dependencies
echo [1/4] Checking dependencies...
if not exist "dependencies\raylib" (
    echo Dependencies not found. Running download script...
    call download_dependencies.bat
    if errorlevel 1 (
        echo ERROR: Failed to download dependencies
        pause
        exit /b 1
    )
) else (
    echo Dependencies already downloaded.
)
echo.

:: Step 2: Build server
echo [2/4] Building server...
msbuild ExtractionShooterServer.vcxproj /p:Configuration=Release /p:Platform=x64 /v:minimal /nologo
if errorlevel 1 (
    echo.
    echo ERROR: Server build failed!
    echo Try opening Visual Studio and building manually.
    pause
    exit /b 1
)
echo Server built successfully!
echo.

:: Step 3: Build raylib client
echo [3/4] Building raylib client...
msbuild ExtractionShooterRaylib.vcxproj /p:Configuration=Release /p:Platform=x64 /v:minimal /nologo
if errorlevel 1 (
    echo.
    echo ERROR: Client build failed!
    echo Try opening Visual Studio and building manually.
    pause
    exit /b 1
)
echo Client built successfully!
echo.

:: Step 4: Build NAT server (optional)
echo [4/4] Building NAT punchthrough server...
msbuild NatPunchServer.vcxproj /p:Configuration=Release /p:Platform=x64 /v:minimal /nologo
if errorlevel 1 (
    echo WARNING: NAT server build failed (optional, continuing...)
) else (
    echo NAT server built successfully!
)
echo.

:: Display success message
echo ========================================
echo   BUILD COMPLETE!
echo ========================================
echo.
echo Your game is ready to run!
echo.
echo Executables created:
echo   - x64\Release\ExtractionShooterServer.exe
echo   - x64\Release\ExtractionShooterRaylib.exe
echo   - x64\Release\NatPunchServer.exe (optional)
echo.
echo.

:: Ask if user wants to run the game
set /p RUN_NOW="Do you want to run the game now? (Y/N): "
if /i "%RUN_NOW%"=="Y" (
    echo.
    echo Starting game server...
    start "Extraction Shooter Server" x64\Release\ExtractionShooterServer.exe

    :: Wait a moment for server to start
    timeout /t 2 /nobreak >nul

    echo Starting game client...
    start "Extraction Shooter Client" x64\Release\ExtractionShooterRaylib.exe

    echo.
    echo Game launched! Check the new windows.
    echo.
    echo To run additional clients for multiplayer testing,
    echo run: x64\Release\ExtractionShooterRaylib.exe
    echo.
) else (
    echo.
    echo To run the game manually:
    echo   1. Start server: x64\Release\ExtractionShooterServer.exe
    echo   2. Start client: x64\Release\ExtractionShooterRaylib.exe
    echo.
)

pause
