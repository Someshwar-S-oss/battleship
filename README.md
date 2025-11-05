# Fleet Commander

Fleet Commander is a terminal-based Battleship-style strategy game written in modern C++. Deploy your fleet, take turns trading fire with the computer, and sink every enemy ship before yours go under.

## Gameplay Highlights

- Classic 10×10 grid with coordinates `A1` through `J10`.
- Five-ship fleet per side: Aircraft Carrier, Battleship, Cruiser, Submarine, and Destroyer.
- Clear hit, miss, and sunk messages plus ANSI-colored board renders for quick readability.
- Computer fleet deployments are persisted to `placement.txt` so the AI can re-use prior layouts.
- Turn-by-turn pauses and short animations keep the action easy to follow.

## Build Requirements

- CMake 3.16+
- A C++17-compliant compiler (e.g., `clang++` 11+, `g++` 9+)

## Build and Run

```bash
cmake -S . -B build
cmake --build build
./build/fleet_commander
```

If you're using Windows PowerShell, replace the final line with `build\Debug\fleet_commander.exe` (or the appropriate configuration output path).

## Resetting Computer Placements

The computer saves its fleet layout to `placement.txt`. Delete this file before launching the game to force a fresh random deployment.
