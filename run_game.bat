@echo off
echo ========================================
echo   Extraction Shooter - Quick Launch
echo ========================================
echo.

:: Check if executables exist
if not exist "x64\Release\ExtractionShooterServer.exe" (
    echo ERROR: Server executable not found!
    echo Please build the project first using setup_and_run.bat
    echo Or run: msbuild ExtractionShooter.sln /p:Configuration=Release /p:Platform=x64
    pause
    exit /b 1
)

if not exist "x64\Release\ExtractionShooterRaylib.exe" (
    echo ERROR: Client executable not found!
    echo Please build the project first using setup_and_run.bat
    pause
    exit /b 1
)

:: Ask how many clients to run
echo How many game clients do you want to run?
echo (Recommended: 1 for single-player testing, 2-4 for multiplayer)
echo.
set /p NUM_CLIENTS="Number of clients (1-8): "

:: Validate input
if "%NUM_CLIENTS%"=="" set NUM_CLIENTS=1
if %NUM_CLIENTS% LSS 1 set NUM_CLIENTS=1
if %NUM_CLIENTS% GTR 8 set NUM_CLIENTS=8

echo.
echo Starting game with %NUM_CLIENTS% client(s)...
echo.

:: Start server
echo [1] Starting server...
start "Extraction Shooter Server" x64\Release\ExtractionShooterServer.exe

:: Wait for server to initialize
timeout /t 3 /nobreak >nul

:: Start clients
set /a COUNTER=1
:LOOP
if %COUNTER% GTR %NUM_CLIENTS% goto END_LOOP

echo [%COUNTER%] Starting client %COUNTER%...
start "Extraction Shooter Client %COUNTER%" x64\Release\ExtractionShooterRaylib.exe

:: Small delay between client launches
timeout /t 1 /nobreak >nul

set /a COUNTER+=1
goto LOOP

:END_LOOP

echo.
echo ========================================
echo   GAME LAUNCHED!
echo ========================================
echo.
echo Server and %NUM_CLIENTS% client(s) are now running.
echo.
echo Game windows:
echo   - Server console (backend)
echo   - Client window(s) (playable)
echo.
echo To test multiplayer:
echo   1. Each client window is a separate player
echo   2. Create account and login on each
echo   3. Enter raids and you'll see each other!
echo.
echo Press any key to exit this launcher...
pause >nul
