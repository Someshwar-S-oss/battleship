# Developer Guide - Fleet Commander

## Architecture Overview

### Design Principles

1. **Separation of Concerns**: Game logic is independent of UI
2. **Reusability**: Core logic shared between terminal and GUI versions
3. **Modularity**: Each component has a single responsibility
4. **Extensibility**: Easy to add new features without breaking existing code

### Component Diagram

```
┌─────────────────────────────────────────────┐
│         Application Layer                    │
├──────────────────┬──────────────────────────┤
│   main.cpp       │   main_gui.cpp            │
│  (Terminal UI)   │   (GUI Entry)             │
└────────┬─────────┴──────────┬────────────────┘
         │                    │
         │         ┌──────────▼───────────┐
         │         │     GameGUI          │
         │         │  (SFML Interface)    │
         │         └──────────┬───────────┘
         │                    │
         └────────┬───────────┘
                  │
         ┌────────▼────────┐
         │   GameLogic     │
         │  (Core Engine)  │
         │                 │
         │  - Board        │
         │  - Ship         │
         │  - Coordinates  │
         └─────────────────┘
```

## Core Components

### 1. GameLogic.h/cpp

#### Ship Class
Represents a single ship in the game.

**Key Methods**:
- `setPositions(coords)`: Place ship at coordinates
- `registerHit(coord)`: Record a hit
- `isSunk()`: Check if all positions are hit
- `occupies(coord)`: Check if ship is at coordinate

**Derived Classes**:
- `AircraftCarrier` (size 5)
- `Battleship` (size 4)
- `Cruiser` (size 3)
- `Submarine` (size 3)
- `Destroyer` (size 2)

#### Board Class
Manages the 10x10 game grid.

**Key Methods**:
- `placeShip(ship, start, horizontal)`: Place ship on board
- `attack(coord, shipName)`: Execute attack at coordinate
- `allShipsSunk()`: Check win condition
- `getCellSymbol(coord, showShips)`: Get display character for cell

**Internal Structure**:
```cpp
struct Cell {
    Ship* ship = nullptr;    // Pointer to ship at this cell
    bool attacked = false;   // Has this cell been attacked?
};
```

### 2. GameGUI.h/cpp

#### GameGUI Class (Main Controller)
Manages the entire GUI application lifecycle.

**State Machine**:
```
Menu → PlacingShips → PlayerTurn ↔ ComputerTurn → GameOver
  ↑                                                    │
  └────────────────────────────────────────────────────┘
```

**Key Methods**:
- `run()`: Main game loop
- `processEvents()`: Handle user input
- `update(deltaTime)`: Update game state
- `render()`: Draw everything to screen

#### BoardView Class
Renders a game board visually.

**Responsibilities**:
- Draw 10x10 grid with labels
- Display ships, hits, misses
- Handle mouse interactions
- Show placement preview
- Highlight cells on hover

**Layout**:
```
     1  2  3  4  5  6  7  8  9 10
  A [ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]
  B [ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]
  ...
  J [ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]
```

#### Button Class
Simple interactive button UI element.

**Features**:
- Hover effect
- Click detection
- Custom text and size

#### MessageBox Class
Displays scrolling game messages.

**Features**:
- Holds last 10 messages
- Auto-scroll
- Semi-transparent background

#### Particle System
Visual effects for combat actions.

**Particle Structure**:
```cpp
struct Particle {
    sf::CircleShape shape;     // Visual representation
    sf::Vector2f velocity;     // Movement speed/direction
    float lifetime;            // Time remaining
    float maxLifetime;         // For fade calculation
};
```

## Code Flow

### Application Startup

```
main_gui.cpp: main()
    ↓
GameGUI: Constructor
    ↓
initWindow() → Create SFML window
    ↓
initFont() → Load font
    ↓
initGameObjects() → Create boards, ships, UI elements
    ↓
changeState(Menu) → Show main menu
    ↓
run() → Start game loop
```

### Game Loop

```cpp
while (window.isOpen()) {
    // 1. Input
    processEvents();  // Handle clicks, key presses
    
    // 2. Update
    update(deltaTime);  // Update animations, AI turn
    
    // 3. Render
    render();  // Draw everything
}
```

