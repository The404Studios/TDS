@echo off
echo ===============================================
echo Neural Civilization - Library Setup Script
echo ===============================================
echo.
echo This script will set up the OpenGL libraries needed for the project.
echo.

set PROJECT_DIR=%~dp0
set LIB_DIR=%PROJECT_DIR%lib
set GL_DIR=%PROJECT_DIR%GL

echo Creating directories...
if not exist "%LIB_DIR%" mkdir "%LIB_DIR%"
if not exist "%GL_DIR%" mkdir "%GL_DIR%"

echo.
echo ===============================================
echo Option 1: Using Windows Built-in OpenGL Only
echo ===============================================
echo.
echo The project can run with just Windows' built-in OpenGL.
echo Use main_windows.cpp for this option.
echo No additional downloads needed!
echo.

echo ===============================================
echo Option 2: Download GLUT and GLEW (recommended)
echo ===============================================
echo.
echo For the full-featured version with better text rendering
echo and easier window management, we need FreeGLUT and GLEW.
echo.

choice /C YN /M "Do you want to download FreeGLUT and GLEW libraries?"
if errorlevel 2 goto UseBuiltin
if errorlevel 1 goto DownloadLibs

:DownloadLibs
echo.
echo Downloading FreeGLUT and GLEW...
echo.

echo Creating temporary download script...
echo $webclient = New-Object System.Net.WebClient > download_libs.ps1
echo Write-Host "Downloading FreeGLUT..." >> download_libs.ps1
echo $webclient.DownloadFile("https://www.transmissionzero.co.uk/files/software/development/GLUT/freeglut-MSVC.zip", "freeglut.zip") >> download_libs.ps1
echo Write-Host "Downloading GLEW..." >> download_libs.ps1
echo $webclient.DownloadFile("https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip/download", "glew.zip") >> download_libs.ps1
echo Write-Host "Downloads complete!" >> download_libs.ps1

powershell -ExecutionPolicy Bypass -File download_libs.ps1

echo.
echo Extracting libraries...
powershell -Command "Expand-Archive -Path 'freeglut.zip' -DestinationPath 'temp_freeglut' -Force"
powershell -Command "Expand-Archive -Path 'glew.zip' -DestinationPath 'temp_glew' -Force"

echo.
echo Copying library files...

REM Copy FreeGLUT files
xcopy /Y /S "temp_freeglut\freeglut\bin\x64\*.dll" "%LIB_DIR%\"
xcopy /Y /S "temp_freeglut\freeglut\lib\x64\*.lib" "%LIB_DIR%\"
xcopy /Y /S "temp_freeglut\freeglut\include\GL\*.h" "%GL_DIR%\"

REM Copy GLEW files
xcopy /Y /S "temp_glew\glew-2.1.0\bin\Release\x64\*.dll" "%LIB_DIR%\"
xcopy /Y /S "temp_glew\glew-2.1.0\lib\Release\x64\*.lib" "%LIB_DIR%\"
xcopy /Y /S "temp_glew\glew-2.1.0\include\GL\*.h" "%GL_DIR%\"

echo.
echo Cleaning up temporary files...
rmdir /S /Q temp_freeglut
rmdir /S /Q temp_glew
del freeglut.zip
del glew.zip
del download_libs.ps1

echo.
echo ===============================================
echo Libraries installed successfully!
echo ===============================================
echo.
echo You can now:
echo 1. Open NeuralCivilization.vcxproj in Visual Studio
echo 2. Build with main.cpp (requires the downloaded libraries)
echo 3. Run the simulation!
echo.
goto End

:UseBuiltin
echo.
echo ===============================================
echo Using Windows Built-in OpenGL
echo ===============================================
echo.
echo To use the Windows-only version:
echo 1. Open NeuralCivilization.vcxproj in Visual Studio
echo 2. Replace main.cpp with main_windows.cpp
echo 3. Build and run!
echo.
echo No additional libraries needed - it uses only
echo opengl32.lib and glu32.lib which come with Windows.
echo.

:End
echo Press any key to exit...
pause > nul
