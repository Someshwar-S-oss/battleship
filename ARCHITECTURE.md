# Visual Architecture Guide

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        Application Layer                         │
├──────────────────────────────┬──────────────────────────────────┤
│     Terminal Version         │         GUI Version               │
│                              │                                   │
│  ┌────────────────────┐     │   ┌────────────────────────┐     │
│  │     main.cpp       │     │   │    main_gui.cpp        │     │
│  │                    │     │   │                        │     │
│  │  - Console I/O     │     │   │  - Creates GameGUI     │     │
│  │  - Text rendering  │     │   │  - Exception handling  │     │
│  │  - Keyboard input  │     │   └────────────┬───────────┘     │
│  └────────┬───────────┘     │                │                  │
│           │                 │   ┌────────────▼───────────┐     │
│           │                 │   │       GameGUI          │     │
│           │                 │   │                        │     │
│           │                 │   │  State Machine:        │     │
│           │                 │   │  - Menu                │     │
│           │                 │   │  - PlacingShips        │     │
│           │                 │   │  - PlayerTurn          │     │
│           │                 │   │  - ComputerTurn        │     │
│           │                 │   │  - GameOver            │     │
│           │                 │   │                        │     │
│           │                 │   │  Components:           │     │
│           │                 │   │  - BoardView (x2)      │     │
│           │                 │   │  - MessageBox          │     │
│           │                 │   │  - Buttons             │     │
│           │                 │   │  - Particle System     │     │
│           │                 │   └────────────┬───────────┘     │
│           │                 │                │                  │
└───────────┼─────────────────┴────────────────┼──────────────────┘
            │                                  │
            │    ┌──────────────────────────┐  │
            └────►   GameLogic Library      ◄──┘
                 │                          │
                 │  ┌────────────────────┐  │
                 │  │   Board Class      │  │
                 │  │                    │  │
                 │  │  - 10x10 Grid      │  │
                 │  │  - Ship placement  │  │
                 │  │  - Attack logic    │  │
                 │  │  - Win detection   │  │
                 │  └────────────────────┘  │
                 │                          │
                 │  ┌────────────────────┐  │
                 │  │   Ship Classes     │  │
                 │  │                    │  │
                 │  │  - AircraftCarrier │  │
                 │  │  - Battleship      │  │
                 │  │  - Cruiser         │  │
                 │  │  - Submarine       │  │
                 │  │  - Destroyer       │  │
                 │  │                    │  │
                 │  │  Methods:          │  │
                 │  │  - registerHit()   │  │
                 │  │  - isSunk()        │  │
                 │  │  - occupies()      │  │
                 │  └────────────────────┘  │
                 └──────────────────────────┘
```

## GUI Component Hierarchy

```
GameGUI (Main Controller)
│
├─── Window Management (SFML)
│    └─── sf::RenderWindow (1200x700)
│
├─── Game State Machine
│    ├─── Menu State
│    │    └─── Button (Start New Game)
│    │
│    ├─── PlacingShips State
│    │    ├─── BoardView (Player)
│    │    ├─── Ship Preview (Ghost ship)
│    │    └─── MessageBox (Instructions)
│    │
│    ├─── PlayerTurn State
│    │    ├─── BoardView (Player) - shows ships
│    │    ├─── BoardView (Enemy) - fogged
│    │    ├─── MessageBox (Combat log)
│    │    └─── Particle Effects
│    │
│    ├─── ComputerTurn State
│    │    └─── (Same as PlayerTurn, but AI acts)
│    │
│    └─── GameOver State
│         ├─── Victory/Defeat Text
│         └─── Button (Play Again)
│
├─── UI Components
│    │
│    ├─── BoardView (Reusable)
│    │    ├─── Grid Drawing (10x10)
│    │    ├─── Cell Rendering
│    │    ├─── Mouse Interaction
│    │    ├─── Hover Highlighting
│    │    └─── Ship Display
│    │
│    ├─── Button
│    │    ├─── Shape (Rectangle)
│    │    ├─── Text (Label)
│    │    ├─── Hover Detection
│    │    └─── Click Handling
│    │
│    └─── MessageBox
│         ├─── Background (Semi-transparent)
│         ├─── Message List (Last 10)
│         └─── Auto-scroll
│
└─── Visual Effects
     └─── Particle System
          ├─── Hit Particles (Red, explosive)
          ├─── Miss Particles (White, splash)
          └─── Sink Particles (Red+Yellow, large)
