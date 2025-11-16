# Build Instructions

## IMPORTANT: Download Dependencies First!

Before building, you MUST run:

```cmd
download_dependencies.bat
```

This downloads:
- raylib 5.0
- raygui
- ENet (for NAT punchthrough)

## Building

### Recommended: Build Raylib Client Only

The solution contains multiple projects. To build the new raylib-based game:

1. Open `ExtractionShooter.sln` in Visual Studio 2022
2. Right-click on **ExtractionShooterRaylib** → Set as Startup Project
3. Build → Build ExtractionShooterRaylib

To build the server:
1. Right-click on **ExtractionShooterServer** → Build

To build NAT punchthrough server:
1. Right-click on **NatPunchServer** → Build

### Building Everything

If you want to build all projects (including the old OpenGL client):

1. Make sure dependencies are downloaded first
2. Build → Build Solution

Note: The old **ExtractionShooterClient** uses OpenGL directly and may have compilation issues if engine stub files are incomplete. Use **ExtractionShooterRaylib** for the complete raylib-based game.

## Quick Build Commands

```cmd
# Download dependencies
download_dependencies.bat

# Build raylib client only (recommended)
msbuild ExtractionShooterRaylib.vcxproj /p:Configuration=Release /p:Platform=x64

# Build server
msbuild ExtractionShooterServer.vcxproj /p:Configuration=Release /p:Platform=x64

# Build NAT server
msbuild NatPunchServer.vcxproj /p:Configuration=Release /p:Platform=x64
```

## Running

After building:

```cmd
cd x64\Release

# Start server
ExtractionShooterServer.exe

# Start raylib client
ExtractionShooterRaylib.exe

# Optional: Start NAT server
NatPunchServer.exe
```

## Troubleshooting

### "Cannot open include file: 'raylib.h'"
**Solution**: Run `download_dependencies.bat` first!

### Old client compilation errors
**Solution**: Build only **ExtractionShooterRaylib** instead of the whole solution. The old OpenGL client (ExtractionShooterClient) is legacy code.

### Missing DLLs when running
**Solution**: Make sure you're running from the `x64\Release` or `x64\Debug` directory, or copy raylib.dll to the executable directory.

## Project Overview

- **ExtractionShooterRaylib** ← New raylib 3D client (USE THIS!)
- **ExtractionShooterServer** ← Game server
- **NatPunchServer** ← NAT punchthrough server
- **ExtractionShooterClient** ← Legacy OpenGL client (may have issues)
