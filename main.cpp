// Combined single-file version generated from repo_sync
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <filesystem>

using Coordinate = std::pair<int, int>;

// ============================================================================
// Game Logic Declarations
// ============================================================================
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
    bool isShipSunkAt(const Coordinate &coord) const;
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

// ============================================================================
// Game Logic Implementation
// ============================================================================
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

bool Board::isShipSunkAt(const Coordinate &coord) const
{
    if (!inBounds(coord))
        return false;

    const Cell &cell = grid[coord.first][coord.second];
    return cell.ship != nullptr && cell.ship->isSunk();
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

// ============================================================================
// GUI Declarations
// ============================================================================
// Color palette for pixel art style
namespace Colors
{
    const sf::Color Ocean(41, 128, 185);        // Deep blue ocean
    const sf::Color OceanDark(52, 73, 94);      // Darker ocean for grid
    const sf::Color Ship(149, 165, 166);        // Gray ship
    const sf::Color ShipDark(127, 140, 141);    // Darker gray for ship outline
    const sf::Color Hit(231, 76, 60);           // Red hit
    const sf::Color Miss(236, 240, 241);        // White miss
    const sf::Color Highlight(241, 196, 15);    // Yellow highlight
    const sf::Color Text(236, 240, 241);        // White text
    const sf::Color Background(44, 62, 80);     // Dark blue background
    const sf::Color ButtonNormal(52, 152, 219); // Blue button
    const sf::Color ButtonHover(41, 128, 185);  // Darker blue hover
}

// Game states
enum class GameState
{
    Menu,
    Settings,
    PlacingShips,
    PlayerTurn,
    ComputerTurn,
    GameOver
};

// Difficulty levels
enum class Difficulty
{
    Easy,
    Medium,
    Hard
};

// Particle types
enum class ParticleType
{
    Explosion,
    WaterSplash,
    Confetti
};

// Button class for UI elements
class Button
{
public:
    Button(const sf::Vector2f &position, const sf::Vector2f &size, const std::string &text, sf::Font &font);

    void draw(sf::RenderWindow &window);
    bool isHovered(const sf::Vector2i &mousePos) const;
    bool isClicked(const sf::Vector2i &mousePos, sf::Event::MouseButtonEvent event) const;
    void setPosition(const sf::Vector2f &position);
    void setBackgroundVisible(bool visible);
    void setTextColor(const sf::Color &color);
    void setHoverTextColor(const sf::Color &color);

private:
    sf::RectangleShape shape;
    sf::Text label;
    sf::Vector2f size;
    bool hovered = false;
    bool backgroundVisible = true;
    sf::Color baseTextColor = Colors::Text;
    sf::Color hoverTextColor = Colors::Highlight;
};

// Board view for rendering game boards
class BoardView
{
public:
    static constexpr int BOARD_SIZE = 10;
    static constexpr float CELL_SIZE = 60.0f;
    static constexpr float GRID_PADDING = 8.0f;

    BoardView(const sf::Vector2f &position, bool showShips);

    void draw(sf::RenderWindow &window, const Board &board, sf::Font &font);
    void draw(sf::RenderWindow &window, const Board &board, sf::Font &font, const std::map<std::string, sf::Texture> &shipTextures);
    void drawPlacementPreview(sf::RenderWindow &window, const Coordinate &start, int shipSize, bool horizontal, bool valid);
    
    bool getCellFromMouse(const sf::Vector2i &mousePos, Coordinate &out) const;
    sf::Vector2f getCellCenter(const Coordinate &coord) const;
    
    void highlightCell(sf::RenderWindow &window, const Coordinate &coord, const sf::Color &color);
    void setShowShips(bool show) { showShips = show; }
    void setUseSprites(bool use) { useSprites = use; }

private:
    sf::Vector2f position;
    bool showShips;
    bool useSprites = false;

    void drawGrid(sf::RenderWindow &window);
    void drawCell(sf::RenderWindow &window, const Board &board, int row, int col, char symbol);
    void drawShipSprite(sf::RenderWindow &window, const Board &board, const Ship &ship, const std::map<std::string, sf::Texture> &shipTextures);
    void drawLabel(sf::RenderWindow &window, const std::string &text, const sf::Vector2f &pos, sf::Font &font);
    char getCellSymbol(const Board &board, int row, int col) const;
};

// Message box for displaying game messages
class MessageBox
{
public:
    MessageBox(const sf::Vector2f &position, const sf::Vector2f &size, sf::Font &font);

    void setMessage(const std::string &message);
    void addMessage(const std::string &message);
    void draw(sf::RenderWindow &window);
    void clear();

private:
    sf::RectangleShape background;
    sf::Text messageText;
    std::vector<std::string> messages;
    sf::Vector2f size;
    sf::Font &font;

    void updateDisplay();
};

// Ship placement helper
struct PlacementState
{
    int currentShipIndex = 0;
    bool horizontal = true;
    Coordinate previewStart{0, 0};
    bool previewValid = false;
};

// Main game GUI class
class GameGUI
{
public:
    GameGUI();
    ~GameGUI();

    void run();

private:
    // Window and rendering
    sf::RenderWindow window;
    sf::Font font;
    sf::Clock deltaClock;
    
    // Ship textures
    std::map<std::string, sf::Texture> shipTextures;
    bool useShipSprites = false;

    struct ShipPortrait
    {
        std::string name;
        sf::Texture texture;
        sf::Sprite sprite;
        bool loaded = false;
    };
    std::vector<ShipPortrait> shipPortraits;
    
    // Menu background
    sf::Texture menuBackgroundTexture;
    sf::Sprite menuBackgroundSprite;
    bool hasMenuBackground = false;
    
    // Audio
    sf::Music menuMusic;
    sf::Music battleMusic;
    sf::Music victoryMusic;
    sf::Music defeatMusic;
    sf::SoundBuffer hitSoundBuffer, missSoundBuffer, sinkSoundBuffer;
    sf::Sound hitSound, missSound, sinkSound;
    float musicVolume = 50.0f;
    float sfxVolume = 70.0f;
    bool musicInitialized = false;
    bool menuMusicLoaded = false;
    bool battleMusicLoaded = false;
    bool victoryMusicLoaded = false;
    bool defeatMusicLoaded = false;
    
    // Game state
    GameState state = GameState::Menu;
    bool playerWon = false;
    Difficulty difficulty = Difficulty::Medium;
    
    // Statistics
    struct GameStats {
        int gamesPlayed = 0;
        int gamesWon = 0;
        int gamesLost = 0;
        int totalShotsFired = 0;
        int totalHits = 0;
        float getAccuracy() const {
            return totalShotsFired > 0 ? (100.0f * totalHits / totalShotsFired) : 0.0f;
        }
    };
    GameStats stats;
    int currentGameShots = 0;
    int currentGameHits = 0;
    
    // Computer AI
    std::vector<Coordinate> hitQueue; // For smart AI targeting
    Coordinate lastHit{-1, -1};
    bool huntingMode = false;
    
    // Game logic (from existing code)
    std::unique_ptr<Board> playerBoard;
    std::unique_ptr<Board> computerBoard;
    std::vector<std::unique_ptr<Ship>> playerFleet;
    std::vector<std::unique_ptr<Ship>> computerFleet;
    
    // GUI components
    std::unique_ptr<BoardView> playerBoardView;
    std::unique_ptr<BoardView> computerBoardView;
    std::unique_ptr<MessageBox> messageBox;
    std::vector<std::unique_ptr<Button>> buttons;
    
    // Ship placement
    PlacementState placementState;
    
    // Animation and effects
    struct Particle
    {
        sf::CircleShape shape;
        sf::Vector2f velocity;
        float lifetime;
        float maxLifetime;
        ParticleType type;
        sf::Color startColor;
    };
    std::vector<Particle> particles;
    
    // Water animation
    sf::Texture waterTexture;
    sf::Sprite waterSprite;
    float waterScrollOffset = 0.0f;
    bool hasWaterTexture = false;
    
    // Fade effect
    sf::RectangleShape fadeOverlay;
    float fadeAlpha = 0.0f;
    
    // Timing for computer turn
    sf::Clock actionClock;
    bool waitingForAction = false;
    float actionDelay = 1.0f;
    
    // Initialize functions
    void initWindow();
    void initFont();
    void initGameObjects();
    void initShipTextures();
    void initShipPortraits();
    void initMenuBackground();
    void initAudio();
    void initWaterBackground();
    void createFleet(std::vector<std::unique_ptr<Ship>> &fleet);
    
    // Statistics
    void loadStats();
    void saveStats();
    void updateStatsOnGameEnd(bool won);
    
    // State management
    void changeState(GameState newState);
    
    // Event handling
    void processEvents();
    void handleMenuEvents(sf::Event &event);
    void handleSettingsEvents(sf::Event &event);
    void handlePlacementEvents(sf::Event &event);
    void handleBattleEvents(sf::Event &event);
    void handleGameOverEvents(sf::Event &event);
    
    // Update logic
    void update(float deltaTime);
    void updateParticles(float deltaTime);
    void updateComputerTurn();
    
    // Rendering
    void render();
    void renderMenu();
    void renderSettings();
    void renderPlacement();
    void renderBattle();
    void renderShipStatusPanel();
    void renderGameOver();
    void renderWaterBackground();
    
    // Ship placement logic
    void setupPlayerFleet();
    void placeCurrentShip(const Coordinate &start, bool horizontal);
    void finishPlacement();
    
    // Computer AI
    void setupComputerFleet();
    void generateComputerPlacements();
    bool loadComputerPlacements();
    void saveComputerPlacements() const;
    bool executeComputerAttack();
    void refillComputerShots();
    std::vector<Coordinate> computerShots;
    std::string placementFile = "placement.txt";
    
    // Battle logic
    void playerAttack(const Coordinate &target);
    void checkGameOver();
    
    // Visual effects
    void createHitEffect(const sf::Vector2f &position);
    void createMissEffect(const sf::Vector2f &position);
    void createConfetti(const sf::Vector2f &position);
    void createParticle(const sf::Vector2f &position, ParticleType type);
    void createSinkEffect(const sf::Vector2f &position);
    
    // UI helpers
    void drawTitle(const std::string &text, float yPosition);
    void drawCenteredText(const std::string &text, float yPosition, unsigned int size = 24);

    // Status helpers
    bool isComputerShipSunk(const std::string &shipName) const;
    
    // Utility
    static std::string coordinateToString(const Coordinate &coord);
    static std::string sanitizeShipName(const std::string &name);
};

// ============================================================================
// GUI Implementation
// ============================================================================
// Include the Ship and Board classes from main.cpp
// We'll need to extract these into a separate header file
// For now, we'll include them through a shared header

// Helper function to parse coordinates (from main.cpp)
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
        return false;

    char rowChar = trimmed.front();
    if (rowChar < 'A' || rowChar > 'J')
        return false;

    std::string columnPart = trimmed.substr(1);
    if (columnPart.empty())
        return false;

    for (char digit : columnPart)
    {
        if (!std::isdigit(static_cast<unsigned char>(digit)))
            return false;
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
        return false;

    out = Coordinate{rowChar - 'A', columnIndex - 1};
    return true;
}

static std::string sanitizeShipKey(const std::string &name)
{
    std::string result;
    result.reserve(name.size());
    for (char ch : name)
    {
        if (!std::isspace(static_cast<unsigned char>(ch)))
        {
            result.push_back(ch);
        }
    }
    return result;
}

// ============================================================================
// Button Implementation
// ============================================================================

Button::Button(const sf::Vector2f &position, const sf::Vector2f &size, const std::string &text, sf::Font &font)
    : size(size)
{
    shape.setPosition(position);
    shape.setSize(size);
    shape.setFillColor(Colors::ButtonNormal);
    shape.setOutlineColor(sf::Color::White);
    shape.setOutlineThickness(2.0f);

    label.setFont(font);
    label.setString(text);
    label.setCharacterSize(28);
    label.setFillColor(baseTextColor);

    // Center text in button
    sf::FloatRect textBounds = label.getLocalBounds();
    label.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    label.setPosition(position.x + size.x / 2.0f, position.y + size.y / 2.0f);
}

void Button::draw(sf::RenderWindow &window)
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    hovered = isHovered(mousePos);

    if (backgroundVisible)
    {
        shape.setFillColor(hovered ? Colors::ButtonHover : Colors::ButtonNormal);
        window.draw(shape);
    }

    label.setFillColor(hovered ? hoverTextColor : baseTextColor);
    window.draw(label);
}

