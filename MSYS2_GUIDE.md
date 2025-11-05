# MSYS2 Setup Guide for Fleet Commander

## Quick Setup (You've Already Done This!)

### ✅ What You've Accomplished

1. **Installed SFML via MSYS2** ✓
2. **Built the project successfully** ✓
3. **Copied required DLLs** ✓
4. **Configured VS Code IntelliSense** ✓

### Running the Game

Simply double-click or run:
```
run_gui.bat
```

Or from command line:
```powershell
cd build
.\fleet_commander_gui.exe
```

## Building from Scratch

If you need to rebuild:

```bat
build_msys2.bat
```

This script:
- Cleans previous build
- Configures with CMake
- Builds both terminal and GUI versions
- **Automatically copies all required DLLs**

## Required DLLs (Auto-copied by build script)

The following DLLs are automatically copied from `C:\msys64\mingw64\bin\`:

### SFML Libraries
- `libsfml-system-2.dll`
- `libsfml-window-2.dll`
- `libsfml-graphics-2.dll`
- `libsfml-audio-2.dll`
- `libsfml-network-2.dll`

### MinGW Runtime
- `libgcc_s_seh-1.dll`
- `libstdc++-6.dll`
- `libwinpthread-1.dll`

### SFML Dependencies
- `libfreetype-6.dll`
- `libbz2-1.dll`
- `libpng16-16.dll`
- `zlib1.dll`
- `libbrotlidec.dll`
- `libbrotlicommon.dll`
- `libharfbuzz-0.dll`
- `libglib-2.0-0.dll`
- `libgraphite2.dll`
- `libintl-8.dll`
- `libiconv-2.dll`
- `libpcre2-8-0.dll`

## VS Code Configuration

Your `.vscode/c_cpp_properties.json` is configured for MSYS2 with:
- GCC 15.2.0 compiler path
- Correct include paths for SFML and C++ standard library
- IntelliSense mode: `windows-gcc-x64`

## Troubleshooting

### "Cannot find DLL" Error

If you see a DLL error:
1. Run `build_msys2.bat` again (it will copy DLLs)
2. Or manually run:
   ```powershell
   cd build
   copy C:\msys64\mingw64\bin\lib*.dll .
   ```

### IntelliSense Errors

If you see red squiggles in VS Code:
1. Open Command Palette (Ctrl+Shift+P)
2. Type: "C/C++: Edit Configurations (UI)"
3. Verify compiler path: `C:/msys64/mingw64/bin/g++.exe`
4. Restart VS Code if needed

### Build Fails

Make sure MSYS2 bin is in PATH during build:
```bat
set PATH=C:\msys64\mingw64\bin;%PATH%
```

The `build_msys2.bat` script handles this automatically.

## Project Structure

```
battleship/
├── build/                      # Build output
│   ├── fleet_commander.exe     # Terminal version
│   ├── fleet_commander_gui.exe # GUI version
│   └── *.dll                   # All required DLLs
├── src/                        # Source code
├── build_msys2.bat            # Build script
└── run_gui.bat                # Launch script
```

## Distributing Your Game

To share your game with others who don't have MSYS2:

1. Create a folder (e.g., `FleetCommander`)
2. Copy from `build/`:
   - `fleet_commander_gui.exe`
   - All `.dll` files
3. Optionally add `assets/fonts/` if you have custom fonts
4. Zip and share!

The recipient just runs `fleet_commander_gui.exe` - no installation needed!

## Adding Fonts

Create this directory structure:
```
build/
├── fleet_commander_gui.exe
├── *.dll
└── assets/
    └── fonts/
        └── arial.ttf  (or any .ttf font)
```

If no font is found, the game will try to use Windows system fonts.

## Performance

Your build is configured with:
- C++17 standard
- Optimizations enabled in Release mode
- Static linking of MinGW runtime
- 60 FPS target

## Next Steps

1. **Play the game!** Run `run_gui.bat`
2. **Customize**: Edit colors in `src/GameGUI.h`
3. **Add sounds**: See main README_GUI.md
4. **Share**: Zip the build folder and distribute

## Commands Reference

```bat
# Build everything
build_msys2.bat

# Run GUI version
run_gui.bat

# Run terminal version
build\fleet_commander.exe

# Clean build
rmdir /s /q build
```

## Credits

- **Build System**: CMake + MinGW Makefiles
- **Graphics Library**: SFML 2.6.1
- **Compiler**: GCC 15.2.0 (MSYS2)

---

**Everything is working! Enjoy your game! 🎮⚓**
