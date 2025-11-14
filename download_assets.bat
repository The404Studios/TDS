@echo off
echo ====================================================
echo   TDS Asset Downloader
echo ====================================================
echo.

REM Create directories
echo Creating asset directories...
if not exist "assets\models\weapons" mkdir assets\models\weapons
if not exist "assets\models\characters" mkdir assets\models\characters
if not exist "assets\models\items" mkdir assets\models\items
if not exist "assets\models\maps" mkdir assets\models\maps
if not exist "assets\textures" mkdir assets\textures
if not exist "assets\sounds\weapons" mkdir assets\sounds\weapons
if not exist "assets\sounds\footsteps" mkdir assets\sounds\footsteps
if not exist "assets\sounds\ambient" mkdir assets\sounds\ambient
if not exist "assets\music" mkdir assets\music

echo.
echo Downloading free 3D models from Kenney.nl...
echo.

REM Download weapon models from Kenney
powershell -Command "& {Invoke-WebRequest -Uri 'https://kenney.nl/content/3-assets/9/weaponpack.zip' -OutFile 'assets\models\weaponpack.zip' -ErrorAction SilentlyContinue}"

if exist "assets\models\weaponpack.zip" (
    echo Extracting weapon models...
    powershell -Command "& {Expand-Archive -Path 'assets\models\weaponpack.zip' -DestinationPath 'assets\models\weapons' -Force}"
    del "assets\models\weaponpack.zip"
    echo [OK] Weapon models downloaded
) else (
    echo [SKIP] Could not download weapon models
)

echo.
echo Downloading character models...
REM Download low-poly character from Quaternius (free)
powershell -Command "& {Invoke-WebRequest -Uri 'https://quaternius.com/assets/free/Ultimate%20Low%20Poly%20Animated%20People%20Pack.zip' -OutFile 'assets\models\characters.zip' -ErrorAction SilentlyContinue}"

if exist "assets\models\characters.zip" (
    echo Extracting character models...
    powershell -Command "& {Expand-Archive -Path 'assets\models\characters.zip' -DestinationPath 'assets\models\characters' -Force}"
    del "assets\models\characters.zip"
    echo [OK] Character models downloaded
) else (
    echo [SKIP] Could not download character models
)

echo.
echo Creating placeholder 3D models...

REM Create placeholder cube model
(
echo # Placeholder Cube
echo v -0.5 -0.5  0.5
echo v  0.5 -0.5  0.5
echo v -0.5  0.5  0.5
echo v  0.5  0.5  0.5
echo v -0.5  0.5 -0.5
echo v  0.5  0.5 -0.5
echo v -0.5 -0.5 -0.5
echo v  0.5 -0.5 -0.5
echo f 1 2 4 3
echo f 3 4 6 5
echo f 5 6 8 7
echo f 7 8 2 1
echo f 2 8 6 4
echo f 7 1 3 5
) > assets\models\cube.obj

echo [OK] Created placeholder cube

REM Create basic floor plane
(
echo # Floor Plane
echo v -10 0 -10
echo v  10 0 -10
echo v  10 0  10
echo v -10 0  10
echo f 1 2 3 4
) > assets\models\floor.obj

echo [OK] Created floor plane

echo.
echo Downloading sound effects...
echo Note: For sounds, please visit:
echo   - Freesound.org ^(https://freesound.org/^)
echo   - OpenGameArt.org ^(https://opengameart.org/^)
echo.
echo Search for:
echo   - "gun shot" for weapon sounds
echo   - "footstep" for movement sounds
echo   - "ambiance" for background audio
echo.
echo Save sounds to:
echo   assets\sounds\weapons\     ^(gunshot.ogg, reload.ogg^)
echo   assets\sounds\footsteps\   ^(concrete1.ogg, grass1.ogg^)
echo   assets\sounds\ambient\     ^(wind.ogg, rain.ogg^)
echo   assets\music\              ^(menu.ogg, combat.ogg^)
echo.

REM Create placeholder sound file info
(
echo TDS Sound Effects Required
echo ============================
echo.
echo Weapons:
echo   - ak47_fire.ogg
echo   - m4a1_fire.ogg
echo   - pistol_fire.ogg
echo   - reload.ogg
echo   - empty_click.ogg
echo.
echo Footsteps:
echo   - concrete_1.ogg to concrete_4.ogg
echo   - grass_1.ogg to grass_4.ogg
echo   - metal_1.ogg to metal_4.ogg
echo.
echo Ambient:
echo   - wind.ogg
echo   - rain.ogg
echo   - birds.ogg
echo.
echo Music:
echo   - menu_theme.ogg
echo   - combat_music.ogg
echo.
echo Download from Freesound.org ^(CC0 license^)
) > assets\SOUND_LIST.txt

echo [OK] Created sound list

echo.
echo Creating example textures...

REM Note: Can't create actual images in batch, but we can provide instructions
(
echo TDS Texture Pack
echo =================
echo.
echo Download PBR textures from:
echo   - PolyHaven.com ^(https://polyhaven.com/textures^)
echo   - AmbientCG.com ^(https://ambientcg.com/^)
echo.
echo Required texture sets ^(2048x2048^):
echo   - Concrete: concrete_albedo.png, concrete_normal.png, concrete_roughness.png
echo   - Metal: metal_albedo.png, metal_normal.png, metal_metallic.png
echo   - Wood: wood_albedo.png, wood_normal.png, wood_roughness.png
echo   - Fabric: fabric_albedo.png, fabric_normal.png
echo.
echo UI Textures:
echo   - crosshair.png ^(32x32^)
echo   - item_icons\ ^(64x64 for each item^)
echo.
) > assets\textures\TEXTURE_LIST.txt

echo [OK] Created texture list

echo.
echo ====================================================
echo   Asset Download Complete!
echo ====================================================
echo.
echo Summary:
echo   [*] Asset directories created
echo   [*] Placeholder 3D models created
echo   [*] Asset lists generated
echo.
echo Next steps:
echo   1. Visit Freesound.org for sound effects ^(CC0 license^)
echo   2. Visit PolyHaven.com for PBR textures
echo   3. Optional: Download models from Kenney.nl or Quaternius.com
echo   4. Build project: cmake -B build ^&^& cmake --build build
echo.
echo Asset locations:
echo   Models:   %CD%\assets\models\
echo   Sounds:   %CD%\assets\sounds\
echo   Textures: %CD%\assets\textures\
echo   Music:    %CD%\assets\music\
echo.
pause