bool Button::isHovered(const sf::Vector2i &mousePos) const
{
    sf::FloatRect bounds = shape.getGlobalBounds();
    return bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
}

bool Button::isClicked(const sf::Vector2i &mousePos, sf::Event::MouseButtonEvent event) const
{
    return event.button == sf::Mouse::Left && isHovered(mousePos);
}

void Button::setPosition(const sf::Vector2f &position)
{
    shape.setPosition(position);
    sf::FloatRect textBounds = label.getLocalBounds();
    label.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    label.setPosition(position.x + size.x / 2.0f, position.y + size.y / 2.0f);
}

void Button::setBackgroundVisible(bool visible)
{
    backgroundVisible = visible;
    if (!backgroundVisible)
    {
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineThickness(0.0f);
    }
    else
    {
        shape.setOutlineThickness(2.0f);
    }
}

void Button::setTextColor(const sf::Color &color)
{
    baseTextColor = color;
    label.setFillColor(baseTextColor);
}

void Button::setHoverTextColor(const sf::Color &color)
{
    hoverTextColor = color;
}

// ============================================================================
// BoardView Implementation
// ============================================================================

BoardView::BoardView(const sf::Vector2f &position, bool showShips)
    : position(position), showShips(showShips)
{
}

void BoardView::draw(sf::RenderWindow &window, const Board &board, sf::Font &font)
{
    drawGrid(window);

    // Draw column numbers
    for (int col = 0; col < BOARD_SIZE; ++col)
    {
        std::string num = std::to_string(col + 1);
        sf::Vector2f labelPos(position.x + GRID_PADDING + col * CELL_SIZE + CELL_SIZE / 2.0f, position.y - 25.0f);
        drawLabel(window, num, labelPos, font);
    }

    // Draw row letters
    for (int row = 0; row < BOARD_SIZE; ++row)
    {
        char letter = static_cast<char>('A' + row);
        std::string label(1, letter);
        sf::Vector2f labelPos(position.x - 25.0f, position.y + GRID_PADDING + row * CELL_SIZE + CELL_SIZE / 2.0f);
        drawLabel(window, label, labelPos, font);
    }

    // Draw cells
    for (int row = 0; row < BOARD_SIZE; ++row)
    {
        for (int col = 0; col < BOARD_SIZE; ++col)
        {
            char symbol = getCellSymbol(board, row, col);
            drawCell(window, board, row, col, symbol);
        }
    }
}

void BoardView::draw(sf::RenderWindow &window, const Board &board, sf::Font &font, const std::map<std::string, sf::Texture> &shipTextures)
{
    drawGrid(window);

    // Draw column numbers
    for (int col = 0; col < BOARD_SIZE; ++col)
    {
        std::string num = std::to_string(col + 1);
        sf::Vector2f labelPos(position.x + GRID_PADDING + col * CELL_SIZE + CELL_SIZE / 2.0f, position.y - 25.0f);
        drawLabel(window, num, labelPos, font);
    }

    // Draw row letters
    for (int row = 0; row < BOARD_SIZE; ++row)
    {
        char letter = static_cast<char>('A' + row);
        std::string label(1, letter);
        sf::Vector2f labelPos(position.x - 25.0f, position.y + GRID_PADDING + row * CELL_SIZE + CELL_SIZE / 2.0f);
        drawLabel(window, label, labelPos, font);
    }

    // Draw water cells (no ships yet)
    for (int row = 0; row < BOARD_SIZE; ++row)
    {
        for (int col = 0; col < BOARD_SIZE; ++col)
        {
            char symbol = getCellSymbol(board, row, col);
            if (symbol != 'S')  // Draw everything except ships
            {
                drawCell(window, board, row, col, symbol);
            }
        }
    }

    // Draw ships as sprites (if showing ships)
    if (showShips && useSprites)
    {
        const auto &ships = board.getShips();
        for (const Ship *ship : ships)
        {
            if (ship && ship->isPlaced())
            {
                drawShipSprite(window, board, *ship, shipTextures);
            }
        }
    }
    else if (showShips && !useSprites)
    {
        // Draw ships as colored rectangles (fallback)
        for (int row = 0; row < BOARD_SIZE; ++row)
        {
            for (int col = 0; col < BOARD_SIZE; ++col)
            {
                char symbol = getCellSymbol(board, row, col);
                if (symbol == 'S')  // Draw ships
                {
                    drawCell(window, board, row, col, symbol);
                }
            }
        }
    }
}

void BoardView::drawShipSprite(sf::RenderWindow &window, const Board &board, const Ship &ship, const std::map<std::string, sf::Texture> &shipTextures)
{
    auto it = shipTextures.find(sanitizeShipKey(ship.getName()));
    if (it == shipTextures.end() || !ship.isPlaced())
    {
        return; // Texture not found or ship not placed
    }

    const sf::Texture &texture = it->second;
    const auto &positions = ship.getPositions();
    
    if (positions.empty())
    {
        return;
    }

    // Determine if ship is horizontal or vertical
    bool horizontal = true;
    if (positions.size() >= 2)
    {
        horizontal = (positions[0].first == positions[1].first);
    }

    // Calculate sprite position and rotation
    Coordinate firstPos = positions.front();
    float x = position.x + GRID_PADDING + firstPos.second * CELL_SIZE;
    float y = position.y + GRID_PADDING + firstPos.first * CELL_SIZE;

    // Create sprite
    sf::Sprite sprite(texture);
    
    // Scale sprite to fit the ship cells
    float shipLength = ship.getSize() * CELL_SIZE;
    sf::Vector2u textureSize = texture.getSize();
    
    if (horizontal)
    {
        // Scale width to ship length, height to cell size
        sprite.setScale(shipLength / textureSize.x, CELL_SIZE / textureSize.y);
        sprite.setPosition(x, y);
    }
    else
    {
        // For vertical, rotate sprite and adjust scaling
        sprite.setScale(shipLength / textureSize.x, CELL_SIZE / textureSize.y);
        sprite.setRotation(90);
        sprite.setPosition(x, y);
        sprite.move(CELL_SIZE, 0); // Adjust for rotation origin
    }

    // Dim the sprite if ship is sunk
    if (ship.isSunk())
    {
        sprite.setColor(sf::Color(128, 128, 128, 180)); // Gray and semi-transparent
    }

    window.draw(sprite);
    
    // Draw hit markers on top of sprite
    for (size_t i = 0; i < positions.size(); ++i)
    {
        Coordinate coord = positions[i];
        if (board.isAttacked(coord))
        {
            float hitX = position.x + GRID_PADDING + coord.second * CELL_SIZE + CELL_SIZE / 2.0f;
            float hitY = position.y + GRID_PADDING + coord.first * CELL_SIZE + CELL_SIZE / 2.0f;
            
            sf::CircleShape hitMarker(CELL_SIZE / 4.0f);
            hitMarker.setFillColor(Colors::Hit);
            hitMarker.setPosition(hitX - hitMarker.getRadius(), hitY - hitMarker.getRadius());
            window.draw(hitMarker);
        }
    }
}

void BoardView::drawPlacementPreview(sf::RenderWindow &window, const Coordinate &start, int shipSize, bool horizontal, bool valid)
{
    sf::Color previewColor = valid ? sf::Color(100, 255, 100, 150) : sf::Color(255, 100, 100, 150);

    for (int i = 0; i < shipSize; ++i)
    {
        int row = start.first + (horizontal ? 0 : i);
        int col = start.second + (horizontal ? i : 0);

        if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE)
        {
            float x = position.x + GRID_PADDING + col * CELL_SIZE;
            float y = position.y + GRID_PADDING + row * CELL_SIZE;

            sf::RectangleShape preview(sf::Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));
            preview.setPosition(x + 1, y + 1);
            preview.setFillColor(previewColor);
            window.draw(preview);
        }
    }
}