```

## Data Flow - Ship Placement

```
User Action                  GameGUI                    GameLogic
─────────────────────────────────────────────────────────────────

Mouse Move
    ↓
    ├─→ getCellFromMouse()
    │       ↓
    │   Update preview coords
    │       ↓
    │   Check validity ─────→ Board::isOccupied()
    │       ↓                      ↓
    │   Set previewValid ←─────── Return bool
    │       ↓
    └── Draw preview
            (green if valid,
             red if invalid)

Press 'R'
    ↓
    └─→ Toggle horizontal flag
            ↓
        Update preview

Mouse Click
    ↓
    └─→ placeCurrentShip() ───→ Board::placeShip()
            │                       │
            │                       ├─ Validate bounds
            │                       ├─ Check collisions
            │                       └─ Place if valid
            │                           ↓
            ↓                       Return success
        Update message box
            ↓
        Next ship or finish
```

## Data Flow - Combat

```
Player Turn                  GameGUI                    GameLogic
─────────────────────────────────────────────────────────────────

Mouse Hover
    ↓
    └─→ Highlight cell
        (yellow outline)

Mouse Click on Enemy Board
    ↓
    └─→ playerAttack(coord) ───→ Board::attack()
            │                        │
            │                        ├─ Check if attacked
            │                        ├─ Mark as attacked
            │                        ├─ Check for ship
            │                        │   │
            │                        │   ├─ Miss → Return Miss
            │                        │   │
            │                        │   └─ Hit
            │                        │       ├─ Ship::registerHit()
            │                        │       ├─ Check if sunk
            │                        │       └─ Return Hit/Sunk
            │                        │
            ↓                       Return AttackResult
        Switch (result)
            │
            ├─ Miss
            │   ├─ Create miss particles
            │   └─ "Miss at A5"
            │
            ├─ Hit
            │   ├─ Create hit particles
            │   └─ "Hit at A5!"
            │
            └─ Sunk
                ├─ Create sink particles
                └─ "Sunk the Battleship!"
                    ↓
            Check game over ────→ Board::allShipsSunk()
                │                      ↓
                ├─ If true → GameOver state
                └─ Else → ComputerTurn state


Computer Turn (After delay)
    ↓
    └─→ executeComputerAttack()
            │
            ├─ Pick random coordinate
            │   (from shuffled list)
            │
            └─→ Board::attack() ───→ (Same as above)
                    │
                    ↓
                Update player board
                    ↓
                Check game over
                    ↓
                → PlayerTurn state
```

## Memory Layout

```
Stack (Main Thread)
│
├─── main()
│    └─── GameGUI game;  (Stack-allocated controller)
│
│
Heap (Dynamic Allocations)
│
├─── std::unique_ptr<Board> playerBoard
│    └─── Board object
│         ├─── std::array<std::array<Cell, 10>, 10> grid
│         └─── std::vector<Ship*> ships (pointers only)
│
├─── std::unique_ptr<Board> computerBoard
│    └─── (Same structure)
│
├─── std::vector<std::unique_ptr<Ship>> playerFleet
│    ├─── AircraftCarrier object
│    ├─── Battleship object
│    ├─── Cruiser object
│    ├─── Submarine object
│    └─── Destroyer object
│
├─── std::vector<std::unique_ptr<Ship>> computerFleet
│    └─── (Same structure)
│
├─── std::unique_ptr<BoardView> playerBoardView
│    └─── BoardView object
│         └─── SFML shapes (vertex arrays)
│
├─── std::unique_ptr<BoardView> computerBoardView
│    └─── (Same structure)
│
├─── std::unique_ptr<MessageBox> messageBox
│    └─── MessageBox object
│         ├─── sf::RectangleShape
│         ├─── sf::Text
│         └─── std::vector<std::string> messages
│
├─── std::vector<std::unique_ptr<Button>> buttons
│    └─── Button objects (created per state)
│
└─── std::vector<Particle> particles
     └─── Particle objects (50-100 active)
          └─── sf::CircleShape each