### Ship Placement Flow

```
1. User enters PlacingShips state
2. Loop for each ship:
   a. Show preview on mouse move
   b. Check validity of placement
   c. User presses R to rotate
   d. User clicks to place
   e. Validate and place ship
3. After all ships placed → Setup computer fleet
4. Transition to PlayerTurn
```

### Combat Flow

```
Player Turn:
1. Hover over enemy board → Highlight cell
2. Click cell → playerAttack()
3. Board::attack() → Calculate result
4. Create particle effect
5. Add message
6. Check game over
7. → Computer Turn

Computer Turn:
1. Wait actionDelay seconds
2. executeComputerAttack()
3. Pick random untried coordinate
4. Board::attack() → Calculate result
5. Create particle effect
6. Add message
7. Check game over
8. → Player Turn
```

## Key Algorithms

### Ship Placement Validation

```cpp
bool isValidPlacement(Coordinate start, int size, bool horizontal) {
    for (int i = 0; i < size; i++) {
        int row = start.row + (horizontal ? 0 : i);
        int col = start.col + (horizontal ? i : 0);
        
        // Check bounds
        if (row < 0 || row >= SIZE || col < 0 || col >= SIZE)
            return false;
        
        // Check overlap
        if (board.isOccupied({row, col}))
            return false;
    }
    return true;
}
```

### Computer Placement Algorithm

```cpp
void generateComputerPlacements() {
    while (true) {  // Retry until success
        board.clear();
        
        bool allPlaced = true;
        for (each ship) {
            bool placed = false;
            
            for (500 attempts) {  // Try 500 times
                random orientation (H/V);
                random start position;
                
                if (board.placeShip(ship, start, horizontal)) {
                    placed = true;
                    break;
                }
            }
            
            if (!placed) {
                allPlaced = false;
                break;  // Start over
            }
        }
        
        if (allPlaced)
            return;  // Success!
    }
}
```

### Attack Resolution

```cpp
AttackResult attack(Coordinate target) {
    Cell& cell = grid[target.row][target.col];
    
    if (cell.attacked)
        return AlreadyTried;
    
    cell.attacked = true;
    
    if (cell.ship == nullptr)
        return Miss;
    
    cell.ship->registerHit(target);
    
    if (cell.ship->isSunk())
        return Sunk;
    
    return Hit;
}
```

## Extending the Game

### Adding a New Ship Type

1. **Define the class** (GameLogic.h):
```cpp
class Corvette : public Ship {
public:
    Corvette() : Ship("Corvette", 1) {}
    std::string getType() const override { return "Corvette"; }
};
```

2. **Add to fleet creation** (GameGUI.cpp):
```cpp
void GameGUI::createFleet(std::vector<std::unique_ptr<Ship>>& fleet) {
    // ... existing ships ...
    fleet.emplace_back(std::make_unique<Corvette>());
}
```

### Adding Sound Effects

1. **Update GameGUI.h**:
```cpp
#include <SFML/Audio.hpp>

class GameGUI {
private:
    sf::SoundBuffer hitBuffer, missBuffer, sinkBuffer;
    sf::Sound hitSound, missSound, sinkSound;
};
```

2. **Initialize in constructor** (GameGUI.cpp):
```cpp
void GameGUI::initGameObjects() {
    // ... existing code ...
    
    hitBuffer.loadFromFile("assets/sounds/hit.wav");
    missBuffer.loadFromFile("assets/sounds/miss.wav");
    sinkBuffer.loadFromFile("assets/sounds/sink.wav");
    
    hitSound.setBuffer(hitBuffer);
    missSound.setBuffer(missBuffer);
    sinkSound.setBuffer(sinkBuffer);
}
```

3. **Play in combat** (GameGUI.cpp):
```cpp
void GameGUI::playerAttack(const Coordinate& target) {
    // ... existing code ...
    
    switch (result) {
    case Board::AttackResult::Hit:
        hitSound.play();
        // ... existing code ...
        break;
    case Board::AttackResult::Miss:
        missSound.play();
        // ... existing code ...
        break;
    case Board::AttackResult::Sunk:
        sinkSound.play();
        // ... existing code ...
        break;
    }
}
```