bool BoardView::getCellFromMouse(const sf::Vector2i &mousePos, Coordinate &out) const
{
    float relX = mousePos.x - position.x - GRID_PADDING;
    float relY = mousePos.y - position.y - GRID_PADDING;

    int col = static_cast<int>(relX / CELL_SIZE);
    int row = static_cast<int>(relY / CELL_SIZE);

    if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE)
    {
        out = Coordinate{row, col};
        return true;
    }

    return false;
}

sf::Vector2f BoardView::getCellCenter(const Coordinate &coord) const
{
    float x = position.x + GRID_PADDING + coord.second * CELL_SIZE + CELL_SIZE / 2.0f;
    float y = position.y + GRID_PADDING + coord.first * CELL_SIZE + CELL_SIZE / 2.0f;
    return sf::Vector2f(x, y);
}

void BoardView::highlightCell(sf::RenderWindow &window, const Coordinate &coord, const sf::Color &color)
{
    float x = position.x + GRID_PADDING + coord.second * CELL_SIZE;
    float y = position.y + GRID_PADDING + coord.first * CELL_SIZE;

    sf::RectangleShape highlight(sf::Vector2f(CELL_SIZE, CELL_SIZE));
    highlight.setPosition(x, y);
    highlight.setFillColor(sf::Color::Transparent);
    highlight.setOutlineColor(color);
    highlight.setOutlineThickness(3.0f);

    window.draw(highlight);
}

void BoardView::drawGrid(sf::RenderWindow &window)
{
    // Draw background
    sf::RectangleShape background(sf::Vector2f(BOARD_SIZE * CELL_SIZE + GRID_PADDING * 2, BOARD_SIZE * CELL_SIZE + GRID_PADDING * 2));
    background.setPosition(position);
    background.setFillColor(Colors::OceanDark);
    window.draw(background);

    // Draw grid lines
    for (int i = 0; i <= BOARD_SIZE; ++i)
    {
        // Vertical lines
        sf::RectangleShape vLine(sf::Vector2f(1, BOARD_SIZE * CELL_SIZE));
        vLine.setPosition(position.x + GRID_PADDING + i * CELL_SIZE, position.y + GRID_PADDING);
        vLine.setFillColor(Colors::Ocean);
        window.draw(vLine);

        // Horizontal lines
        sf::RectangleShape hLine(sf::Vector2f(BOARD_SIZE * CELL_SIZE, 1));
        hLine.setPosition(position.x + GRID_PADDING, position.y + GRID_PADDING + i * CELL_SIZE);
        hLine.setFillColor(Colors::Ocean);
        window.draw(hLine);
    }
}

void BoardView::drawCell(sf::RenderWindow &window, const Board &board, int row, int col, char symbol)
{
    float x = position.x + GRID_PADDING + col * CELL_SIZE;
    float y = position.y + GRID_PADDING + row * CELL_SIZE;

    sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));
    cell.setPosition(x + 1, y + 1);

    sf::Color cellColor = Colors::Ocean;
    sf::CircleShape marker;
    bool drawMarker = false;

    const bool sunkCell = (symbol == 'X') && board.isShipSunkAt(Coordinate{row, col});

    switch (symbol)
    {
    case 'S': // Ship
        cellColor = Colors::Ship;
        cell.setOutlineColor(Colors::ShipDark);
        cell.setOutlineThickness(1.0f);
        break;
    case 'X': // Hit
        cellColor = Colors::Ocean;
        marker.setRadius(CELL_SIZE / 4.0f);
        marker.setFillColor(Colors::Hit);
        marker.setPosition(x + CELL_SIZE / 2.0f - marker.getRadius(), y + CELL_SIZE / 2.0f - marker.getRadius());
        drawMarker = true;
        break;
    case 'O': // Miss
        cellColor = Colors::Ocean;
        marker.setRadius(CELL_SIZE / 6.0f);
        marker.setFillColor(Colors::Miss);
        marker.setPosition(x + CELL_SIZE / 2.0f - marker.getRadius(), y + CELL_SIZE / 2.0f - marker.getRadius());
        drawMarker = true;
        break;
    default: // Water
        cellColor = Colors::Ocean;
        break;
    }

    cell.setFillColor(cellColor);
    window.draw(cell);

    if (drawMarker)
    {
        window.draw(marker);
    }

    if (sunkCell)
    {
        sf::RectangleShape sunkOverlay(sf::Vector2f(CELL_SIZE - 10, CELL_SIZE - 10));
        sunkOverlay.setPosition(x + 5, y + 5);
        sunkOverlay.setFillColor(sf::Color(255, 215, 0, 90));
        sunkOverlay.setOutlineColor(sf::Color(255, 215, 0, 160));
        sunkOverlay.setOutlineThickness(2.0f);
        window.draw(sunkOverlay);
    }
}

void BoardView::drawLabel(sf::RenderWindow &window, const std::string &text, const sf::Vector2f &pos, sf::Font &font)
{
    sf::Text label;
    label.setFont(font);
    label.setString(text);
    label.setCharacterSize(18);
    label.setFillColor(Colors::Text);

    sf::FloatRect bounds = label.getLocalBounds();
    label.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    label.setPosition(pos);

    window.draw(label);
}

char BoardView::getCellSymbol(const Board &board, int row, int col) const
{
    return board.getCellSymbol(Coordinate{row, col}, showShips);
}

// ============================================================================
// MessageBox Implementation
// ============================================================================

MessageBox::MessageBox(const sf::Vector2f &position, const sf::Vector2f &size, sf::Font &font)
    : size(size), font(font)
{
    background.setPosition(position);
    background.setSize(size);
    background.setFillColor(sf::Color(0, 0, 0, 200));
    background.setOutlineColor(Colors::Text);
    background.setOutlineThickness(2.0f);

    messageText.setFont(font);
    messageText.setCharacterSize(22);
    messageText.setFillColor(Colors::Text);
    messageText.setPosition(position.x + 15, position.y + 15);
}

void MessageBox::setMessage(const std::string &message)
{
    messages.clear();
    messages.push_back(message);
    updateDisplay();
}

void MessageBox::addMessage(const std::string &message)
{
    messages.push_back(message);
    // Keep only last 10 messages
    if (messages.size() > 10)
    {
        messages.erase(messages.begin());
    }
    updateDisplay();
}

void MessageBox::draw(sf::RenderWindow &window)
{
    window.draw(background);
    window.draw(messageText);
}

void MessageBox::clear()
{
    messages.clear();
    updateDisplay();
}

void MessageBox::updateDisplay()
{
    std::string combined;
    for (const auto &msg : messages)
    {
        if (!combined.empty())
        {
            combined += "\n";
        }
        combined += msg;
    }
    messageText.setString(combined);
}

// ============================================================================
// GameGUI Implementation
// ============================================================================

GameGUI::GameGUI()
{
    initWindow();
    initFont();
    initMenuBackground();
    initShipTextures();
    initShipPortraits();
    initAudio();
    initWaterBackground();
    initGameObjects();
    loadStats();
    
    // Initialize fade overlay
    fadeOverlay.setSize(sf::Vector2f(1920, 1080));
    fadeOverlay.setFillColor(sf::Color(0, 0, 0, 0));
    
    changeState(GameState::Menu);
}

GameGUI::~GameGUI() = default;

