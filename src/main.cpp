#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using Coordinate = std::pair<int, int>;

static std::string coordinateToString(const Coordinate &coord)
{
    char letter = static_cast<char>('A' + coord.first);
    return std::string(1, letter) + std::to_string(coord.second + 1);
}

static bool parseCoordinate(const std::string &input, Coordinate &out)
{
    std::string trimmed;
    trimmed.reserve(input.size());
    for (char ch : input)
    {
        if (!std::isspace(static_cast<unsigned char>(ch)))
        {
            trimmed.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
        }
    }

    if (trimmed.size() < 2)
    {
        return false;
    }

    char rowChar = trimmed.front();
    if (rowChar < 'A' || rowChar > 'J')
    {
        return false;
    }

    std::string columnPart = trimmed.substr(1);
    if (columnPart.empty())
    {
        return false;
    }

    for (char digit : columnPart)
    {
        if (!std::isdigit(static_cast<unsigned char>(digit)))
        {
            return false;
        }
    }

    int columnIndex = 0;
    try
    {
        columnIndex = std::stoi(columnPart);
    }
    catch (...)
    {
        return false;
    }

    if (columnIndex < 1 || columnIndex > 10)
    {
        return false;
    }

    out = Coordinate{rowChar - 'A', columnIndex - 1};
    return true;
}

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

class Ship
{
public:
    Ship(std::string shipName, int shipSize) : name(std::move(shipName)), size(shipSize) {}
    virtual ~Ship() = default;

    const std::string &getName() const
    {
        return name;
    }

    virtual std::string getType() const
    {
        return name;
    }

    int getSize() const
    {
        return size;
    }

    void setPositions(const std::vector<Coordinate> &coords)
    {
        positions = coords;
        hits.assign(coords.size(), false);
    }

    const std::vector<Coordinate> &getPositions() const
    {
        return positions;
    }

    bool isPlaced() const
    {
        return static_cast<int>(positions.size()) == size;
    }

    bool occupies(const Coordinate &coord) const
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

    bool registerHit(const Coordinate &coord)
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

    bool isSunk() const
    {
        if (hits.empty())
        {
            return false;
        }
        return std::all_of(hits.begin(), hits.end(), [](bool hit)
                           { return hit; });
    }

    void reset()
    {
        positions.clear();
        hits.clear();
    }

private:
    std::string name;
    int size;
    std::vector<Coordinate> positions;
    std::vector<bool> hits;
};

class AircraftCarrier : public Ship
{
public:
    AircraftCarrier() : Ship("Aircraft Carrier", 5) {}
    std::string getType() const override
    {
        return "Aircraft Carrier";
    }
};

class Battleship : public Ship
{
public:
    Battleship() : Ship("Battleship", 4) {}
    std::string getType() const override
    {
        return "Battleship";
    }
};

class Cruiser : public Ship
{
public:
    Cruiser() : Ship("Cruiser", 3) {}
    std::string getType() const override
    {
        return "Cruiser";
    }
};

class Submarine : public Ship
{
public:
    Submarine() : Ship("Submarine", 3) {}
    std::string getType() const override
    {
        return "Submarine";
    }
};

class Destroyer : public Ship
{
public:
    Destroyer() : Ship("Destroyer", 2) {}
    std::string getType() const override
    {
        return "Destroyer";
    }
};

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

    Board()
    {
        clear();
    }

    void clear()
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

    bool placeShip(Ship &ship, const Coordinate &start, bool horizontal)
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

    AttackResult attack(const Coordinate &target, std::string &shipName)
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

    bool allShipsSunk() const
    {
        return std::all_of(ships.begin(), ships.end(), [](const Ship *ship)
                           { return ship->isSunk(); });
    }

    void displayOwn() const
    {
        display(false);
    }

    void displayFogged() const
    {
        display(true);
    }

