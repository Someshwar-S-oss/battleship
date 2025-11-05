# Quick Setup Guide - Fleet Commander GUI

## Windows Setup (PowerShell)

### Using vcpkg (Easiest Method)

```powershell
# 1. Install vcpkg (if not already installed)
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# 2. Install SFML
.\vcpkg install sfml:x64-windows

# 3. Clone and build the game
cd C:\Users\YourUsername\Repos
git clone <repo-url> battleship
cd battleship

# 4. Create fonts directory and add a font
New-Item -ItemType Directory -Path "assets\fonts" -Force
# Copy a .ttf font file to assets\fonts\arial.ttf

# 5. Build
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release

# 6. Run
.\Release\fleet_commander_gui.exe
```

### Using MSYS2 (Alternative Method)

```bash
# 1. Download and install MSYS2 from https://www.msys2.org/

# 2. Open MSYS2 MinGW 64-bit terminal and install packages
pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-sfml mingw-w64-x86_64-ninja

# 3. Build the game
cd /c/Users/YourUsername/Repos/battleship
mkdir build && cd build
cmake .. -G "Ninja"
ninja

# 4. Run
./fleet_commander_gui.exe
```

## Linux Setup

### Ubuntu/Debian

```bash
# 1. Install dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake git libsfml-dev

# 2. Clone and build
git clone <repo-url> battleship
cd battleship
mkdir build && cd build
cmake ..
make -j$(nproc)

# 3. Run
./fleet_commander_gui
```

### Fedora

```bash
# 1. Install dependencies
sudo dnf install -y gcc-c++ cmake git SFML-devel

# 2. Clone and build
git clone <repo-url> battleship
cd battleship
mkdir build && cd build
cmake ..
make -j$(nproc)

# 3. Run
./fleet_commander_gui
```

### Arch Linux

```bash
# 1. Install dependencies
sudo pacman -S base-devel cmake git sfml

# 2. Clone and build
git clone <repo-url> battleship
cd battleship
mkdir build && cd build
cmake ..
make -j$(nproc)

# 3. Run
./fleet_commander_gui
```

## macOS Setup

```bash
# 1. Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 2. Install dependencies
brew install cmake sfml

# 3. Clone and build
git clone <repo-url> battleship
cd battleship
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)

# 4. Run
./fleet_commander_gui
```

## Font Setup

The game needs a TrueType font file. You have several options:

### Option 1: Use System Fonts (Automatic)
The game will try to use system fonts automatically:
- Windows: `C:\Windows\Fonts\arial.ttf`
- Linux: `/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf`
- macOS: `/System/Library/Fonts/Helvetica.ttc`

### Option 2: Add Custom Font (Recommended for Pixel Art)
1. Download a pixel art font like:
   - Press Start 2P: https://fonts.google.com/specimen/Press+Start+2P
   - VT323: https://fonts.google.com/specimen/VT323
   - Pixelify Sans: https://fonts.google.com/specimen/Pixelify+Sans

2. Create fonts directory:
   ```bash
   mkdir -p assets/fonts
   ```

3. Copy your font:
   ```bash
   cp YourFont.ttf assets/fonts/arial.ttf
   ```

## Verification

After building, you should have:
```
build/
├── fleet_commander          # Terminal version
├── fleet_commander_gui      # GUI version (if SFML found)
└── libgame_logic.a          # Game logic library
```

## Common Issues

### Issue: "SFML not found"
**Solution**: Make sure SFML is installed and CMake can find it.
```bash
# Check SFML installation
# Linux:
dpkg -l | grep sfml
# macOS:
brew list | grep sfml
# Windows vcpkg:
vcpkg list | findstr sfml
```

### Issue: "Failed to load font"
**Solution**: 
1. Create `assets/fonts/` directory in the same folder as the executable
2. Add a `.ttf` font file named `arial.ttf`
3. Or the game will try to use system fonts automatically

### Issue: Build fails with "C++17 required"
**Solution**: Update your compiler
```bash
# Check compiler version
gcc --version  # Need 7.0+
clang --version  # Need 5.0+
```

### Issue: "libsfml-graphics.so.2.5: cannot open shared object file"
**Solution**: Update library path (Linux)
```bash
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
# Or add to ~/.bashrc for permanent fix
```

## Running the Game

### GUI Version
```bash
# From build directory
./fleet_commander_gui

# Or create a launch script (Linux/macOS)
echo '#!/bin/bash
cd "$(dirname "$0")"
./fleet_commander_gui' > launch_gui.sh
chmod +x launch_gui.sh
./launch_gui.sh
```

### Terminal Version
```bash
./fleet_commander
```

## Next Steps

After successful setup:
1. Read `README_GUI.md` for full documentation
2. Try the tutorial in the game
3. Customize colors in `src/GameGUI.h`
4. Add sound effects (see README_GUI.md)
5. Report any issues on GitHub

## Development Build (Debug Mode)

For development with debug symbols:
```bash
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
gdb ./fleet_commander_gui  # For debugging
```

## Performance Testing

Test your build performance:
```bash
# Linux/macOS
time ./fleet_commander_gui

# Check FPS in-game
# Should maintain 60 FPS on modern hardware
```

## Clean Build

If you encounter issues, try a clean build:
```bash
rm -rf build
mkdir build && cd build
cmake ..
make clean
make
```

---

**Ready to battle! 🎮⚓**

For more detailed information, see `README_GUI.md`.