void GameGUI::initWindow()
{
    window.create(sf::VideoMode(1920, 1080), "Fleet Commander - Pixel Art Edition", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
}

void GameGUI::initFont()
{
    // Try to load a pixel-style font from assets first
    bool fontLoaded = false;
    
    // Try assets folder
    if (font.loadFromFile("assets/fonts/arial.ttf"))
    {
        fontLoaded = true;
        std::cout << "Loaded font from assets/fonts/arial.ttf" << std::endl;
    }
    // Try Windows system fonts
    else if (font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf"))
    {
        fontLoaded = true;
        std::cout << "Loaded font from Windows\\Fonts\\arial.ttf" << std::endl;
    }
    else if (font.loadFromFile("C:\\Windows\\Fonts\\calibri.ttf"))
    {
        fontLoaded = true;
        std::cout << "Loaded font from Windows\\Fonts\\calibri.ttf" << std::endl;
    }
    else if (font.loadFromFile("C:\\Windows\\Fonts\\verdana.ttf"))
    {
        fontLoaded = true;
        std::cout << "Loaded font from Windows\\Fonts\\verdana.ttf" << std::endl;
    }
    
    if (!fontLoaded)
    {
        std::cerr << "WARNING: Could not load any font file. Text may not display correctly." << std::endl;
        std::cerr << "The game will continue, but you may want to check your font installation." << std::endl;
    }
}

void GameGUI::initMenuBackground()
{
    // Try to load menu background image - try multiple formats
    bool loaded = false;
    
    // Try PNG first
    if (menuBackgroundTexture.loadFromFile("assets/textures/menu_background.png"))
    {
        loaded = true;
        std::cout << "Loaded menu background from assets/textures/menu_background.png" << std::endl;
    }
    // Try JPG/JPEG as fallback
    else if (menuBackgroundTexture.loadFromFile("assets/textures/menu_background.jpg"))
    {
        loaded = true;
        std::cout << "Loaded menu background from assets/textures/menu_background.jpg" << std::endl;
    }
    else if (menuBackgroundTexture.loadFromFile("assets/textures/menu_background.jpeg"))
    {
        loaded = true;
        std::cout << "Loaded menu background from assets/textures/menu_background.jpeg" << std::endl;
    }
    // Try BMP as fallback
    else if (menuBackgroundTexture.loadFromFile("assets/textures/menu_background.bmp"))
    {
        loaded = true;
        std::cout << "Loaded menu background from assets/textures/menu_background.bmp" << std::endl;
    }
    
    if (loaded)
    {
        hasMenuBackground = true;
        menuBackgroundSprite.setTexture(menuBackgroundTexture);
        
        // Scale to fit 1920x1080 window
        sf::Vector2u textureSize = menuBackgroundTexture.getSize();
        float scaleX = 1920.0f / textureSize.x;
        float scaleY = 1080.0f / textureSize.y;
        menuBackgroundSprite.setScale(scaleX, scaleY);
    }
    else
    {
        hasMenuBackground = false;
        std::cout << "No menu background found (optional). Using default background." << std::endl;
        std::cout << "Tip: Save your image as a standard PNG (8-bit RGBA) or JPG format." << std::endl;
    }
}

void GameGUI::initShipTextures()
{
    // Try to load ship textures
    // Expected file names (you can name your PNG files like this):
    struct ShipTextureInfo
    {
        std::string key;
        std::vector<std::string> candidates;
    };

    const std::vector<ShipTextureInfo> shipFiles = {
        {"AircraftCarrier", {
                                   "assets/textures/aircraft_carrier.png",
                                   "assets/textures/Aircraft_Carrier.png",
                                   "assets/textures/aircraft_carrier.jpg",
                                   "assets/textures/Aircraft_Carrier.jpg",
                                   "assets/textures/AircraftCarrier.png",
                                   "assets/textures/AircraftCarrier.jpg"}},
        {"Battleship", {
                             "assets/textures/battleship.png",
                             "assets/textures/Battleship.png",
                             "assets/textures/battleship.jpg",
                             "assets/textures/Battleship.jpg"}},
        {"Cruiser", {
                          "assets/textures/cruiser.png",
                          "assets/textures/Cruiser.png",
                          "assets/textures/cruiser.jpg",
                          "assets/textures/Cruiser.jpg"}},
        {"Submarine", {
                            "assets/textures/submarine.png",
                            "assets/textures/Submarine.png",
                            "assets/textures/submarine.jpg",
                            "assets/textures/Submarine.jpg"}},
        {"Destroyer", {
                            "assets/textures/destroyer.png",
                            "assets/textures/Destroyer.png",
                            "assets/textures/destroyer.jpg",
                            "assets/textures/Destroyer.jpg"}}
    };

    int loadedCount = 0;
    for (const auto &info : shipFiles)
    {
        sf::Texture texture;
        std::string loadedPath;
        bool loaded = false;

        for (const auto &candidate : info.candidates)
        {
            if (texture.loadFromFile(candidate))
            {
                loadedPath = candidate;
                loaded = true;
                break;
            }
        }

        if (loaded)
        {
            texture.setSmooth(false); // Keep pixel art crisp
            shipTextures[info.key] = std::move(texture);
            ++loadedCount;
            std::cout << "Loaded texture: " << loadedPath << std::endl;
        }
        else
        {
            std::cerr << "Warning: Could not find texture for " << info.key
                      << ". Checked " << info.candidates.size() << " path(s)." << std::endl;
        }
    }

    if (loadedCount == static_cast<int>(shipFiles.size()))
    {
        useShipSprites = true;
        std::cout << "Ship sprites enabled!" << std::endl;
    }
    else
    {
        useShipSprites = false;
        std::cout << "Using default colored rectangles for ships." << std::endl;
    }
}

void GameGUI::initShipPortraits()
{
    shipPortraits.clear();
    shipPortraits.reserve(5);

    struct PortraitInfo
    {
        const char *name;
        const char *file;
    };

    const std::array<PortraitInfo, 5> portraitFiles = {
        PortraitInfo{"Aircraft Carrier", "assets/textures/Aircraft_Carrier.jpg"},
        PortraitInfo{"Battleship", "assets/textures/Battleship.jpg"},
        PortraitInfo{"Cruiser", "assets/textures/Cruiser.jpg"},
        PortraitInfo{"Submarine", "assets/textures/Submarine.jpg"},
        PortraitInfo{"Destroyer", "assets/textures/Destroyer.jpg"}
    };

    const float portraitWidth = 180.0f;
    const float portraitHeight = 100.0f;

    for (std::size_t index = 0; index < portraitFiles.size(); ++index)
    {
        shipPortraits.emplace_back();
        ShipPortrait &portrait = shipPortraits.back();
        portrait.name = portraitFiles[index].name;

        if (portrait.texture.loadFromFile(portraitFiles[index].file))
        {
            portrait.loaded = true;
            portrait.sprite.setTexture(portrait.texture);
            portrait.sprite.setScale(1.0f, 1.0f);

            sf::Vector2u size = portrait.texture.getSize();
            if (size.x > 0 && size.y > 0)
            {
                float scaleX = portraitWidth / static_cast<float>(size.x);
                float scaleY = portraitHeight / static_cast<float>(size.y);
                float scale = std::min(scaleX, scaleY);
                portrait.sprite.setScale(scale, scale);
            }

            const sf::FloatRect bounds = portrait.sprite.getLocalBounds();
            portrait.sprite.setOrigin(bounds.left + bounds.width / 2.0f,
                                      bounds.top + bounds.height / 2.0f);
        }
        else
        {
            std::cerr << "Warning: Could not load portrait file " << portraitFiles[index].file << std::endl;
            portrait.loaded = false;
        }
    }
}

void GameGUI::initAudio()
{
    // Load menu music
    menuMusicLoaded = menuMusic.openFromFile("assets/audio/menu.ogg") ||
                      menuMusic.openFromFile("assets/audio/menu.mp3") ||
                      menuMusic.openFromFile("assets/audio/menu.wav");
    if (menuMusicLoaded)
    {
        menuMusic.setLoop(true);
        menuMusic.setVolume(musicVolume);
        std::cout << "Menu music loaded." << std::endl;
    }
    else
    {
        std::cout << "No menu music found (optional)." << std::endl;
    }

    // Load battle music
    battleMusicLoaded = battleMusic.openFromFile("assets/audio/battle.ogg") ||
                        battleMusic.openFromFile("assets/audio/battle.mp3") ||
                        battleMusic.openFromFile("assets/audio/battle.wav");
    if (battleMusicLoaded)
    {
        battleMusic.setLoop(true);
        battleMusic.setVolume(musicVolume);
        std::cout << "Battle music loaded." << std::endl;
    }
    else
    {
        std::cout << "No battle music found (optional)." << std::endl;
    }

    // Load victory music
    victoryMusicLoaded = victoryMusic.openFromFile("assets/audio/victory.ogg") ||
                         victoryMusic.openFromFile("assets/audio/victory.mp3") ||
                         victoryMusic.openFromFile("assets/audio/victory.wav");
    if (victoryMusicLoaded)
    {
        victoryMusic.setLoop(false);
        victoryMusic.setVolume(musicVolume);
        std::cout << "Victory music loaded." << std::endl;
    }
    else
    {
        std::cout << "No victory music found (optional)." << std::endl;
    }

    // Load defeat music
    defeatMusicLoaded = defeatMusic.openFromFile("assets/audio/defeat.ogg") ||
                        defeatMusic.openFromFile("assets/audio/defeat.mp3") ||
                        defeatMusic.openFromFile("assets/audio/defeat.wav");
    if (defeatMusicLoaded)
    {
        defeatMusic.setLoop(false);
        defeatMusic.setVolume(musicVolume);
        std::cout << "Defeat music loaded." << std::endl;
    }
    else
    {
        std::cout << "No defeat music found (optional)." << std::endl;
    }

    // Load sound effects
    if (hitSoundBuffer.loadFromFile("assets/audio/hit.wav") ||
        hitSoundBuffer.loadFromFile("assets/audio/hit.ogg"))
    {
        hitSound.setBuffer(hitSoundBuffer);
        hitSound.setVolume(sfxVolume);
        std::cout << "Hit sound effect loaded." << std::endl;
    }
    
    if (missSoundBuffer.loadFromFile("assets/audio/miss.wav") ||
        missSoundBuffer.loadFromFile("assets/audio/miss.ogg"))
    {
        missSound.setBuffer(missSoundBuffer);
        missSound.setVolume(sfxVolume);
        std::cout << "Miss sound effect loaded." << std::endl;
    }
    
    if (sinkSoundBuffer.loadFromFile("assets/audio/sink.wav") ||
        sinkSoundBuffer.loadFromFile("assets/audio/sink.ogg"))
    {
        sinkSound.setBuffer(sinkSoundBuffer);
        sinkSound.setVolume(sfxVolume);
        std::cout << "Sink sound effect loaded." << std::endl;
    }
}

void GameGUI::initWaterBackground()
{
    // Try to load animated water texture
    if (waterTexture.loadFromFile("assets/textures/water.png") ||
        waterTexture.loadFromFile("assets/textures/water.jpg"))
    {
        hasWaterTexture = true;
        waterTexture.setRepeated(true);
        waterSprite.setTexture(waterTexture);
        waterSprite.setTextureRect(sf::IntRect(0, 0, 1920, 1080));
        waterSprite.setColor(sf::Color(255, 255, 255, 100)); // Semi-transparent
        std::cout << "Water background texture loaded." << std::endl;
    }
    else
    {
        hasWaterTexture = false;
    }
}

void GameGUI::loadStats()
{
    std::ifstream file("stats.txt");
    if (file.is_open())
    {
        file >> stats.gamesPlayed >> stats.gamesWon >> stats.gamesLost 
             >> stats.totalShotsFired >> stats.totalHits;
        file.close();
        std::cout << "Loaded statistics: " << stats.gamesPlayed << " games played." << std::endl;
    }
    else
    {
        std::cout << "No previous statistics found. Starting fresh!" << std::endl;
    }
}

void GameGUI::saveStats()
{
    std::ofstream file("stats.txt");
    if (file.is_open())
    {
        file << stats.gamesPlayed << " " << stats.gamesWon << " " << stats.gamesLost << " "
             << stats.totalShotsFired << " " << stats.totalHits;
        file.close();
    }
}

void GameGUI::updateStatsOnGameEnd(bool won)
{
    stats.gamesPlayed++;
    if (won)
        stats.gamesWon++;
    else
        stats.gamesLost++;
    
    stats.totalShotsFired += currentGameShots;
    stats.totalHits += currentGameHits;
    
    saveStats();
}

void GameGUI::initGameObjects()
{
    // Initialize boards
    playerBoard = std::make_unique<Board>();
    computerBoard = std::make_unique<Board>();

    // Create fleets
    createFleet(playerFleet);
    createFleet(computerFleet);

    // Initialize board views (centered for 1920x1080)
    playerBoardView = std::make_unique<BoardView>(sf::Vector2f(150, 200), true);
    computerBoardView = std::make_unique<BoardView>(sf::Vector2f(1050, 200), false);
    
    // Enable sprites if textures were loaded
    if (useShipSprites)
    {
        playerBoardView->setUseSprites(true);
        computerBoardView->setUseSprites(true);
    }

    // Initialize message box
    messageBox = std::make_unique<MessageBox>(sf::Vector2f(150, 900), sf::Vector2f(1380, 120), font);
    
    // Initialize computer shots
    refillComputerShots();
}

void GameGUI::refillComputerShots()
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
    
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(computerShots.begin(), computerShots.end(), g);
}

void GameGUI::createFleet(std::vector<std::unique_ptr<Ship>> &fleet)
{
    fleet.clear();
    fleet.emplace_back(std::make_unique<AircraftCarrier>());
    fleet.emplace_back(std::make_unique<Battleship>());
    fleet.emplace_back(std::make_unique<Cruiser>());
    fleet.emplace_back(std::make_unique<Submarine>());
    fleet.emplace_back(std::make_unique<Destroyer>());
}

void GameGUI::run()
{
    while (window.isOpen())
    {
        float deltaTime = deltaClock.restart().asSeconds();

        processEvents();
        update(deltaTime);
        render();
    }
}

void GameGUI::processEvents()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
        }

        switch (state)
        {
        case GameState::Menu:
            handleMenuEvents(event);
            break;
        case GameState::Settings:
            handleSettingsEvents(event);
            break;
        case GameState::PlacingShips:
            handlePlacementEvents(event);
            break;
        case GameState::PlayerTurn:
        case GameState::ComputerTurn:
            handleBattleEvents(event);
            break;
        case GameState::GameOver:
            handleGameOverEvents(event);
            break;
        }
    }
}