private:
    struct Cell
    {
        Ship *ship = nullptr;
        bool attacked = false;
    };

    std::array<std::array<Cell, SIZE>, SIZE> grid{};
    std::vector<Ship *> ships;

    bool inBounds(const Coordinate &coord) const
    {
        return coord.first >= 0 && coord.first < SIZE && coord.second >= 0 && coord.second < SIZE;
    }

    bool isOccupied(const Coordinate &coord) const
    {
        const Cell &cell = grid[coord.first][coord.second];
        return cell.ship != nullptr;
    }

    void display(bool hideShips) const
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
};

class Game
{
public:
    Game() : placementFile("placement.txt"), rng(std::random_device{}())
    {
        createFleet(playerFleet);
        createFleet(computerFleet);
        refillComputerShots();
    }

    void run()
    {
        showWelcome();

        waitForEnter("Press Enter when you're ready to deploy your fleet...");
        std::cout << "\n";

        setupPlayerFleet();
        setupComputerFleet();

        std::cout << "\nBattle commencing!\n";
        showBoards();

        while (true)
        {
            if (playerTurn())
            {
                std::cout << "\nCongratulations, Admiral. You secured the seas!\n";
                break;
            }
            waitForEnter();

            if (computerTurn())
            {
                std::cout << "\nYour fleet has been destroyed. Mission failed.\n";
                break;
            }
            waitForEnter();
        }
    }

private:
    Board playerBoard;
    Board computerBoard;
    std::vector<std::unique_ptr<Ship>> playerFleet;
    std::vector<std::unique_ptr<Ship>> computerFleet;
    std::string placementFile;
    std::vector<Coordinate> computerShots;
    std::mt19937 rng;

    void waitForEnter(const std::string &prompt = "Press Enter to continue...") const
    {
        std::cout << prompt;
        std::cout.flush();
        if (!std::cin.good())
        {
            std::cin.clear();
        }
        std::string line;
        std::getline(std::cin, line);
    }

    static void createFleet(std::vector<std::unique_ptr<Ship>> &fleet)
    {
        fleet.clear();
        fleet.emplace_back(std::make_unique<AircraftCarrier>());
        fleet.emplace_back(std::make_unique<Battleship>());
        fleet.emplace_back(std::make_unique<Cruiser>());
        fleet.emplace_back(std::make_unique<Submarine>());
        fleet.emplace_back(std::make_unique<Destroyer>());
    }

    void refillComputerShots()
    {
        computerShots.clear();
        computerShots.reserve(Board::SIZE * Board::SIZE);
        for (int row = 0; row < Board::SIZE; ++row)
        {
            for (int col = 0; col < Board::SIZE; ++col)
            {
                computerShots.emplace_back(row, col);
            }
        }
        std::shuffle(computerShots.begin(), computerShots.end(), rng);
    }

    void showWelcome() const
    {
        std::cout << "=== Fleet Commander ===\n";
        std::cout << "Welcome to Fleet Commander!\n";
        std::cout << "Rules:\n";
        std::cout << " - Grid size: 10x10 (A–J, 1–10)\n";
        std::cout << " - You and the computer each have 5 ships.\n";
        std::cout << " - Take turns firing coordinates like A5 or D10.\n";
        std::cout << " - 'X' = hit, 'O' = miss, 'S' = your ship.\n";
        std::cout << " - Sink all enemy ships to win.\n\n";
    }

    void setupPlayerFleet()
    {
        std::cout << "Player Board (S marks your ships)\n";
        playerBoard.displayOwn();

        for (auto &ship : playerFleet)
        {
            bool placed = false;
            while (!placed)
            {
                std::cout << "\nPlacing " << ship->getType() << " (length " << ship->getSize() << ")" << std::endl;
                Coordinate start{};
                if (!promptCoordinate("Enter starting coordinate (e.g., A1): ", start))
                {
                    continue;
                }

                bool horizontal = promptOrientation();

                if (playerBoard.placeShip(*ship, start, horizontal))
                {
                    std::cout << ship->getName() << " positioned at " << coordinateToString(start)
                              << (horizontal ? " horizontally." : " vertically.") << std::endl;
                    playerBoard.displayOwn();
                    placed = true;
                }
                else
                {
                    std::cout << "Invalid placement. Ensure ships stay within bounds and avoid overlap." << std::endl;
                }
            }
        }
    }

