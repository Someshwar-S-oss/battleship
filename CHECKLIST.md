# Getting Started Checklist

Use this checklist to get your GUI Battleship game up and running!

## 🎯 Step 1: Verify Your Setup

### ✅ Check Prerequisites

- [ ] C++17 compatible compiler installed
  - Windows: Visual Studio 2017+, MinGW-w64, or Clang
  - Linux: GCC 7+, Clang 5+
  - macOS: Xcode Command Line Tools (Clang)

- [ ] CMake 3.16 or higher installed
  ```bash
  cmake --version
  ```

- [ ] Git installed (for cloning, optional)
  ```bash
  git --version
  ```

### ✅ Verify Project Files

Make sure you have all these files:

- [ ] `src/main.cpp` - Original terminal version
- [ ] `src/main_gui.cpp` - GUI entry point
- [ ] `src/GameLogic.h` - Game logic header
- [ ] `src/GameLogic.cpp` - Game logic implementation
- [ ] `src/GameGUI.h` - GUI framework header
- [ ] `src/GameGUI.cpp` - GUI framework implementation
- [ ] `CMakeLists.txt` - Build configuration
- [ ] `README_GUI.md` - Main documentation
- [ ] `SETUP.md` - Quick setup guide

## 🎯 Step 2: Install SFML

### Windows (Choose One Method)

#### Option A: vcpkg (Recommended)
- [ ] Clone vcpkg
  ```powershell
  git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
  cd C:\vcpkg
  .\bootstrap-vcpkg.bat
  .\vcpkg integrate install
  ```

- [ ] Install SFML
  ```powershell
  .\vcpkg install sfml:x64-windows
  ```

#### Option B: MSYS2
- [ ] Download MSYS2 from https://www.msys2.org/
- [ ] Install packages
  ```bash
  pacman -Syu
  pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-sfml
  ```

### Linux

- [ ] Install SFML
  ```bash
  # Ubuntu/Debian
  sudo apt-get update
  sudo apt-get install libsfml-dev
  
  # Fedora
  sudo dnf install SFML-devel
  
  # Arch
  sudo pacman -S sfml
  ```

### macOS

- [ ] Install Homebrew (if not installed)
  ```bash
  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
  ```

- [ ] Install SFML
  ```bash
  brew install sfml
  ```

## 🎯 Step 3: Prepare Assets

### Fonts

- [ ] Create fonts directory
  ```bash
  mkdir -p assets/fonts
  ```

- [ ] Choose one option:

  **Option A: Use System Fonts** (Easiest)
  - [ ] Nothing to do! Game will auto-detect system fonts

  **Option B: Add Custom Font** (Best for pixel art)
  - [ ] Download a pixel font (e.g., Press Start 2P from Google Fonts)
  - [ ] Copy to `assets/fonts/arial.ttf`
  - [ ] Or copy any .ttf file you like

## 🎯 Step 4: Build the Project

### Windows

#### Using vcpkg
- [ ] Navigate to project
  ```powershell
  cd C:\Users\YourUsername\Repos\battleship
  ```

- [ ] Run build script
  ```powershell
  .\build.bat
  ```

  OR manually:
  ```powershell
  mkdir build
  cd build
  cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
  cmake --build . --config Release
  ```

#### Using MSYS2
- [ ] Open MSYS2 MinGW 64-bit terminal
- [ ] Navigate to project
  ```bash
  cd /c/Users/YourUsername/Repos/battleship
  ```

- [ ] Build
  ```bash
  mkdir build && cd build
  cmake .. -G "Ninja"
  ninja
  ```

### Linux/macOS

- [ ] Navigate to project
  ```bash
  cd ~/battleship
  ```

- [ ] Run build script
  ```bash
  chmod +x build.sh
  ./build.sh
  ```

  OR manually:
  ```bash
  mkdir build && cd build
  cmake ..
  make -j$(nproc)  # Linux
  make -j$(sysctl -n hw.ncpu)  # macOS
  ```

## 🎯 Step 5: Verify Build

- [ ] Check for executables in `build/` directory:

  **Windows:**
  - [ ] `build/Release/fleet_commander.exe` (Terminal)
  - [ ] `build/Release/fleet_commander_gui.exe` (GUI)

  **Linux/macOS:**
  - [ ] `build/fleet_commander` (Terminal)
  - [ ] `build/fleet_commander_gui` (GUI)

- [ ] If GUI version not built, check:
  - [ ] SFML installed correctly?
  - [ ] CMake found SFML? (check CMake output)
  - [ ] Read error messages in build output

## 🎯 Step 6: Run the Game

### First Test: Terminal Version

- [ ] Run terminal version (should always work)
  ```bash
  # Windows
  .\build\Release\fleet_commander.exe
  
  # Linux/macOS
  ./build/fleet_commander
  ```