void GameGUI::handleMenuEvents(sf::Event &event)
{
    if (event.type == sf::Event::MouseButtonPressed)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        for (size_t i = 0; i < buttons.size(); ++i)
        {
            if (buttons[i]->isClicked(mousePos, event.mouseButton))
            {
                if (i == 0) // Start New Game
                {
                    currentGameShots = 0;
                    currentGameHits = 0;
                    changeState(GameState::PlacingShips);
                }
                else if (i == 1) // Settings
                {
                    changeState(GameState::Settings);
                }
                else if (i >= 2 && i <= 4) // Difficulty
                {
                    difficulty = static_cast<Difficulty>(i - 2);
                }
                break;
            }
        }
    }
}

void GameGUI::handleSettingsEvents(sf::Event &event)
{
    static bool draggingMusic = false;
    static bool draggingSFX = false;

    auto applyMusicVolume = [this]() {
        if (menuMusicLoaded)
        {
            menuMusic.setVolume(musicVolume);
        }
        if (battleMusicLoaded)
        {
            battleMusic.setVolume(musicVolume);
        }
        if (victoryMusicLoaded)
        {
            victoryMusic.setVolume(musicVolume);
        }
        if (defeatMusicLoaded)
        {
            defeatMusic.setVolume(musicVolume);
        }
    };
    
    if (event.type == sf::Event::MouseButtonPressed)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        
        // Check if clicking on sliders
        sf::FloatRect musicSlider(660, 390, 600, 20);
        sf::FloatRect sfxSlider(660, 540, 600, 20);
        
        if (musicSlider.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
        {
            draggingMusic = true;
            float percent = (mousePos.x - 660) / 600.0f;
            musicVolume = std::max(0.0f, std::min(100.0f, percent * 100.0f));
            applyMusicVolume();
        }
        else if (sfxSlider.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
        {
            draggingSFX = true;
            float percent = (mousePos.x - 660) / 600.0f;
            sfxVolume = std::max(0.0f, std::min(100.0f, percent * 100.0f));
            hitSound.setVolume(sfxVolume);
            missSound.setVolume(sfxVolume);
            sinkSound.setVolume(sfxVolume);
        }
        
        // Check back button
        for (auto &button : buttons)
        {
            if (button->isClicked(mousePos, event.mouseButton))
            {
                changeState(GameState::Menu);
                break;
            }
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased)
    {
        draggingMusic = false;
        draggingSFX = false;
    }
    else if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        
        if (draggingMusic)
        {
            float percent = (mousePos.x - 660) / 600.0f;
            musicVolume = std::max(0.0f, std::min(100.0f, percent * 100.0f));
            applyMusicVolume();
        }
        else if (draggingSFX)
        {
            float percent = (mousePos.x - 660) / 600.0f;
            sfxVolume = std::max(0.0f, std::min(100.0f, percent * 100.0f));
            hitSound.setVolume(sfxVolume);
            missSound.setVolume(sfxVolume);
            sinkSound.setVolume(sfxVolume);
        }
    }
}

void GameGUI::handlePlacementEvents(sf::Event &event)
{
    if (event.type == sf::Event::MouseMoved)
    {
        Coordinate coord;
        if (playerBoardView->getCellFromMouse(sf::Mouse::getPosition(window), coord))
        {
            placementState.previewStart = coord;
            
            // Check if placement would be valid
            if (placementState.currentShipIndex < static_cast<int>(playerFleet.size()))
            {
                Ship *ship = playerFleet[placementState.currentShipIndex].get();
                
                // Create a temporary copy of the board to test placement
                bool valid = true;
                for (int i = 0; i < ship->getSize(); ++i)
                {
                    int row = coord.first + (placementState.horizontal ? 0 : i);
                    int col = coord.second + (placementState.horizontal ? i : 0);
                    Coordinate testCoord{row, col};
                    
                    if (row < 0 || row >= Board::SIZE || col < 0 || col >= Board::SIZE ||
                        playerBoard->isOccupied(testCoord))
                    {
                        valid = false;
                        break;
                    }
                }
                placementState.previewValid = valid;
            }
        }
    }

    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::R)
        {
            placementState.horizontal = !placementState.horizontal;
        }
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        Coordinate coord;
        if (playerBoardView->getCellFromMouse(sf::Mouse::getPosition(window), coord))
        {
            placeCurrentShip(coord, placementState.horizontal);
        }
    }
}

void GameGUI::handleBattleEvents(sf::Event &event)
{
    if (state != GameState::PlayerTurn)
        return;

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        Coordinate target;
        if (computerBoardView->getCellFromMouse(sf::Mouse::getPosition(window), target))
        {
            playerAttack(target);
        }
    }
}

void GameGUI::handleGameOverEvents(sf::Event &event)
{
    if (event.type == sf::Event::MouseButtonPressed)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        for (size_t i = 0; i < buttons.size(); ++i)
        {
            if (buttons[i]->isClicked(mousePos, event.mouseButton))
            {
                if (i == 0) // Play Again
                {
                    currentGameShots = 0;
                    currentGameHits = 0;
                    changeState(GameState::PlacingShips);
                }
                else if (i == 1) // Main Menu
                {
                    changeState(GameState::Menu);
                }
                break;
            }
        }
    }
}

void GameGUI::update(float deltaTime)
{
    updateParticles(deltaTime);
    
    // Animate water background
    if (hasWaterTexture)
    {
        waterScrollOffset += 20.0f * deltaTime; // Scroll speed
    }
    
    // Fade effects
    if (fadeAlpha > 0)
    {
        fadeAlpha -= 100.0f * deltaTime;
        if (fadeAlpha < 0) fadeAlpha = 0;
    }

    if (state == GameState::ComputerTurn)
    {
        updateComputerTurn();
    }
}

void GameGUI::updateParticles(float deltaTime)
{
    for (auto it = particles.begin(); it != particles.end();)
    {
        it->lifetime -= deltaTime;
        if (it->lifetime <= 0)
        {
            it = particles.erase(it);
        }
        else
        {
            // Apply gravity to confetti
            if (it->type == ParticleType::Confetti)
            {
                it->velocity.y += 300.0f * deltaTime; // Gravity
            }
            
            it->shape.move(it->velocity * deltaTime);
            
            // Fade out
            sf::Color color = it->startColor;
            color.a = static_cast<sf::Uint8>(255 * (it->lifetime / it->maxLifetime));
            it->shape.setFillColor(color);
            
            ++it;
        }
    }
}

void GameGUI::updateComputerTurn()
{
    if (!waitingForAction)
    {
        waitingForAction = true;
        actionClock.restart();
    }

    if (actionClock.getElapsedTime().asSeconds() >= actionDelay)
    {
        waitingForAction = false;
        bool keepTurn = executeComputerAttack();
        if (!keepTurn && state != GameState::GameOver)
        {
            changeState(GameState::PlayerTurn);
        }
    }
}

void GameGUI::render()
{
    window.clear(Colors::Background);

    // Draw animated water background for battle scenes
    if (state == GameState::PlayerTurn || state == GameState::ComputerTurn || state == GameState::PlacingShips)
    {
        renderWaterBackground();
    }

    switch (state)
    {
    case GameState::Menu:
        renderMenu();
        break;
    case GameState::Settings:
        renderSettings();
        break;
    case GameState::PlacingShips:
        renderPlacement();
        break;
    case GameState::PlayerTurn:
    case GameState::ComputerTurn:
        renderBattle();
        break;
    case GameState::GameOver:
        renderGameOver();
        break;
    }

    // Draw particles
    for (const auto &particle : particles)
    {
        window.draw(particle.shape);
    }
    
    // Draw fade overlay (for transitions)
    if (fadeAlpha > 0)
    {
        fadeOverlay.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(fadeAlpha)));
        window.draw(fadeOverlay);
    }

    window.display();
}

void GameGUI::renderWaterBackground()
{
    if (hasWaterTexture)
    {
        // Scroll the water texture
        int offsetX = static_cast<int>(waterScrollOffset) % waterTexture.getSize().x;
        waterSprite.setTextureRect(sf::IntRect(offsetX, 0, 1920, 1080));
        window.draw(waterSprite);
    }
}

