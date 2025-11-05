# Fleet Commander - Pixel Art Battleship

A native desktop Battleship game with both terminal and GUI versions, featuring pixel art style graphics and smooth gameplay.

![Fleet Commander](assets/screenshot.png)

## Features

### Game Features
- **Two-player gameplay**: Human vs AI
- **10x10 grid board**: Classic Battleship layout (A-J rows, 1-10 columns)
- **5 ship types**: Aircraft Carrier (5), Battleship (4), Cruiser (3), Submarine (3), Destroyer (2)
- **Smart AI opponent**: Random but challenging computer player
- **Hit/miss tracking**: Visual feedback for all attacks
- **Ship sinking detection**: Know when you've destroyed an enemy ship

### GUI Version Features
- **Pixel art aesthetic**: Retro-inspired visual style
- **Interactive ship placement**: Click and drag interface with rotation
- **Particle effects**: Visual feedback for hits, misses, and sinks
- **Real-time animations**: Smooth transitions and effects
- **Message log**: Track the battle history
- **Dual board display**: See your fleet and enemy waters simultaneously
- **Hover highlighting**: Preview your target before attacking

### Terminal Version Features
- **Classic ASCII interface**: Works in any terminal
- **Color-coded display**: Easy to read game state
- **Keyboard input**: Simple text-based controls

## Architecture

The project is structured into three main components:

### 1. Game Logic (`GameLogic.h/cpp`)
- **Ship class**: Manages individual ships, hit tracking, and sinking
- **Board class**: Handles 10x10 grid, ship placement, and attack resolution
- **Pure C++ logic**: No UI dependencies, fully reusable

### 2. GUI Layer (`GameGUI.h/cpp`)
- **SFML-based rendering**: Hardware-accelerated 2D graphics
- **BoardView**: Renders game boards with pixel art style
- **Button**: Interactive UI elements
- **MessageBox**: Scrolling message display
- **Particle system**: Visual effects for attacks
- **Game state machine**: Menu, Placement, Battle, GameOver states

### 3. Application Entry Points
- **main.cpp**: Terminal version (original implementation)
- **main_gui.cpp**: GUI version entry point

## Requirements

### For Terminal Version
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16 or higher

### For GUI Version
- All terminal version requirements
- SFML 2.5 or higher
- Graphics drivers with OpenGL support

## Installation

### Windows

#### Option 1: Using vcpkg (Recommended)
```powershell
# Install vcpkg if not already installed
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# Install SFML
.\vcpkg install sfml:x64-windows

# Build the project
cd path\to\battleship
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

#### Option 2: Manual SFML Installation
1. Download SFML 2.5+ from https://www.sfml-dev.org/download.php
2. Extract to a location (e.g., `C:\SFML`)
3. Build with CMake:
```powershell
mkdir build
cd build
cmake .. -DSFML_DIR=C:\SFML\lib\cmake\SFML
cmake --build . --config Release
```

#### Option 3: Using MSYS2
```bash
# Install MSYS2 from https://www.msys2.org/
# Open MSYS2 MinGW 64-bit terminal
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-sfml

# Build the project
cd /path/to/battleship
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
```

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install build-essential cmake libsfml-dev

# Build the project
cd battleship
mkdir build && cd build
cmake ..
make

# Run
./fleet_commander      # Terminal version
./fleet_commander_gui  # GUI version (if SFML found)
```

### Linux (Fedora/RHEL)

```bash
# Install dependencies
sudo dnf install gcc-c++ cmake SFML-devel

# Build the project
cd battleship
mkdir build && cd build
cmake ..
make
```

### macOS

```bash
# Install dependencies using Homebrew
brew install cmake sfml

# Build the project
cd battleship
mkdir build && cd build
cmake ..
make

# Run
./fleet_commander      # Terminal version
./fleet_commander_gui  # GUI version
```

## Building from Source

### Quick Start
```bash
# Clone the repository
git clone <your-repo-url>
cd battleship

# Build
mkdir build && cd build
cmake ..
cmake --build .

# Run terminal version
./fleet_commander

# Run GUI version (if SFML is installed)
./fleet_commander_gui
```

### CMake Options

