#pragma once

#include "GameLogic.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <array>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>

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

private:
    sf::RectangleShape shape;
    sf::Text label;
    sf::Vector2f size;
    bool hovered = false;
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
    void drawCell(sf::RenderWindow &window, int row, int col, char symbol);
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
    
    // Game state
    GameState state;
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
    void executeComputerAttack();
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
    
    // Utility
    static std::string coordinateToString(const Coordinate &coord);
    static std::string sanitizeShipName(const std::string &name);
};
