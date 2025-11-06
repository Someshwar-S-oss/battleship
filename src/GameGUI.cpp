#include "GameGUI.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>

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
    label.setFillColor(Colors::Text);

    // Center text in button
    sf::FloatRect textBounds = label.getLocalBounds();
    label.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    label.setPosition(position.x + size.x / 2.0f, position.y + size.y / 2.0f);
}

void Button::draw(sf::RenderWindow &window)
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    hovered = isHovered(mousePos);

    if (hovered)
    {
        shape.setFillColor(Colors::ButtonHover);
    }
    else
    {
        shape.setFillColor(Colors::ButtonNormal);
    }

    window.draw(shape);
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
            drawCell(window, row, col, symbol);
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
                drawCell(window, row, col, symbol);
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
                    drawCell(window, row, col, symbol);
                }
            }
        }
    }
}

void BoardView::drawShipSprite(sf::RenderWindow &window, const Board &board, const Ship &ship, const std::map<std::string, sf::Texture> &shipTextures)
{
    auto it = shipTextures.find(ship.getName());
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

void BoardView::drawCell(sf::RenderWindow &window, int row, int col, char symbol)
{
    float x = position.x + GRID_PADDING + col * CELL_SIZE;
    float y = position.y + GRID_PADDING + row * CELL_SIZE;

    sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));
    cell.setPosition(x + 1, y + 1);

    sf::Color cellColor = Colors::Ocean;
    sf::CircleShape marker;
    bool drawMarker = false;

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
    std::vector<std::pair<std::string, std::string>> shipFiles = {
        {"AircraftCarrier", "assets/textures/aircraft_carrier.png"},
        {"Battleship", "assets/textures/battleship.png"},
        {"Cruiser", "assets/textures/cruiser.png"},
        {"Submarine", "assets/textures/submarine.png"},
        {"Destroyer", "assets/textures/destroyer.png"}
    };

    bool allLoaded = true;
    for (const auto &[shipName, filePath] : shipFiles)
    {
        sf::Texture texture;
        if (texture.loadFromFile(filePath))
        {
            texture.setSmooth(false); // Keep pixel art crisp
            shipTextures[shipName] = texture;
            std::cout << "Loaded texture: " << filePath << std::endl;
        }
        else
        {
            std::cerr << "Warning: Could not load " << filePath << std::endl;
            allLoaded = false;
        }
    }

    if (allLoaded)
    {
        useShipSprites = true;
        std::cout << "Ship sprites enabled!" << std::endl;
    }
    else
    {
        std::cout << "Using default colored rectangles for ships." << std::endl;
        useShipSprites = false;
    }
}

