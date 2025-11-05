#pragma once

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using Coordinate = std::pair<int, int>;

// Ship class - represents a ship on the board
class Ship
{
public:
    Ship(std::string shipName, int shipSize);
    virtual ~Ship() = default;

    const std::string &getName() const { return name; }
    virtual std::string getType() const { return name; }
    int getSize() const { return size; }

    void setPositions(const std::vector<Coordinate> &coords);
    const std::vector<Coordinate> &getPositions() const { return positions; }

    bool isPlaced() const;
    bool occupies(const Coordinate &coord) const;
    bool registerHit(const Coordinate &coord);
    bool isSunk() const;
    void reset();

private:
    std::string name;
    int size;
    std::vector<Coordinate> positions;
    std::vector<bool> hits;
};

// Specific ship types
class AircraftCarrier : public Ship
{
public:
    AircraftCarrier() : Ship("Aircraft Carrier", 5) {}
    std::string getType() const override { return "Aircraft Carrier"; }
};

class Battleship : public Ship
{
public:
    Battleship() : Ship("Battleship", 4) {}
    std::string getType() const override { return "Battleship"; }
};

class Cruiser : public Ship
{
public:
    Cruiser() : Ship("Cruiser", 3) {}
    std::string getType() const override { return "Cruiser"; }
};

class Submarine : public Ship
{
public:
    Submarine() : Ship("Submarine", 3) {}
    std::string getType() const override { return "Submarine"; }
};

class Destroyer : public Ship
{
public:
    Destroyer() : Ship("Destroyer", 2) {}
    std::string getType() const override { return "Destroyer"; }
};

// Board class - manages the game grid
class Board
{
public:
    static constexpr int SIZE = 10;

    enum class AttackResult
    {
        Invalid,
        AlreadyTried,
        Miss,
        Hit,
        Sunk
    };

    Board();

    void clear();
    bool placeShip(Ship &ship, const Coordinate &start, bool horizontal);
    AttackResult attack(const Coordinate &target, std::string &shipName);
    bool allShipsSunk() const;

    void displayOwn() const;
    void displayFogged() const;

    // Accessors for GUI
    bool isOccupied(const Coordinate &coord) const;
    bool isAttacked(const Coordinate &coord) const;
    bool hasShipAt(const Coordinate &coord) const;
    char getCellSymbol(const Coordinate &coord, bool showShips) const;
    const std::vector<Ship *> &getShips() const { return ships; }

private:
    struct Cell
    {
        Ship *ship = nullptr;
        bool attacked = false;
    };

    std::array<std::array<Cell, SIZE>, SIZE> grid{};
    std::vector<Ship *> ships;

    bool inBounds(const Coordinate &coord) const;
    void display(bool hideShips) const;
};