The build system automatically detects SFML:
- **SFML found**: Builds both terminal and GUI versions
- **SFML not found**: Builds only terminal version with a warning

### Build Configurations

```bash
# Debug build (with debug symbols)
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Release build (optimized)
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## Usage

### GUI Version

1. **Launch the game**:
   ```bash
   ./fleet_commander_gui
   ```

2. **Main Menu**:
   - Click "Start New Game" to begin

3. **Ship Placement**:
   - Move mouse over the board to preview ship placement
   - Press `R` to rotate ships (horizontal/vertical)
   - Click to place the current ship
   - Repeat for all 5 ships

4. **Battle**:
   - Hover over enemy waters (right board)
   - Click a cell to attack
   - Watch for hit/miss effects
   - Computer takes its turn automatically
   - Continue until all ships of one side are sunk

5. **Game Over**:
   - View the result (Victory/Defeat)
   - Click "Play Again" to restart

### Terminal Version

1. **Launch the game**:
   ```bash
   ./fleet_commander
   ```

2. **Follow the on-screen prompts**:
   - Enter coordinates like "A5", "D10"
   - Choose orientation: H (horizontal) or V (vertical)
   - Attack enemy positions during your turn

## Controls

### GUI Version
| Control | Action |
|---------|--------|
| Mouse Move | Preview placement / Hover target |
| Left Click | Place ship / Attack cell |
| R Key | Rotate ship during placement |
| ESC | Close game (from any screen) |

### Terminal Version
| Input | Action |
|-------|--------|
| A1-J10 | Coordinate input |
| H | Horizontal orientation |
| V | Vertical orientation |
| Enter | Confirm input |

## Project Structure

```
battleship/
├── src/
│   ├── main.cpp          # Terminal version entry point
│   ├── main_gui.cpp      # GUI version entry point
│   ├── GameLogic.h       # Core game logic (header)
│   ├── GameLogic.cpp     # Core game logic (implementation)
│   ├── GameGUI.h         # GUI framework (header)
│   └── GameGUI.cpp       # GUI framework (implementation)
├── assets/
│   └── fonts/
│       └── arial.ttf     # Font file (add your own)
├── CMakeLists.txt        # Build configuration
├── README.md             # This file
└── placement.txt         # AI ship placement cache (auto-generated)
```

## Customization

### Changing Colors (GameGUI.h)
```cpp
namespace Colors
{
    const sf::Color Ocean(41, 128, 185);      // Change ocean color
    const sf::Color Hit(231, 76, 60);         // Change hit marker color
    const sf::Color Miss(236, 240, 241);      // Change miss marker color
    // ... modify other colors
}
```

### Adjusting Board Size
The game uses a constant 10x10 grid. To change it, modify:
```cpp
// In GameLogic.h
class Board
{
public:
    static constexpr int SIZE = 10;  // Change this value
    // ...
};
```

### Adding Custom Fonts
1. Download a pixel art font (e.g., [Press Start 2P](https://fonts.google.com/specimen/Press+Start+2P))
2. Place it in `assets/fonts/`
3. Update `GameGUI.cpp`:
```cpp
void GameGUI::initFont()
{
    font.loadFromFile("assets/fonts/YourFont.ttf");
}
```

## Troubleshooting

### SFML Not Found
**Problem**: CMake can't find SFML
**Solution**:
- Ensure SFML is installed correctly
- Provide SFML path explicitly:
  ```bash
  cmake .. -DSFML_DIR=/path/to/sfml/lib/cmake/SFML
  ```

### Font Loading Failed
**Problem**: "Failed to load font!" message
**Solution**:
- Create `assets/fonts/` directory
- Copy a TrueType font (`.ttf`) into it
- Or install system fonts (the code tries to use system Arial/DejaVu)

### Black Screen on Launch
**Problem**: Window opens but shows nothing
**Solution**:
- Update graphics drivers
- Check OpenGL support: `glxinfo | grep OpenGL` (Linux)
- Try different SFML version

### Build Errors
**Problem**: Compilation fails
**Solution**:
- Ensure C++17 support: `gcc --version` or `clang --version`
- Update CMake: `cmake --version` (need 3.16+)
- Check compiler flags in CMakeLists.txt

### Performance Issues
**Problem**: Low FPS or stuttering
**Solution**:
- Build in Release mode: `cmake -DCMAKE_BUILD_TYPE=Release ..`
- Reduce particle count in `createHitEffect()` functions
- Check graphics driver support

## Development

### Code Style
- **Standard**: C++17
- **Naming**: CamelCase for classes, camelCase for functions
- **Indentation**: 4 spaces
- **Headers**: Include guards with `#pragma once`

