#include "GameLogic.h"
#include <cctype>
#include <iostream>

// Utility function for colorizing console output
static std::string colorizeSymbol(char symbol)
{
    constexpr const char *RED = "\033[31m";
    constexpr const char *BLUE = "\033[34m";
    constexpr const char *GREEN = "\033[32m";
    constexpr const char *RESET = "\033[0m";

    switch (symbol)
    {
    case 'X':
        return std::string(RED) + 'X' + RESET;
    case 'O':
        return std::string(BLUE) + 'O' + RESET;
    case 'S':
        return std::string(GREEN) + 'S' + RESET;
    default:
        return std::string(1, symbol);
    }
}

// ============================================================================
// Ship Implementation
// ============================================================================

Ship::Ship(std::string shipName, int shipSize)
    : name(std::move(shipName)), size(shipSize)
{
}

void Ship::setPositions(const std::vector<Coordinate> &coords)
{
    positions = coords;
    hits.assign(coords.size(), false);
}

bool Ship::isPlaced() const
{
    return static_cast<int>(positions.size()) == size;
}

bool Ship::occupies(const Coordinate &coord) const
{
    for (const auto &pos : positions)
    {
        if (pos.first == coord.first && pos.second == coord.second)
        {
            return true;
        }
    }
    return false;
}

bool Ship::registerHit(const Coordinate &coord)
{
    for (std::size_t i = 0; i < positions.size(); ++i)
    {
        if (positions[i] == coord)
        {
            hits[i] = true;
            return true;
        }
    }
    return false;
}

bool Ship::isSunk() const
{
    if (hits.empty())
    {
        return false;
    }
    return std::all_of(hits.begin(), hits.end(), [](bool hit)
                       { return hit; });
}

void Ship::reset()
{
    positions.clear();
    hits.clear();
}

// ============================================================================
// Board Implementation
// ============================================================================

Board::Board()
{
    clear();
}

void Board::clear()
{
    for (auto &row : grid)
    {
        for (auto &cell : row)
        {
            cell.ship = nullptr;
            cell.attacked = false;
        }
    }
    ships.clear();
}

bool Board::placeShip(Ship &ship, const Coordinate &start, bool horizontal)
{
    std::vector<Coordinate> prospective;
    prospective.reserve(ship.getSize());

    for (int i = 0; i < ship.getSize(); ++i)
    {
        int row = start.first + (horizontal ? 0 : i);
        int col = start.second + (horizontal ? i : 0);

        Coordinate candidate{row, col};
        if (!inBounds(candidate) || isOccupied(candidate))
        {
            return false;
        }
        prospective.push_back(candidate);
    }

    ship.setPositions(prospective);

    for (const auto &coord : prospective)
    {
        Cell &cell = grid[coord.first][coord.second];
        cell.ship = &ship;
    }

    if (std::find(ships.begin(), ships.end(), &ship) == ships.end())
    {
        ships.push_back(&ship);
    }

    return true;
}

Board::AttackResult Board::attack(const Coordinate &target, std::string &shipName)
{
    if (!inBounds(target))
    {
        return AttackResult::Invalid;
    }

    Cell &cell = grid[target.first][target.second];
    if (cell.attacked)
    {
        return AttackResult::AlreadyTried;
    }

    cell.attacked = true;

    if (cell.ship == nullptr)
    {
        return AttackResult::Miss;
    }

    Ship *hitShip = cell.ship;
    hitShip->registerHit(target);
    shipName = hitShip->getName();

    if (hitShip->isSunk())
    {
        return AttackResult::Sunk;
    }

    return AttackResult::Hit;
}

bool Board::allShipsSunk() const
{
    return std::all_of(ships.begin(), ships.end(), [](const Ship *ship)
                       { return ship->isSunk(); });
}

bool Board::inBounds(const Coordinate &coord) const
{
    return coord.first >= 0 && coord.first < SIZE && coord.second >= 0 && coord.second < SIZE;
}

bool Board::isOccupied(const Coordinate &coord) const
{
    const Cell &cell = grid[coord.first][coord.second];
    return cell.ship != nullptr;
}

bool Board::isAttacked(const Coordinate &coord) const
{
    if (!inBounds(coord))
        return false;
    return grid[coord.first][coord.second].attacked;
}

bool Board::hasShipAt(const Coordinate &coord) const
{
    if (!inBounds(coord))
        return false;
    return grid[coord.first][coord.second].ship != nullptr;
}

char Board::getCellSymbol(const Coordinate &coord, bool showShips) const
{
    if (!inBounds(coord))
        return '~';

    const Cell &cell = grid[coord.first][coord.second];
    
    if (cell.attacked)
    {
        return cell.ship ? 'X' : 'O';
    }
    else if (cell.ship && showShips)
    {
        return 'S';
    }
    
    return '~';
}

void Board::displayOwn() const
{
    display(false);
}

void Board::displayFogged() const
{
    display(true);
}

void Board::display(bool hideShips) const
{
    std::cout << "    ";
    for (int col = 1; col <= SIZE; ++col)
    {
        if (col < 10)
        {
            std::cout << ' ';
        }
        std::cout << col << ' ';
    }
    std::cout << "\n";

    for (int row = 0; row < SIZE; ++row)
    {
        char label = static_cast<char>('A' + row);
        std::cout << ' ' << label << "  ";
        for (int col = 0; col < SIZE; ++col)
        {
            const Cell &cell = grid[row][col];
            char symbol = '~';
            if (cell.attacked)
            {
                symbol = cell.ship ? 'X' : 'O';
            }
            else if (cell.ship && !hideShips)
            {
                symbol = 'S';
            }
            const std::string decorated = colorizeSymbol(symbol);
            std::cout << ' ' << decorated << ' ';
        }
        std::cout << "\n";
    }
}
