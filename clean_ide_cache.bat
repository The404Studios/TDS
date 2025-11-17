@echo off
REM TDS - IDE Cache Cleanup Script (Windows)
REM Run this script to completely clean all IDE caches and force a fresh reindex

echo ==========================================
echo TDS - IDE Cache Cleanup
echo ==========================================
echo.

cd /d "%~dp0"

echo 1. Removing Visual Studio cache...
if exist .vs\ rd /s /q .vs
if exist out\ rd /s /q out
if exist x64\ rd /s /q x64
if exist x86\ rd /s /q x86
if exist Debug\ rd /s /q Debug
if exist Release\ rd /s /q Release
del /q *.suo 2>nul
del /q *.user 2>nul
del /q *.userosscache 2>nul
del /q *.sln.docstates 2>nul
del /q *.VC.db 2>nul
del /q *.VC.opendb 2>nul
echo    OK Visual Studio cache removed
echo.

echo 2. Removing VS Code cache...
if exist .vscode\ rd /s /q .vscode
echo    OK VS Code cache removed
echo.

echo 3. Removing JetBrains (CLion, etc.) cache...
if exist .idea\ rd /s /q .idea
for /d %%i in (cmake-build-*) do rd /s /q "%%i"
echo    OK JetBrains cache removed
echo.

echo 4. Removing CMake cache...
if exist build\CMakeFiles\ rd /s /q build\CMakeFiles
del /q build\CMakeCache.txt 2>nul
del /q build\*.cmake 2>nul
echo    OK CMake cache removed
echo.

echo 5. Regenerating compile_commands.json...
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. >nul 2>&1
cd ..
echo    OK compile_commands.json regenerated
echo.

echo ==========================================
echo OK IDE cache cleanup complete!
echo ==========================================
echo.
echo Next steps:
echo 1. Close your IDE completely if it's still open
echo 2. Reopen the PROJECT FOLDER (not a .sln file)
echo 3. Let it detect CMakeLists.txt
echo 4. Wait for indexing to complete
echo.
echo If using Visual Studio:
echo   - Use 'Open ^> Folder' NOT 'Open ^> Project/Solution'
echo   - Point to: %CD%
echo.
echo All errors for SceneManager, UIButton, WeatherSystem
echo should disappear after reindexing.
echo.
pause