### Adding New Features

#### Example: Adding Sound Effects
1. Install SFML Audio: `vcpkg install sfml:x64-windows` (includes audio)
2. Update `GameGUI.h`:
```cpp
#include <SFML/Audio.hpp>

class GameGUI
{
private:
    sf::SoundBuffer hitBuffer;
    sf::Sound hitSound;
};
```
3. Load and play sounds in `GameGUI.cpp`:
```cpp
void GameGUI::initGameObjects()
{
    hitBuffer.loadFromFile("assets/sounds/hit.wav");
    hitSound.setBuffer(hitBuffer);
}

void GameGUI::playerAttack(const Coordinate &target)
{
    // ... existing code ...
    if (result == Board::AttackResult::Hit)
    {
        hitSound.play();
        // ... existing code ...
    }
}
```

### Testing

#### Unit Tests (Future Enhancement)
To add unit tests, integrate Google Test:
```cmake
# In CMakeLists.txt
find_package(GTest REQUIRED)
enable_testing()

add_executable(game_tests tests/game_logic_tests.cpp)
target_link_libraries(game_tests game_logic GTest::GTest GTest::Main)
add_test(NAME GameTests COMMAND game_tests)
```

## Performance Notes

### GUI Version Performance
- **Target FPS**: 60 FPS
- **Particle limit**: ~100 active particles
- **Memory usage**: ~50-100 MB
- **CPU usage**: Low (<5% on modern hardware)

### Optimization Tips
- Particles auto-cleanup after lifetime expires
- Board rendering is optimized (no redraw of static elements)
- Event-driven updates reduce unnecessary computations

## Credits

- **Developer**: [Your Name]
- **Framework**: SFML (Simple and Fast Multimedia Library)
- **Inspiration**: Classic Battleship board game
- **Art Style**: Pixel art aesthetic

## License

This project is available under the MIT License. See LICENSE file for details.

## Contributing

Contributions are welcome! Here's how:

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Commit changes: `git commit -m 'Add amazing feature'`
4. Push to branch: `git push origin feature/amazing-feature`
5. Open a Pull Request

### Feature Ideas
- [ ] Networked multiplayer
- [ ] Custom ship sizes and counts
- [ ] Multiple difficulty levels
- [ ] Sound effects and music
- [ ] Save/load game state
- [ ] High score tracking
- [ ] Different board sizes
- [ ] Power-ups and special abilities
- [ ] Animated ship sprites
- [ ] Custom themes/skins

## FAQ

**Q: Can I play against another human?**
A: Currently only vs AI. Multiplayer is a planned feature.

**Q: Does the game save progress?**
A: The AI placement is cached in `placement.txt`. Full save/load is a future feature.

**Q: Can I change the grid size?**
A: Yes, modify `Board::SIZE` in `GameLogic.h`, but you'll need to adjust UI layout.

**Q: Why use SFML instead of Qt?**
A: SFML is perfect for pixel art games: simpler, more performant, and better for 2D graphics.

**Q: Is the terminal version still maintained?**
A: Yes! Both versions share the same core logic and are equally supported.

## Version History

### v1.0.0 (Current)
- ✅ Full GUI implementation with SFML
- ✅ Interactive ship placement
- ✅ Particle effects for combat
- ✅ AI opponent
- ✅ Terminal version preserved
- ✅ Cross-platform support

### Future Versions
- v1.1.0: Sound effects and music
- v1.2.0: Network multiplayer
- v2.0.0: Advanced AI with difficulty levels

## Support

For issues, questions, or suggestions:
- **Issues**: Open a GitHub issue
- **Email**: [your-email@example.com]
- **Discussions**: Use GitHub Discussions

---

**Happy sailing, Admiral! ⚓🎮**
