@echo off
:: Build Environment Verification Script
:: Checks if all dependencies and tools are present before building

echo ========================================
echo   Build Environment Verification
echo ========================================
echo.

set ERROR_COUNT=0

:: Check for MSBuild
echo [1/6] Checking for MSBuild...
where msbuild >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [FAIL] MSBuild not found! Install Visual Studio 2022 or Build Tools.
    set /a ERROR_COUNT+=1
) else (
    echo [PASS] MSBuild found
)

:: Check for raylib
echo [2/6] Checking for raylib...
if exist "dependencies\raylib\include\raylib.h" (
    echo [PASS] raylib found
) else (
    echo [FAIL] raylib not found! Run: download_dependencies.bat
    set /a ERROR_COUNT+=1
)

:: Check for raygui
echo [3/6] Checking for raygui...
if exist "dependencies\raygui\raygui.h" (
    echo [PASS] raygui found
) else (
    echo [FAIL] raygui not found! Run: download_dependencies.bat
    set /a ERROR_COUNT+=1
)

:: Check for raylib library file
echo [4/6] Checking for raylib.lib...
if exist "dependencies\raylib\lib\raylib.lib" (
    echo [PASS] raylib.lib found
) else (
    echo [FAIL] raylib.lib not found! Run: download_dependencies.bat
    set /a ERROR_COUNT+=1
)

:: Check for ENet (optional but recommended)
echo [5/6] Checking for ENet...
if exist "dependencies\enet\include\enet\enet.h" (
    echo [PASS] ENet found
) else (
    echo [WARN] ENet not found (optional for NAT punchthrough)
)

:: Check for project files
echo [6/6] Checking for project files...
if exist "ExtractionShooterRaylib.vcxproj" (
    echo [PASS] ExtractionShooterRaylib.vcxproj found
) else (
    echo [FAIL] ExtractionShooterRaylib.vcxproj missing!
    set /a ERROR_COUNT+=1
)

echo.
echo ========================================
echo   Verification Results
echo ========================================
echo.

if %ERROR_COUNT% EQU 0 (
    echo [SUCCESS] Environment is ready to build!
    echo.
    echo You can now build the project:
    echo   - Run: setup_and_run.bat
    echo   - Or: msbuild ExtractionShooterRaylib.vcxproj /p:Configuration=Release /p:Platform=x64
    echo.
) else (
    echo [FAILED] Found %ERROR_COUNT% error(s)
    echo.
    echo Required actions:
    echo   1. Run: download_dependencies.bat
    echo   2. Install Visual Studio 2022 (if MSBuild missing)
    echo   3. Re-run this verification script
    echo.
)

pause
exit /b %ERROR_COUNT%