    void setupComputerFleet()
    {
        if (loadComputerPlacements())
        {
            std::cout << "\nEnemy fleet loaded from saved deployment." << std::endl;
            return;
        }

        std::cout << "\nEnemy fleet deploying..." << std::endl;
        generateComputerPlacements();
        saveComputerPlacements();
    }

    bool promptCoordinate(const std::string &label, Coordinate &out) const
    {
        while (true)
        {
            std::cout << label;
            std::string input;
            if (!std::getline(std::cin, input))
            {
                return false;
            }
            if (parseCoordinate(input, out))
            {
                return true;
            }
            std::cout << "Invalid coordinate. Use rows A-J and columns 1-10." << std::endl;
        }
    }

    bool promptOrientation() const
    {
        while (true)
        {
            std::cout << "Orientation (H for horizontal, V for vertical): ";
            std::string input;
            if (!std::getline(std::cin, input))
            {
                continue;
            }
            if (input.empty())
            {
                continue;
            }
            char value = static_cast<char>(std::toupper(static_cast<unsigned char>(input.front())));
            if (value == 'H')
            {
                return true;
            }
            if (value == 'V')
            {
                return false;
            }
            std::cout << "Please enter H or V." << std::endl;
        }
    }

    bool playerTurn()
    {
        std::cout << "\nYour turn." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        while (true)
        {
            Coordinate target{};
            if (!promptCoordinate("Enter target (e.g., B7): ", target))
            {
                continue;
            }

            std::string shipName;
            Board::AttackResult result = computerBoard.attack(target, shipName);

            if (result == Board::AttackResult::Invalid)
            {
                std::cout << "Coordinate lies outside the battle grid." << std::endl;
                continue;
            }
            if (result == Board::AttackResult::AlreadyTried)
            {
                std::cout << "You've already fired at that location." << std::endl;
                continue;
            }

            announceResult("You", target, result, shipName);
            showBoards();

            return computerBoard.allShipsSunk();
        }
    }

    bool computerTurn()
    {
        std::cout << "\nEnemy turn." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(800));

        while (true)
        {
            if (computerShots.empty())
            {
                refillComputerShots();
            }

            Coordinate target = computerShots.back();
            computerShots.pop_back();

            std::string shipName;
            Board::AttackResult result = playerBoard.attack(target, shipName);

            if (result == Board::AttackResult::Invalid || result == Board::AttackResult::AlreadyTried)
            {
                continue;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            announceResult("Enemy", target, result, shipName);
            showBoards();

            return playerBoard.allShipsSunk();
        }
    }

    void announceResult(const std::string &attacker, const Coordinate &target, Board::AttackResult result,
                        const std::string &shipName) const
    {
        std::string label = coordinateToString(target);
        const bool isPlayer = attacker == "You";
        const std::string fireVerb = isPlayer ? "fire" : "fires";
        const std::string missClause = isPlayer ? "miss." : "misses.";
        const std::string hitClause = isPlayer ? "hit!" : "hits!";
        const std::string sinkVerb = isPlayer ? "sink" : "sinks";

        switch (result)
        {
        case Board::AttackResult::Miss:
            std::cout << attacker << ' ' << fireVerb << " at " << label << " and " << missClause << std::endl;
            break;
        case Board::AttackResult::Hit:
            std::cout << attacker << ' ' << fireVerb << " at " << label << " and " << hitClause << std::endl;
            break;
        case Board::AttackResult::Sunk:
            std::cout << attacker << ' ' << sinkVerb << " the " << shipName << " at " << label << "!" << std::endl;
            break;
        default:
            break;
        }
    }