### Adding Multiplayer (Network)

1. **Choose networking library**: SFML Network, Boost.Asio, or ENet

2. **Add network layer**:
```cpp
class NetworkManager {
public:
    void host(int port);
    void connect(string ip, int port);
    void sendAttack(Coordinate target);
    Coordinate receiveAttack();
};
```

3. **Modify game states**:
```cpp
enum class GameState {
    Menu,
    HostOrJoin,      // New
    WaitingForPlayer, // New
    PlacingShips,
    PlayerTurn,
    OpponentTurn,    // Renamed from ComputerTurn
    GameOver
};
```

4. **Replace AI logic**:
```cpp
void GameGUI::updateOpponentTurn() {
    if (network.isConnected()) {
        Coordinate attack = network.receiveAttack();
        executeNetworkAttack(attack);
    } else {
        executeComputerAttack();  // Fallback to AI
    }
}
```

### Custom Board Sizes

1. **Make size configurable** (GameLogic.h):
```cpp
class Board {
public:
    Board(int size = 10);  // Default 10x10
    int getSize() const { return size; }
    
private:
    int size;
    std::vector<std::vector<Cell>> grid;  // Dynamic 2D vector
};
```

2. **Update UI layout** (GameGUI.cpp):
```cpp
BoardView::BoardView(const sf::Vector2f& position, int boardSize) 
    : position(position), boardSize(boardSize) {
    // Adjust CELL_SIZE based on boardSize
    cellSize = 400.0f / boardSize;  // Scale to fit
}
```

### Advanced AI with Difficulty Levels

```cpp
enum class Difficulty {
    Easy,    // Random shooting
    Medium,  // Hunt mode after hit
    Hard     // Probability map + pattern recognition
};

class AIPlayer {
public:
    void setDifficulty(Difficulty level);
    Coordinate chooseTarget(const Board& board);
    
private:
    Difficulty difficulty;
    std::vector<Coordinate> huntQueue;  // For Medium/Hard
    
    Coordinate randomShot(const Board& board);
    Coordinate huntShot(const Board& board);
    Coordinate smartShot(const Board& board);
};
```

## Testing

### Unit Tests Example

```cpp
#include <gtest/gtest.h>
#include "GameLogic.h"

TEST(ShipTest, PlacementAndHits) {
    Destroyer ship;
    std::vector<Coordinate> positions = {{0,0}, {0,1}};
    ship.setPositions(positions);
    
    EXPECT_TRUE(ship.isPlaced());
    EXPECT_EQ(ship.getSize(), 2);
    
    EXPECT_FALSE(ship.isSunk());
    ship.registerHit({0,0});
    EXPECT_FALSE(ship.isSunk());
    ship.registerHit({0,1});
    EXPECT_TRUE(ship.isSunk());
}

TEST(BoardTest, AttackLogic) {
    Board board;
    Destroyer ship;
    
    board.placeShip(ship, {5,5}, true);
    
    std::string shipName;
    auto result = board.attack({5,5}, shipName);
    EXPECT_EQ(result, Board::AttackResult::Hit);
    
    result = board.attack({5,5}, shipName);
    EXPECT_EQ(result, Board::AttackResult::AlreadyTried);
    
    result = board.attack({0,0}, shipName);
    EXPECT_EQ(result, Board::AttackResult::Miss);
}
```

### Integration Tests

```cpp
TEST(GameIntegration, FullGameFlow) {
    Board playerBoard, computerBoard;
    
    // Setup fleets
    // ... place all ships ...
    
    // Simulate game
    while (!playerBoard.allShipsSunk() && !computerBoard.allShipsSunk()) {
        // Player turn
        Coordinate target = chooseRandomTarget(computerBoard);
        std::string shipName;
        computerBoard.attack(target, shipName);
        
        // Computer turn
        target = chooseRandomTarget(playerBoard);
        playerBoard.attack(target, shipName);
    }
    
    // Verify winner
    EXPECT_TRUE(playerBoard.allShipsSunk() || computerBoard.allShipsSunk());
}
```

