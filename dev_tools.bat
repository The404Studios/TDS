@echo off
:: Development Tools and Utilities for Extraction Shooter

:MENU
cls
echo ========================================
echo   Extraction Shooter - Dev Tools
echo ========================================
echo.
echo [1] Quick Build (Release)
echo [2] Quick Build (Debug)
echo [3] Clean Build Outputs
echo [4] Run Server Only
echo [5] Run Client Only
echo [6] Run with Debugger
echo [7] View Server Logs
echo [8] Clear Player Data
echo [9] Generate Test Accounts
echo [0] Exit
echo.
set /p CHOICE="Select option: "

if "%CHOICE%"=="1" goto BUILD_RELEASE
if "%CHOICE%"=="2" goto BUILD_DEBUG
if "%CHOICE%"=="3" goto CLEAN
if "%CHOICE%"=="4" goto RUN_SERVER
if "%CHOICE%"=="5" goto RUN_CLIENT
if "%CHOICE%"=="6" goto RUN_DEBUG
if "%CHOICE%"=="7" goto VIEW_LOGS
if "%CHOICE%"=="8" goto CLEAR_DATA
if "%CHOICE%"=="9" goto GEN_ACCOUNTS
if "%CHOICE%"=="0" goto END
goto MENU

:BUILD_RELEASE
echo.
echo Building in Release mode...
msbuild ExtractionShooterServer.vcxproj /p:Configuration=Release /p:Platform=x64 /v:minimal
msbuild ExtractionShooterRaylib.vcxproj /p:Configuration=Release /p:Platform=x64 /v:minimal
echo.
echo Build complete!
pause
goto MENU

:BUILD_DEBUG
echo.
echo Building in Debug mode...
msbuild ExtractionShooterServer.vcxproj /p:Configuration=Debug /p:Platform=x64 /v:minimal
msbuild ExtractionShooterRaylib.vcxproj /p:Configuration=Debug /p:Platform=x64 /v:minimal
echo.
echo Build complete!
pause
goto MENU

:CLEAN
echo.
echo Cleaning build outputs...
rd /s /q x64 2>nul
rd /s /q Debug 2>nul
rd /s /q Release 2>nul
del /q *.log 2>nul
echo Clean complete!
pause
goto MENU

:RUN_SERVER
echo.
echo Starting server...
if exist "x64\Release\ExtractionShooterServer.exe" (
    x64\Release\ExtractionShooterServer.exe
) else if exist "x64\Debug\ExtractionShooterServer.exe" (
    x64\Debug\ExtractionShooterServer.exe
) else (
    echo ERROR: Server executable not found! Build first.
    pause
)
goto MENU

:RUN_CLIENT
echo.
echo Starting client...
if exist "x64\Release\ExtractionShooterRaylib.exe" (
    x64\Release\ExtractionShooterRaylib.exe
) else if exist "x64\Debug\ExtractionShooterRaylib.exe" (
    x64\Debug\ExtractionShooterRaylib.exe
) else (
    echo ERROR: Client executable not found! Build first.
    pause
)
goto MENU

:RUN_DEBUG
echo.
echo Opening Visual Studio for debugging...
devenv ExtractionShooter.sln
goto MENU

:VIEW_LOGS
echo.
echo Server Logs:
echo ========================================
if exist "server.log" (
    type server.log | more
) else (
    echo No log file found.
)
echo.
pause
goto MENU

:CLEAR_DATA
echo.
echo WARNING: This will delete all player accounts and data!
set /p CONFIRM="Are you sure? (Y/N): "
if /i "%CONFIRM%"=="Y" (
    echo.
    echo Clearing player data...
    rd /s /q Data 2>nul
    del /q Server\*.dat 2>nul
    echo.
    echo Player data cleared!
) else (
    echo.
    echo Cancelled.
)
pause
goto MENU

:GEN_ACCOUNTS
echo.
echo Generate Test Accounts
echo ========================================
echo.
echo This will create test accounts for multiplayer testing.
echo.
echo Creating accounts:
echo   - test1 / password123
echo   - test2 / password123
echo   - test3 / password123
echo.
echo NOTE: Run the server once first to create the Data directory
echo Then restart server after this script completes.
echo.
set /p CONTINUE="Continue? (Y/N): "
if /i not "%CONTINUE%"=="Y" goto MENU

if not exist "Data" mkdir Data

:: Create a simple account file (this is a placeholder - actual format depends on AuthManager)
echo Creating test accounts...
echo test1:password123 > Data\test_accounts.txt
echo test2:password123 >> Data\test_accounts.txt
echo test3:password123 >> Data\test_accounts.txt

echo.
echo Test accounts created!
echo NOTE: You may need to implement actual account creation via server API
pause
goto MENU

:END
echo.
echo Goodbye!
timeout /t 1 /nobreak >nul
exit /b 0