    void showBoards() const
    {
        std::cout << "\nYour Fleet:" << std::endl;
        playerBoard.displayOwn();
        std::cout << "\nEnemy Waters:" << std::endl;
        computerBoard.displayFogged();
    }

    bool loadComputerPlacements()
    {
        std::ifstream input(placementFile);
        if (!input)
        {
            return false;
        }

        computerBoard.clear();
        for (auto &ship : computerFleet)
        {
            ship->reset();
        }

        std::vector<Ship *> unused;
        for (auto &ship : computerFleet)
        {
            unused.push_back(ship.get());
        }

        std::string line;
        int placed = 0;
        while (std::getline(input, line))
        {
            if (line.empty())
            {
                continue;
            }
            std::istringstream iss(line);
            std::string nameToken;
            std::string coordToken;
            char orient = 0;
            if (!(iss >> nameToken >> coordToken >> orient))
            {
                return false;
            }

            Ship *targetShip = nullptr;
            for (Ship *candidate : unused)
            {
                std::string sanitized = sanitizeShipName(candidate->getName());
                if (sanitized == nameToken)
                {
                    targetShip = candidate;
                    break;
                }
            }

            if (!targetShip)
            {
                return false;
            }

            Coordinate start{};
            if (!parseCoordinate(coordToken, start))
            {
                return false;
            }

            bool horizontal;
            if (orient == 'H' || orient == 'h')
            {
                horizontal = true;
            }
            else if (orient == 'V' || orient == 'v')
            {
                horizontal = false;
            }
            else
            {
                return false;
            }

            if (!computerBoard.placeShip(*targetShip, start, horizontal))
            {
                return false;
            }

            unused.erase(std::remove(unused.begin(), unused.end(), targetShip), unused.end());
            ++placed;
        }

        return placed == static_cast<int>(computerFleet.size());
    }

    void generateComputerPlacements()
    {
        std::uniform_int_distribution<int> orientationDist(0, 1);
        while (true)
        {
            computerBoard.clear();
            for (auto &ship : computerFleet)
            {
                ship->reset();
            }

            bool success = true;
            for (auto &ship : computerFleet)
            {
                bool placed = false;
                for (int attempt = 0; attempt < 500 && !placed; ++attempt)
                {
                    bool horizontal = orientationDist(rng) == 0;
                    int maxRow = horizontal ? Board::SIZE - 1 : Board::SIZE - ship->getSize();
                    int maxCol = horizontal ? Board::SIZE - ship->getSize() : Board::SIZE - 1;

                    if (maxRow < 0 || maxCol < 0)
                    {
                        continue;
                    }

                    std::uniform_int_distribution<int> rowDist(0, maxRow);
                    std::uniform_int_distribution<int> colDist(0, maxCol);

                    Coordinate start{rowDist(rng), colDist(rng)};

                    if (computerBoard.placeShip(*ship, start, horizontal))
                    {
                        placed = true;
                    }
                }

                if (!placed)
                {
                    success = false;
                    break;
                }
            }

            if (success)
            {
                return;
            }
        }
    }

    void saveComputerPlacements() const
    {
        std::ofstream output(placementFile, std::ios::trunc);
        if (!output)
        {
            std::cerr << "Warning: Unable to save computer placement to " << placementFile << std::endl;
            return;
        }

        for (const auto &ship : computerFleet)
        {
            const auto &positions = ship->getPositions();
            if (positions.empty())
            {
                continue;
            }

            char orientation = 'H';
            if (positions.size() >= 2 && positions[0].first != positions[1].first)
            {
                orientation = 'V';
            }

            output << sanitizeShipName(ship->getName()) << ' ' << coordinateToString(positions.front()) << ' '
                   << orientation << '\n';
        }
    }

    static std::string sanitizeShipName(const std::string &name)
    {
        std::string result;
        for (char ch : name)
        {
            if (ch != ' ')
            {
                result.push_back(ch);
            }
        }
        return result;
    }
};

int main()
{
    Game game;
    game.run();
    return 0;
}