- [ ] Verify it works before trying GUI

### Main Event: GUI Version

- [ ] Run GUI version
  ```bash
  # Windows
  .\build\Release\fleet_commander_gui.exe
  
  # Linux/macOS
  ./build/fleet_commander_gui
  ```

- [ ] Check for:
  - [ ] Window opens (1200x700)
  - [ ] Title: "Fleet Commander - Pixel Art Edition"
  - [ ] Menu screen with "Start New Game" button

## 🎯 Step 7: Test Gameplay

### Menu
- [ ] Click "Start New Game" button
- [ ] Transitions to ship placement screen

### Ship Placement
- [ ] See your board on left
- [ ] Instructions at top
- [ ] Move mouse over board → see preview ship
- [ ] Press R → ship rotates
- [ ] Click → ship places (if valid)
- [ ] Repeat for all 5 ships
- [ ] After all placed → transitions to battle

### Battle
- [ ] See two boards (yours left, enemy right)
- [ ] Turn indicator shows "YOUR TURN"
- [ ] Hover over enemy board → yellow highlight
- [ ] Click enemy cell → attack
- [ ] See particle effects (red/white)
- [ ] Message appears in log
- [ ] Computer takes turn automatically
- [ ] Repeat until game over

### Game Over
- [ ] "VICTORY!" or "DEFEAT" message
- [ ] "Play Again" button
- [ ] Click → returns to menu

## 🎯 Step 8: Troubleshooting (If Needed)

### Issue: Build Fails

- [ ] Check compiler version supports C++17
- [ ] Verify CMake 3.16+
- [ ] Read error messages carefully
- [ ] Check SETUP.md for platform-specific issues

### Issue: "SFML not found"

- [ ] Verify SFML installation
  ```bash
  # Linux
  dpkg -l | grep sfml
  
  # macOS
  brew list | grep sfml
  
  # Windows vcpkg
  vcpkg list | findstr sfml
  ```

- [ ] Provide SFML path explicitly
  ```bash
  cmake .. -DSFML_DIR=/path/to/sfml/lib/cmake/SFML
  ```

### Issue: "Failed to load font"

- [ ] Create assets directory
  ```bash
  mkdir -p assets/fonts
  ```

- [ ] Add a .ttf font file
- [ ] Or check if system fonts accessible

### Issue: Black Screen

- [ ] Update graphics drivers
- [ ] Check OpenGL support
  ```bash
  # Linux
  glxinfo | grep "OpenGL version"
  ```

- [ ] Try different SFML version

### Issue: Low FPS / Stuttering

- [ ] Build in Release mode (not Debug)
- [ ] Check CPU/GPU usage
- [ ] Reduce particle count in code

## 🎯 Step 9: Explore & Customize

- [ ] Read `README_GUI.md` for full features
- [ ] Check `DEVELOPER.md` for customization
- [ ] Try changing colors in `src/GameGUI.h`
- [ ] Add your own fonts
- [ ] Experiment with particle effects

## 🎯 Step 10: Next Steps

### Learn
- [ ] Study the code architecture (ARCHITECTURE.md)
- [ ] Understand game state machine
- [ ] Learn SFML basics from official tutorials

### Enhance
- [ ] Add sound effects (see DEVELOPER.md)
- [ ] Create custom ship sprites
- [ ] Implement difficulty levels
- [ ] Add save/load functionality

### Share
- [ ] Create a GitHub repository
- [ ] Share screenshots
- [ ] Get feedback from friends
- [ ] Contribute improvements

## 📚 Reference Documentation

Quick links to help:

- [ ] **SETUP.md** - Installation instructions
- [ ] **README_GUI.md** - Complete user guide
- [ ] **DEVELOPER.md** - Development guide
- [ ] **ARCHITECTURE.md** - System architecture
- [ ] **PROJECT_SUMMARY.md** - Overview

## ✅ Success Criteria

You're done when:

- ✅ Game builds without errors
- ✅ GUI window opens properly
- ✅ Can place all ships with mouse
- ✅ Combat works with visual effects
- ✅ Game detects winner correctly
- ✅ Can play multiple rounds

## 🆘 Getting Help

If stuck:

1. **Read the docs** - Check README_GUI.md and SETUP.md first
2. **Check CMake output** - Often shows what's wrong
3. **Google the error** - Many SFML issues are documented
4. **SFML forums** - https://en.sfml-dev.org/forums/
5. **Stack Overflow** - Tag with [sfml] and [c++]

## 🎉 You're Ready!

Once you've completed this checklist, you have a fully functional GUI Battleship game!

**Enjoy your game! ⚓🎮**

---

*Estimated time: 15-30 minutes for first-time setup*
*Subsequent builds: 2-5 minutes*