## Performance Optimization

### Profiling

```bash
# Linux
gprof ./fleet_commander_gui gmon.out > analysis.txt

# Valgrind
valgrind --tool=callgrind ./fleet_commander_gui

# macOS
instruments -t "Time Profiler" ./fleet_commander_gui
```

### Common Bottlenecks

1. **Excessive particle creation**
   - Solution: Pool particles, limit max count
   
2. **Frequent board redrawing**
   - Solution: Cache rendered boards, only redraw on change
   
3. **String allocations in hot loops**
   - Solution: Use string_view, reserve capacity

### Optimization Example

Before:
```cpp
void BoardView::draw(sf::RenderWindow& window, const Board& board) {
    for (int row = 0; row < Board::SIZE; row++) {
        for (int col = 0; col < Board::SIZE; col++) {
            std::string symbol = getCellSymbol(board, row, col);
            drawCell(window, row, col, symbol);
        }
    }
}
```

After:
```cpp
void BoardView::draw(sf::RenderWindow& window, const Board& board) {
    if (!dirty) {
        window.draw(cachedSprite);  // Draw cached version
        return;
    }
    
    cachedTexture.clear();
    for (int row = 0; row < Board::SIZE; row++) {
        for (int col = 0; col < Board::SIZE; col++) {
            char symbol = getCellSymbol(board, row, col);
            drawCell(cachedTexture, row, col, symbol);
        }
    }
    cachedTexture.display();
    dirty = false;
}
```

## Debugging Tips

### SFML Window Debugging

```cpp
// Enable debug output
std::cout << "Window size: " << window.getSize().x << "x" << window.getSize().y << std::endl;

// Check if textures loaded
if (!texture.loadFromFile("file.png")) {
    std::cerr << "Failed to load texture!" << std::endl;
}

// Verify mouse coordinates
auto mousePos = sf::Mouse::getPosition(window);
std::cout << "Mouse: " << mousePos.x << ", " << mousePos.y << std::endl;
```

### Game Logic Debugging

```cpp
// Add logging to Board::attack
AttackResult Board::attack(const Coordinate& target, std::string& shipName) {
    std::cout << "Attack at " << target.first << "," << target.second << std::endl;
    
    // ... existing code ...
    
    std::cout << "Result: " << static_cast<int>(result) << std::endl;
    return result;
}
```

### Memory Leak Detection

```bash
# Linux/macOS
valgrind --leak-check=full ./fleet_commander_gui

# Check for:
# - "definitely lost" blocks
# - Unreleased SFML resources
# - Dangling Ship* pointers
```

## Best Practices

### Code Organization
- ✅ Keep game logic separate from UI
- ✅ Use const correctness
- ✅ Prefer smart pointers over raw pointers
- ✅ Document complex algorithms
- ✅ Use meaningful variable names

### SFML Best Practices
- ✅ Load resources once (fonts, textures)
- ✅ Use sf::RenderTexture for caching
- ✅ Batch draw calls when possible
- ✅ Use deltaTime for frame-independent movement
- ✅ Handle window resize events

### Git Workflow
```bash
# Feature branch
git checkout -b feature/new-ship-type

# Regular commits
git commit -m "Add: Corvette ship class"
git commit -m "Test: Add Corvette unit tests"
git commit -m "UI: Display Corvette in fleet"

# Merge to main
git checkout main
git merge feature/new-ship-type
```

## Resources

### SFML Documentation
- Official Docs: https://www.sfml-dev.org/documentation/
- Tutorials: https://www.sfml-dev.org/tutorials/
- Forum: https://en.sfml-dev.org/forums/

### C++ Resources
- cppreference: https://en.cppreference.com/
- C++ Core Guidelines: https://isocpp.github.io/CppCoreGuidelines/

### Game Development
- Game Programming Patterns: https://gameprogrammingpatterns.com/
- Red Blob Games: https://www.redblobgames.com/

---

**Happy coding! 💻🎮**