void GameGUI::renderMenu()
{
    // Draw background image if available
    if (hasMenuBackground)
    {
        window.draw(menuBackgroundSprite);
    }
    
    drawTitle("FLEET COMMANDER", 150);
    drawCenteredText("Pixel Art Edition", 280, 28);

    const float difficultyButtonY = 720.0f;
    const float difficultyButtonBaseX = 660.0f;
    const float difficultyButtonSpacing = 210.0f;
    const sf::Vector2f difficultyButtonSize(200.0f, 60.0f);
    
    // Draw buttons
    if (buttons.empty())
    {
        buttons.push_back(std::make_unique<Button>(sf::Vector2f(760, 400), sf::Vector2f(400, 80), "Start New Game", font));
        buttons.back()->setBackgroundVisible(false);
        buttons.back()->setHoverTextColor(Colors::Highlight);

        buttons.push_back(std::make_unique<Button>(sf::Vector2f(760, 500), sf::Vector2f(400, 80), "Settings", font));
        buttons.back()->setBackgroundVisible(false);
        buttons.back()->setHoverTextColor(Colors::Highlight);
        
        // Difficulty buttons
        buttons.push_back(std::make_unique<Button>(sf::Vector2f(difficultyButtonBaseX, difficultyButtonY), difficultyButtonSize, "Easy", font));
        buttons.back()->setBackgroundVisible(false);
        buttons.back()->setHoverTextColor(Colors::Highlight);
        buttons.push_back(std::make_unique<Button>(sf::Vector2f(difficultyButtonBaseX + difficultyButtonSpacing, difficultyButtonY), difficultyButtonSize, "Medium", font));
        buttons.back()->setBackgroundVisible(false);
        buttons.back()->setHoverTextColor(Colors::Highlight);
        buttons.push_back(std::make_unique<Button>(sf::Vector2f(difficultyButtonBaseX + 2 * difficultyButtonSpacing, difficultyButtonY), difficultyButtonSize, "Hard", font));
        buttons.back()->setBackgroundVisible(false);
        buttons.back()->setHoverTextColor(Colors::Highlight);
    }

    for (size_t i = 0; i < buttons.size(); ++i)
    {
        // Highlight selected difficulty
        if (i >= 2 && i <= 4)
        {
            int diffIdx = static_cast<int>(difficulty);
            if (i == diffIdx + 2)
            {
                // Draw selection indicator
                sf::RectangleShape selector(difficultyButtonSize);
                bool hovered = buttons[i]->isHovered(sf::Mouse::getPosition(window));
                float selectorX = difficultyButtonBaseX + (static_cast<float>(i - 2) * difficultyButtonSpacing) + (hovered ? -5.0f : 0.0f);
                float selectorY = difficultyButtonY + (hovered ? -5.0f : 0.0f);
                selector.setPosition(selectorX, selectorY);
                selector.setFillColor(sf::Color::Transparent);
                selector.setOutlineColor(Colors::Highlight);
                selector.setOutlineThickness(4.0f);
                window.draw(selector);
            }
        }
        buttons[i]->draw(window);
    }

    drawCenteredText("Select Difficulty:", 660, 22);
    drawCenteredText("Sink all enemy ships to win!", 820, 24);
    
    // Display stats
    std::stringstream ss;
    ss << "Stats: " << stats.gamesPlayed << " Games | " 
       << stats.gamesWon << " Wins | "
       << std::fixed << std::setprecision(1) << stats.getAccuracy() << "% Accuracy";
    drawCenteredText(ss.str(), 950, 20);
}

void GameGUI::renderSettings()
{
    drawTitle("SETTINGS", 150);
    
    // Music Volume
    drawCenteredText("Music Volume: " + std::to_string(static_cast<int>(musicVolume)) + "%", 350, 24);
    sf::RectangleShape musicSlider(sf::Vector2f(600, 20));
    musicSlider.setPosition(660, 390);
    musicSlider.setFillColor(Colors::OceanDark);
    musicSlider.setOutlineColor(Colors::Text);
    musicSlider.setOutlineThickness(2);
    window.draw(musicSlider);
    
    sf::RectangleShape musicFill(sf::Vector2f(600 * (musicVolume / 100.0f), 20));
    musicFill.setPosition(660, 390);
    musicFill.setFillColor(Colors::Highlight);
    window.draw(musicFill);
    
    // SFX Volume
    drawCenteredText("SFX Volume: " + std::to_string(static_cast<int>(sfxVolume)) + "%", 500, 24);
    sf::RectangleShape sfxSlider(sf::Vector2f(600, 20));
    sfxSlider.setPosition(660, 540);
    sfxSlider.setFillColor(Colors::OceanDark);
    sfxSlider.setOutlineColor(Colors::Text);
    sfxSlider.setOutlineThickness(2);
    window.draw(sfxSlider);
    
    sf::RectangleShape sfxFill(sf::Vector2f(600 * (sfxVolume / 100.0f), 20));
    sfxFill.setPosition(660, 540);
    sfxFill.setFillColor(Colors::Highlight);
    window.draw(sfxFill);
    
    // Draw back button
    if (buttons.empty())
    {
        buttons.push_back(std::make_unique<Button>(sf::Vector2f(760, 700), sf::Vector2f(400, 80), "Back to Menu", font));
    }
    
    for (auto &button : buttons)
    {
        button->draw(window);
    }
    
    drawCenteredText("Click and drag sliders to adjust volume", 650, 20);
}

void GameGUI::renderPlacement()
{
    drawTitle("DEPLOY YOUR FLEET", 80);

    // Draw instruction message
    if (placementState.currentShipIndex < static_cast<int>(playerFleet.size()))
    {
        Ship *currentShip = playerFleet[placementState.currentShipIndex].get();
        std::string instruction = "Placing " + currentShip->getType() + " (size " + 
                                  std::to_string(currentShip->getSize()) + "). Press R to rotate.";
        drawCenteredText(instruction, 140, 24);
    }

    // Draw player board
    if (playerBoard)
    {
        if (useShipSprites)
        {
            playerBoardView->draw(window, *playerBoard, font, shipTextures);
        }
        else
        {
            playerBoardView->draw(window, *playerBoard, font);
        }
        
        // Draw placement preview
        if (placementState.currentShipIndex < static_cast<int>(playerFleet.size()))
        {
            Ship *currentShip = playerFleet[placementState.currentShipIndex].get();
            playerBoardView->drawPlacementPreview(window, placementState.previewStart, 
                currentShip->getSize(), placementState.horizontal, placementState.previewValid);
        }
    }

    renderShipStatusPanel();

    messageBox->draw(window);
}

void GameGUI::renderShipStatusPanel()
{
    if (shipPortraits.empty())
    {
        return;
    }

    const float panelX = 1680.0f;
    const float panelY = 200.0f;
    const float panelWidth = 220.0f;
    const float panelHeight = 720.0f;
    const float panelCenterX = panelX + panelWidth / 2.0f;
    const float entryStart = panelY + 90.0f;
    const float entrySpacing = 155.0f;
    const float labelOffset = 14.0f;
    const float placeholderWidth = panelWidth - 50.0f;
    const float placeholderHeight = 100.0f;

    sf::RectangleShape panel(sf::Vector2f(panelWidth, panelHeight));
    panel.setPosition(panelX, panelY);
    panel.setFillColor(sf::Color(0, 0, 0, 160));
    panel.setOutlineColor(Colors::Text);
    panel.setOutlineThickness(2.0f);
    window.draw(panel);

    sf::Text heading("Enemy Fleet", font, 24);
    heading.setFillColor(Colors::Text);
    sf::FloatRect headingBounds = heading.getLocalBounds();
    heading.setOrigin(headingBounds.left + headingBounds.width / 2.0f,
                      headingBounds.top + headingBounds.height / 2.0f);
    heading.setPosition(panelX + panelWidth / 2.0f, panelY + 30.0f);
    window.draw(heading);

    for (std::size_t index = 0; index < shipPortraits.size(); ++index)
    {
        ShipPortrait &portrait = shipPortraits[index];
        float entryCenterY = entryStart + entrySpacing * static_cast<float>(index);

        bool sunk = isComputerShipSunk(portrait.name);
        sf::Color spriteColor = sunk ? sf::Color(160, 160, 160, 140) : sf::Color::White;

        float spriteHalfHeight = placeholderHeight / 2.0f;

        if (portrait.loaded)
        {
            portrait.sprite.setPosition(panelCenterX, entryCenterY - 10.0f);
            portrait.sprite.setColor(spriteColor);
            window.draw(portrait.sprite);
            spriteHalfHeight = portrait.sprite.getGlobalBounds().height / 2.0f;
        }
        else
        {
            sf::RectangleShape placeholder(sf::Vector2f(placeholderWidth, placeholderHeight));
            placeholder.setOrigin(placeholder.getSize().x / 2.0f, placeholder.getSize().y / 2.0f);
            placeholder.setPosition(panelCenterX, entryCenterY - 10.0f);
            placeholder.setFillColor(sf::Color(80, 80, 80, 120));
            placeholder.setOutlineColor(sf::Color(120, 120, 120, 180));
            placeholder.setOutlineThickness(2.0f);
            window.draw(placeholder);
            spriteHalfHeight = placeholder.getSize().y / 2.0f;
        }

        unsigned int labelSize = portrait.name.size() > 12 ? 16u : 18u;
        sf::Text label(portrait.name, font, labelSize);
        label.setFillColor(sunk ? sf::Color(200, 200, 200, 190) : Colors::Text);
        sf::FloatRect labelBounds = label.getLocalBounds();
        label.setOrigin(labelBounds.left + labelBounds.width / 2.0f,
                        labelBounds.top + labelBounds.height / 2.0f);
        label.setPosition(panelCenterX, entryCenterY + spriteHalfHeight + labelOffset);
        window.draw(label);

        if (sunk)
        {
            sf::Text sunkText("SUNK", font, 14);
            sunkText.setFillColor(Colors::Hit);
            sf::FloatRect sunkBounds = sunkText.getLocalBounds();
            sunkText.setOrigin(sunkBounds.left + sunkBounds.width / 2.0f,
                               sunkBounds.top + sunkBounds.height / 2.0f);
            sunkText.setPosition(panelCenterX, entryCenterY + spriteHalfHeight + labelOffset + 24.0f);
            window.draw(sunkText);
        }
    }
}