void GameGUI::initAudio()
{
    // Load menu music
    if (menuMusic.openFromFile("assets/audio/menu.ogg") || 
        menuMusic.openFromFile("assets/audio/menu.mp3") ||
        menuMusic.openFromFile("assets/audio/menu.wav"))
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
    if (battleMusic.openFromFile("assets/audio/battle.ogg") || 
        battleMusic.openFromFile("assets/audio/battle.mp3") ||
        battleMusic.openFromFile("assets/audio/battle.wav"))
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
    if (victoryMusic.openFromFile("assets/audio/victory.ogg") || 
        victoryMusic.openFromFile("assets/audio/victory.mp3") ||
        victoryMusic.openFromFile("assets/audio/victory.wav"))
    {
        victoryMusic.setLoop(false); // Don't loop victory music
        victoryMusic.setVolume(musicVolume);
        std::cout << "Victory music loaded." << std::endl;
    }
    else
    {
        std::cout << "No victory music found (optional)." << std::endl;
    }
    
    // Load defeat music
    if (defeatMusic.openFromFile("assets/audio/defeat.ogg") || 
        defeatMusic.openFromFile("assets/audio/defeat.mp3") ||
        defeatMusic.openFromFile("assets/audio/defeat.wav"))
    {
        defeatMusic.setLoop(false); // Don't loop defeat music
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
    messageBox = std::make_unique<MessageBox>(sf::Vector2f(150, 900), sf::Vector2f(1620, 120), font);
    
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
            menuMusic.setVolume(musicVolume);
            battleMusic.setVolume(musicVolume);
            victoryMusic.setVolume(musicVolume);
            defeatMusic.setVolume(musicVolume);
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
            menuMusic.setVolume(musicVolume);
            battleMusic.setVolume(musicVolume);
            victoryMusic.setVolume(musicVolume);
            defeatMusic.setVolume(musicVolume);
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
        executeComputerAttack();
        waitingForAction = false;
        changeState(GameState::PlayerTurn);
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
    
    // Draw buttons
    if (buttons.empty())
    {
        buttons.push_back(std::make_unique<Button>(sf::Vector2f(760, 400), sf::Vector2f(400, 80), "Start New Game", font));
        buttons.push_back(std::make_unique<Button>(sf::Vector2f(760, 500), sf::Vector2f(400, 80), "Settings", font));
        
        // Difficulty buttons (moved down to avoid overlap with Settings button)
        buttons.push_back(std::make_unique<Button>(sf::Vector2f(660, 680), sf::Vector2f(200, 60), "Easy", font));
        buttons.push_back(std::make_unique<Button>(sf::Vector2f(870, 680), sf::Vector2f(200, 60), "Medium", font));
        buttons.push_back(std::make_unique<Button>(sf::Vector2f(1080, 680), sf::Vector2f(200, 60), "Hard", font));
    }

    for (size_t i = 0; i < buttons.size(); ++i)
    {
        // Highlight selected difficulty
        if (i >= 2 && i <= 4)
        {
            int diffIdx = static_cast<int>(difficulty);
            if (i == static_cast<size_t>(diffIdx + 2))
            {
                // Draw selection indicator
                sf::RectangleShape selector(sf::Vector2f(200, 60));
                selector.setPosition(buttons[i]->isHovered(sf::Mouse::getPosition(window)) ? 
                    sf::Vector2f(660 + (i-2)*210 - 5, 675) : sf::Vector2f(660 + (i-2)*210, 680));
                selector.setFillColor(sf::Color::Transparent);
                selector.setOutlineColor(Colors::Highlight);
                selector.setOutlineThickness(4.0f);
                window.draw(selector);
            }
        }
        buttons[i]->draw(window);
    }

    drawCenteredText("Select Difficulty:", 640, 22);
    drawCenteredText("Sink all enemy ships to win!", 840, 24);
    
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

    messageBox->draw(window);
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
}

void GameGUI::changeState(GameState newState)
{
    GameState oldState = state;
    state = newState;
    buttons.clear();

    // Determine which music should be playing for each state
    sf::Music* oldMusic = nullptr;
    sf::Music* newMusic = nullptr;
    
    // Map old state to its music
    switch (oldState)
    {
    case GameState::Menu:
    case GameState::Settings:
        oldMusic = &menuMusic;
        break;
    case GameState::PlacingShips:
    case GameState::PlayerTurn:
    case GameState::ComputerTurn:
        oldMusic = &battleMusic;
        break;
    case GameState::GameOver:
        oldMusic = playerWon ? &victoryMusic : &defeatMusic;
        break;
    }
    
    // Map new state to its music
    switch (newState)
    {
    case GameState::Menu:
    case GameState::Settings:
        newMusic = &menuMusic;
        break;
    case GameState::PlacingShips:
    case GameState::PlayerTurn:
    case GameState::ComputerTurn:
        newMusic = &battleMusic;
        break;
    case GameState::GameOver:
        newMusic = playerWon ? &victoryMusic : &defeatMusic;
        break;
    }
    
    // Only change music if switching to a different track
    if (oldMusic != newMusic)
    {
        // Stop all music
        menuMusic.stop();
        battleMusic.stop();
        victoryMusic.stop();
        defeatMusic.stop();
        
        // Start the new music
        if (newMusic && newMusic->getStatus() != sf::Music::Playing)
        {
            newMusic->play();
        }
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
        // Keep current music playing
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
        // Music already started above
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
        break;
    case Board::AttackResult::Sunk:
        createSinkEffect(computerBoardView->getCellCenter(target));
        messageBox->addMessage("Sunk the " + shipName + "!");
        sinkSound.play();
        currentGameShots++;
        currentGameHits++;
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
        changeState(GameState::ComputerTurn);
    }
}

void GameGUI::executeComputerAttack()
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
        return;
    }

    std::string shipName;
    Board::AttackResult result = playerBoard->attack(target, shipName);

    if (result == Board::AttackResult::Invalid || result == Board::AttackResult::AlreadyTried)
    {
        return;
    }

    switch (result)
    {
    case Board::AttackResult::Miss:
        createMissEffect(playerBoardView->getCellCenter(target));
        messageBox->addMessage("Enemy misses at " + coordinateToString(target));
        missSound.play();
        huntingMode = false;
        break;
    case Board::AttackResult::Hit:
        createHitEffect(playerBoardView->getCellCenter(target));
        messageBox->addMessage("Enemy hits at " + coordinateToString(target) + "!");
        hitSound.play();
        
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
        break;
    default:
        break;
    }

    checkGameOver();
    
    if (computerShots.empty())
    {
        refillComputerShots();
    }
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