SFML Resources (Managed by SFML)
│
├─── sf::RenderWindow
├─── sf::Font (loaded once)
└─── GPU resources (textures, vertex buffers)
```

## Coordinate Systems

```
Board Coordinates (Logical)
─────────────────────────────
     0   1   2   3   ... 9
  ┌─────────────────────────┐
0 │ A1  A2  A3  A4  ... A10 │
1 │ B1  B2  B3  B4  ... B10 │
2 │ C1  C2  C3  C4  ... C10 │
...│ ...                 ... │
9 │ J1  J2  J3  J4  ... J10 │
  └─────────────────────────┘

User Input: "A5"
    ↓
Parse: {row: 0, col: 4}
    ↓
Use in Board


Screen Coordinates (Pixels)
───────────────────────────
Window: 1200x700

Player Board Position: (50, 150)
    ↓
Cell (0, 0) screen pos: (55, 155)
    [50 + PADDING + 0*CELL_SIZE, 150 + PADDING + 0*CELL_SIZE]
    ↓
Cell (3, 5) screen pos: (255, 275)
    [50 + 5 + 5*40, 150 + 5 + 3*40]

Enemy Board Position: (650, 150)
    ↓
(Same calculation relative to 650)


Coordinate Conversion
─────────────────────
Screen → Board:
    boardX = (mouseX - boardPosX - PADDING) / CELL_SIZE
    boardY = (mouseY - boardPosY - PADDING) / CELL_SIZE

Board → Screen:
    screenX = boardPosX + PADDING + col * CELL_SIZE
    screenY = boardPosY + PADDING + row * CELL_SIZE

Board → User (Display):
    row letter = 'A' + row
    col number = col + 1
    result = "A5"
```

## Build Process

```
Source Files              CMake              Compilation
─────────────────────────────────────────────────────────

GameLogic.h         ─┐
                     ├─→ game_logic target
GameLogic.cpp       ─┘        ↓
                           Compile to
                          game_logic.a
                          (static lib)
                              ↓
                    ┌─────────┴─────────┐
                    │                   │
                    ↓                   ↓
main.cpp  ──────→ Link            GameGUI.h  ─┐
                    ↓              GameGUI.cpp ─┤
           fleet_commander                      ├─→ Link with
           (terminal exe)          main_gui.cpp ─┘  - game_logic.a
                                                     - SFML libs
                                       ↓
                                 fleet_commander_gui
                                    (GUI exe)

Dependencies Check
──────────────────
CMake checks for SFML:
    ├─ Found → Build GUI version
    └─ Not found → Build terminal only
                   (show warning)
```

## State Transitions

```
                    ┌────────┐
                    │  Menu  │
                    └───┬────┘
                        │ Click "Start"
                        ↓
                ┌───────────────┐
                │ PlacingShips  │
                │               │
                │ Ship 1 → Ship 2
                │    → Ship 3   │
                │    → Ship 4   │
                │    → Ship 5   │
                └───────┬───────┘
                        │ All placed
                        ↓
                ┌──────────────┐
                │  PlayerTurn  │◄────┐
                └──────┬───────┘     │
                       │ Attack      │
                       ↓             │
                ┌──────────────┐    │
                │ ComputerTurn │    │
                └──────┬───────┘    │
                       │ Attack     │
                       ↓            │
                  Game Over?        │
                   ├─ No ──────────┘
                   │
                   └─ Yes
                       ↓
                ┌──────────────┐
                │   GameOver   │
                │              │
                │ Victory or   │
                │   Defeat     │
                └──────┬───────┘
                       │ Click "Play Again"
                       ↓
                    (Back to Menu)
```

---

**For more details, see:**
- Code: `src/GameGUI.h` and `src/GameGUI.cpp`
- Logic: `src/GameLogic.h` and `src/GameLogic.cpp`
- Build: `CMakeLists.txt`
- Docs: `README_GUI.md`, `DEVELOPER.md`