bool GameGUI::isComputerShipSunk(const std::string &shipName) const
{
    for (const auto &ship : computerFleet)
    {
        if (ship && ship->getName() == shipName)
        {
            return ship->isSunk();
        }
    }
    return false;
}

void GameGUI::renderBattle()
{
    drawTitle("FLEET COMMANDER", 50);

    // Draw board labels
    sf::Text playerLabel("Your Fleet", font, 28);
    playerLabel.setFillColor(Colors::Text);
    playerLabel.setPosition(320, 160);
    window.draw(playerLabel);

    sf::Text enemyLabel("Enemy Waters", font, 28);
    enemyLabel.setFillColor(Colors::Text);
    enemyLabel.setPosition(1200, 160);
    window.draw(enemyLabel);

    // Draw boards
    if (playerBoard && computerBoard)
    {
        if (useShipSprites)
        {
            playerBoardView->draw(window, *playerBoard, font, shipTextures);
            computerBoardView->draw(window, *computerBoard, font, shipTextures);
        }
        else
        {
            playerBoardView->draw(window, *playerBoard, font);
            computerBoardView->draw(window, *computerBoard, font);
        }
    }

    // Highlight hovered cell during player turn
    if (state == GameState::PlayerTurn)
    {
        Coordinate hoverCoord;
        if (computerBoardView->getCellFromMouse(sf::Mouse::getPosition(window), hoverCoord))
        {
            computerBoardView->highlightCell(window, hoverCoord, Colors::Highlight);
        }
    }

    // Turn indicator
    std::string turnText = (state == GameState::PlayerTurn) ? "YOUR TURN" : "ENEMY TURN";
    sf::Color turnColor = (state == GameState::PlayerTurn) ? Colors::Highlight : Colors::Hit;
    
    sf::Text turnIndicator(turnText, font, 32);
    turnIndicator.setFillColor(turnColor);
    sf::FloatRect bounds = turnIndicator.getLocalBounds();
    turnIndicator.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    turnIndicator.setPosition(960, 120);
    window.draw(turnIndicator);
    
    // Display accuracy
    float accuracy = currentGameShots > 0 ? (100.0f * currentGameHits / currentGameShots) : 0.0f;
    std::stringstream ss;
    ss << "Shots: " << currentGameShots << " | Hits: " << currentGameHits 
       << " | Accuracy: " << std::fixed << std::setprecision(1) << accuracy << "%";
    drawCenteredText(ss.str(), 850, 20);
    
    // Show ship tooltip on hover (player board only for now)
    if (playerBoard)
    {
        Coordinate hoverCoord;
        if (playerBoardView->getCellFromMouse(sf::Mouse::getPosition(window), hoverCoord))
        {
            const auto& ships = playerBoard->getShips();
            for (const Ship* ship : ships)
            {
                if (ship && ship->isPlaced())
                {
                    const auto& positions = ship->getPositions();
                    for (const auto& pos : positions)
                    {
                        if (pos.first == hoverCoord.first && pos.second == hoverCoord.second)
                        {
                            // Draw tooltip
                            sf::Vector2f tooltipPos = playerBoardView->getCellCenter(hoverCoord);
                            tooltipPos.y -= 40;
                            
                            sf::Text tooltip(ship->getType(), font, 18);
                            tooltip.setFillColor(Colors::Text);
                            tooltip.setStyle(sf::Text::Bold);
                            sf::FloatRect tBounds = tooltip.getLocalBounds();
                            tooltip.setOrigin(tBounds.left + tBounds.width / 2.0f, tBounds.top + tBounds.height / 2.0f);
                            tooltip.setPosition(tooltipPos);
                            
                            // Background for tooltip
                            sf::RectangleShape tooltipBg(sf::Vector2f(tBounds.width + 20, tBounds.height + 10));
                            tooltipBg.setPosition(tooltipPos.x - tBounds.width/2 - 10, tooltipPos.y - tBounds.height/2 - 5);
                            tooltipBg.setFillColor(sf::Color(0, 0, 0, 200));
                            tooltipBg.setOutlineColor(Colors::Highlight);
                            tooltipBg.setOutlineThickness(2);
                            
                            window.draw(tooltipBg);
                            window.draw(tooltip);
                            break;
                        }
                    }
                }
            }
        }
    }

    renderShipStatusPanel();

    messageBox->draw(window);
}

void GameGUI::renderGameOver()
{
    drawTitle("GAME OVER", 150);

    std::string resultText = playerWon ? "VICTORY!" : "DEFEAT";
    sf::Color resultColor = playerWon ? sf::Color::Green : Colors::Hit;

    sf::Text result(resultText, font, 64);
    result.setFillColor(resultColor);
    result.setStyle(sf::Text::Bold);
    sf::FloatRect bounds = result.getLocalBounds();
    result.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    result.setPosition(960, 300);
    window.draw(result);
    
    // Game stats
    float gameAccuracy = currentGameShots > 0 ? (100.0f * currentGameHits / currentGameShots) : 0.0f;
    
    std::stringstream ss;
    ss << "This Game: " << currentGameShots << " shots, " << currentGameHits << " hits"
       << " (" << std::fixed << std::setprecision(1) << gameAccuracy << "%)";
    drawCenteredText(ss.str(), 400, 24);
    
    ss.str("");
    ss << "Overall Stats: " << stats.gamesPlayed << " games, "
       << stats.gamesWon << " wins, " << stats.gamesLost << " losses";
    drawCenteredText(ss.str(), 450, 24);
    
    ss.str("");
    ss << "Career Accuracy: " << std::fixed << std::setprecision(1) << stats.getAccuracy() << "%";
    drawCenteredText(ss.str(), 500, 24);
    
    // Show difficulty
    std::string diffStr = (difficulty == Difficulty::Easy) ? "Easy" :
                         (difficulty == Difficulty::Medium) ? "Medium" : "Hard";
    drawCenteredText("Difficulty: " + diffStr, 550, 24);

    // Draw buttons
    if (buttons.empty())
    {
        buttons.push_back(std::make_unique<Button>(sf::Vector2f(660, 650), sf::Vector2f(300, 70), "Play Again", font));
        buttons.push_back(std::make_unique<Button>(sf::Vector2f(980, 650), sf::Vector2f(300, 70), "Main Menu", font));
    }

    for (auto &button : buttons)
    {
        button->draw(window);
    }

    renderShipStatusPanel();
}

void GameGUI::changeState(GameState newState)
{
    GameState oldState = state;
    state = newState;
    buttons.clear();

    auto musicForState = [this](GameState stateValue) -> sf::Music * {
        switch (stateValue)
        {
        case GameState::Menu:
        case GameState::Settings:
            return menuMusicLoaded ? &menuMusic : nullptr;
        case GameState::PlacingShips:
        case GameState::PlayerTurn:
        case GameState::ComputerTurn:
            return battleMusicLoaded ? &battleMusic : nullptr;
        case GameState::GameOver:
            if (playerWon)
            {
                return victoryMusicLoaded ? &victoryMusic : nullptr;
            }
            return defeatMusicLoaded ? &defeatMusic : nullptr;
        }
        return nullptr;
    };

    sf::Music *oldMusic = musicInitialized ? musicForState(oldState) : nullptr;
    sf::Music *newMusic = musicForState(newState);

    if (!musicInitialized || oldMusic != newMusic)
    {
        if (menuMusicLoaded)
        {
            menuMusic.stop();
        }
        if (battleMusicLoaded)
        {
            battleMusic.stop();
        }
        if (victoryMusicLoaded)
        {
            victoryMusic.stop();
        }
        if (defeatMusicLoaded)
        {
            defeatMusic.stop();
        }

        if (newMusic && newMusic->getStatus() != sf::Music::Playing)
        {
            newMusic->play();
        }

        musicInitialized = true;
    }

    switch (newState)
    {
    case GameState::Menu:
        // Reset game
        initGameObjects();
        messageBox->clear();
        particles.clear();
        break;

    case GameState::Settings:
        // Retain current music
        break;

    case GameState::PlacingShips:
        placementState = PlacementState();
        messageBox->setMessage("Click on the board to place your ships. Press R to rotate.");
        break;

    case GameState::PlayerTurn:
        messageBox->addMessage("Your turn - click on enemy waters to attack!");
        break;

    case GameState::ComputerTurn:
        messageBox->addMessage("Enemy is attacking...");
        waitingForAction = false;
        break;

    case GameState::GameOver:
        break;
    }
}

void GameGUI::placeCurrentShip(const Coordinate &start, bool horizontal)
{
    if (playerBoard && placementState.currentShipIndex < static_cast<int>(playerFleet.size()))
    {
        Ship *ship = playerFleet[placementState.currentShipIndex].get();
        if (playerBoard->placeShip(*ship, start, horizontal))
        {
            messageBox->addMessage(ship->getName() + " placed successfully!");
            placementState.currentShipIndex++;
            
            if (placementState.currentShipIndex >= static_cast<int>(playerFleet.size()))
            {
                finishPlacement();
            }
        }
        else
        {
            messageBox->addMessage("Invalid placement - try again!");
        }
    }
}

void GameGUI::finishPlacement()
{
    setupComputerFleet();
    messageBox->addMessage("All ships deployed! Battle begins!");
    changeState(GameState::PlayerTurn);
}

void GameGUI::setupComputerFleet()
{
    if (!loadComputerPlacements())
    {
        generateComputerPlacements();
        saveComputerPlacements();
    }
}

void GameGUI::generateComputerPlacements()
{
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> orientationDist(0, 1);
    
    while (true)
    {
        computerBoard->clear();
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

                if (computerBoard->placeShip(*ship, start, horizontal))
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

bool GameGUI::loadComputerPlacements()
{
    std::ifstream input(placementFile);
    if (!input)
    {
        return false;
    }

    computerBoard->clear();
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

        if (!computerBoard->placeShip(*targetShip, start, horizontal))
        {
            return false;
        }

        unused.erase(std::remove(unused.begin(), unused.end(), targetShip), unused.end());
        ++placed;
    }

    return placed == static_cast<int>(computerFleet.size());
}

void GameGUI::saveComputerPlacements() const
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

void GameGUI::playerAttack(const Coordinate &target)
{
    std::string shipName;
    Board::AttackResult result = computerBoard->attack(target, shipName);

    bool keepTurn = false;

    switch (result)
    {
    case Board::AttackResult::Miss:
        createMissEffect(computerBoardView->getCellCenter(target));
        messageBox->addMessage("Miss at " + coordinateToString(target));
        missSound.play();
        currentGameShots++;
        break;
    case Board::AttackResult::Hit:
        createHitEffect(computerBoardView->getCellCenter(target));
        messageBox->addMessage("Hit at " + coordinateToString(target) + "!");
        hitSound.play();
        currentGameShots++;
        currentGameHits++;
        keepTurn = true;
        break;
    case Board::AttackResult::Sunk:
        createSinkEffect(computerBoardView->getCellCenter(target));
        messageBox->addMessage("Sunk the " + shipName + "!");
        sinkSound.play();
        currentGameShots++;
        currentGameHits++;
        keepTurn = true;
        break;
    case Board::AttackResult::AlreadyTried:
        messageBox->addMessage("Already tried " + coordinateToString(target));
        return;
    case Board::AttackResult::Invalid:
        return;
    }

    checkGameOver();
    if (state != GameState::GameOver)
    {
        if (keepTurn)
        {
            messageBox->addMessage("Take another shot!");
        }
        else
        {
            changeState(GameState::ComputerTurn);
        }
    }
}

bool GameGUI::executeComputerAttack()
{
    Coordinate target;
    bool validTarget = false;
    
    // Smart AI for Medium and Hard difficulty
    if (difficulty != Difficulty::Easy && !hitQueue.empty())
    {
        target = hitQueue.back();
        hitQueue.pop_back();
        validTarget = true;
    }
    else if (difficulty == Difficulty::Hard && huntingMode && lastHit.first != -1)
    {
        // Hard mode: Try adjacent cells to last hit
        std::vector<Coordinate> adjacent = {
            {lastHit.first - 1, lastHit.second},
            {lastHit.first + 1, lastHit.second},
            {lastHit.first, lastHit.second - 1},
            {lastHit.first, lastHit.second + 1}
        };
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(adjacent.begin(), adjacent.end(), gen);
        
        for (const auto& coord : adjacent)
        {
            if (coord.first >= 0 && coord.first < 10 && coord.second >= 0 && coord.second < 10)
            {
                if (!playerBoard->isAttacked(coord))
                {
                    target = coord;
                    validTarget = true;
                    break;
                }
            }
        }
    }
    
    // Random shot if no smart target
    if (!validTarget)
    {
        while (!computerShots.empty())
        {
            target = computerShots.back();
            computerShots.pop_back();
            
            if (!playerBoard->isAttacked(target))
            {
                validTarget = true;
                break;
            }
        }
    }
    
    if (!validTarget)
    {
        refillComputerShots();
        return true;
    }

    std::string shipName;
    Board::AttackResult result = playerBoard->attack(target, shipName);

    if (result == Board::AttackResult::Invalid || result == Board::AttackResult::AlreadyTried)
    {
        return true;
    }

    bool keepTurn = false;

    switch (result)
    {
    case Board::AttackResult::Miss:
        createMissEffect(playerBoardView->getCellCenter(target));
        messageBox->addMessage("Enemy misses at " + coordinateToString(target));
        missSound.play();
        huntingMode = false;
        keepTurn = false;
        break;
    case Board::AttackResult::Hit:
        createHitEffect(playerBoardView->getCellCenter(target));
        messageBox->addMessage("Enemy hits at " + coordinateToString(target) + "!");
        hitSound.play();
        keepTurn = true;
        
        // Add adjacent cells to hit queue for smart targeting
        if (difficulty != Difficulty::Easy)
        {
            lastHit = target;
            huntingMode = true;
            
            std::vector<Coordinate> adjacent = {
                {target.first - 1, target.second},
                {target.first + 1, target.second},
                {target.first, target.second - 1},
                {target.first, target.second + 1}
            };
            
            for (const auto& coord : adjacent)
            {
                if (coord.first >= 0 && coord.first < 10 && coord.second >= 0 && coord.second < 10)
                {
                    if (!playerBoard->isAttacked(coord))
                    {
                        hitQueue.push_back(coord);
                    }
                }
            }
        }
        break;
    case Board::AttackResult::Sunk:
        createSinkEffect(playerBoardView->getCellCenter(target));
        messageBox->addMessage("Enemy sinks your " + shipName + "!");
        sinkSound.play();
        hitQueue.clear();
        huntingMode = false;
        lastHit = {-1, -1};
        keepTurn = true;
        break;
    default:
        break;
    }

    checkGameOver();
    if (state == GameState::GameOver)
    {
        return false;
    }

    if (computerShots.empty())
    {
        refillComputerShots();
    }

    return keepTurn;
}

void GameGUI::checkGameOver()
{
    if (playerBoard->allShipsSunk())
    {
        playerWon = false;
        updateStatsOnGameEnd(false);
        changeState(GameState::GameOver);
    }
    else if (computerBoard->allShipsSunk())
    {
        playerWon = true;
        updateStatsOnGameEnd(true);
        
        // Create confetti for victory
        for (int i = 0; i < 100; ++i)
        {
            createConfetti(sf::Vector2f(960, 540));
        }
        
        changeState(GameState::GameOver);
    }
}

void GameGUI::createHitEffect(const sf::Vector2f &position)
{
    createParticle(position, ParticleType::Explosion);
}

void GameGUI::createMissEffect(const sf::Vector2f &position)
{
    createParticle(position, ParticleType::WaterSplash);
}

void GameGUI::createConfetti(const sf::Vector2f &position)
{
    createParticle(position, ParticleType::Confetti);
}

void GameGUI::createParticle(const sf::Vector2f &position, ParticleType type)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0, 6.28318f);
    
    int count = 10;
    float speedMin = 30, speedMax = 80;
    float radiusMin = 2, radiusMax = 4;
    float lifetime = 0.8f;
    sf::Color color = Colors::Miss;
    
    switch (type)
    {
    case ParticleType::Explosion:
        count = 30;
        speedMin = 50;
        speedMax = 180;
        radiusMin = 3;
        radiusMax = 6;
        lifetime = 1.2f;
        color = Colors::Hit;
        break;
    case ParticleType::WaterSplash:
        count = 15;
        speedMin = 30;
        speedMax = 90;
        radiusMin = 2;
        radiusMax = 3;
        lifetime = 0.7f;
        color = Colors::Miss;
        break;
    case ParticleType::Confetti:
        count = 3;
        speedMin = 100;
        speedMax = 250;
        radiusMin = 4;
        radiusMax = 8;
        lifetime = 2.5f;
        color = sf::Color::Yellow;
        break;
    }
    
    std::uniform_real_distribution<float> speedDist(speedMin, speedMax);
    std::uniform_real_distribution<float> radiusDist(radiusMin, radiusMax);
    
    for (int i = 0; i < count; ++i)
    {
        Particle p;
        p.shape.setRadius(radiusDist(gen));
        p.type = type;
        p.startColor = color;
        
        // Vary colors
        if (type == ParticleType::Explosion)
        {
            p.startColor = (i % 3 == 0) ? sf::Color::Yellow : 
                          (i % 3 == 1) ? sf::Color(255, 100, 0) : Colors::Hit;
        }
        else if (type == ParticleType::Confetti)
        {
            sf::Color confettiColors[] = {
                sf::Color::Red, sf::Color::Yellow, sf::Color::Green,
                sf::Color::Blue, sf::Color::Magenta, sf::Color::Cyan
            };
            p.startColor = confettiColors[i % 6];
        }
        
        p.shape.setFillColor(p.startColor);
        p.shape.setPosition(position);

        float angle = angleDist(gen);
        float speed = speedDist(gen);
        p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
        
        // Confetti falls down
        if (type == ParticleType::Confetti)
        {
            p.velocity.y = -std::abs(p.velocity.y); // Start going up
        }
        
        p.lifetime = lifetime;
        p.maxLifetime = lifetime;

        particles.push_back(p);
    }
}

void GameGUI::createSinkEffect(const sf::Vector2f &position)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0, 6.28318f);
    std::uniform_real_distribution<float> speedDist(80, 200);

    for (int i = 0; i < 40; ++i)
    {
        Particle p;
        p.shape.setRadius(4);
        p.shape.setFillColor(i % 2 == 0 ? Colors::Hit : sf::Color::Yellow);
        p.shape.setPosition(position);

        float angle = angleDist(gen);
        float speed = speedDist(gen);
        p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
        p.lifetime = 1.5f;
        p.maxLifetime = 1.5f;

        particles.push_back(p);
    }
}

void GameGUI::drawTitle(const std::string &text, float yPosition)
{
    sf::Text title(text, font, 64);
    title.setFillColor(Colors::Text);
    title.setStyle(sf::Text::Bold);

    sf::FloatRect bounds = title.getLocalBounds();
    title.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    title.setPosition(960, yPosition);

    window.draw(title);
}

void GameGUI::drawCenteredText(const std::string &text, float yPosition, unsigned int size)
{
    sf::Text textObj(text, font, size);
    textObj.setFillColor(Colors::Text);

    sf::FloatRect bounds = textObj.getLocalBounds();
    textObj.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    textObj.setPosition(960, yPosition);

    window.draw(textObj);
}

std::string GameGUI::coordinateToString(const Coordinate &coord)
{
    char letter = static_cast<char>('A' + coord.first);
    return std::string(1, letter) + std::to_string(coord.second + 1);
}

std::string GameGUI::sanitizeShipName(const std::string &name)
{
    return sanitizeShipKey(name);
}

// ============================================================================
// Entry Point
// ============================================================================
int main()
{
    try
    {
        GameGUI game;
        game.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
