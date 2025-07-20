#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <queue>
#include <iostream>

// divide them by the divisors of 120 to mantain aspect ratio:
// 1 2 3 4 5 6 8 10 12 15 20 24 30 40 60 120

constexpr int DIVISOR = 24;

constexpr int WIDTH = 1920 / DIVISOR;  // Use fixed values or your screen resolution
constexpr int HEIGHT = 1080 / DIVISOR; // Use fixed values or your screen resolution


// here are the tile value meanings:
constexpr int emptyTile = -3; 
constexpr int undefinedTile = -1; // undefined tile (not used in this example, but can be useful for future extensions)
constexpr int wallTile = -2; 
constexpr int startTile = 0; 
constexpr int endTile = -4; 
constexpr int pathTile = -5; // marked pathfinding route (for Lee's algorithm)
// every code above 0 is for distance from the start tile in Lee's algorithm
// the start tile should always be 0

// for the borders
int matrix[WIDTH][HEIGHT]; 

sf::Vector2i start = {1, 1};
sf::Vector2i end = {WIDTH - 2, HEIGHT - 2}; // Start and end points for Lee's algorithm

bool M_Toggled = false; // to toggle the Lee's algorithm distance display on and off

void emptyMatrix() {

    for (int i = 1; i < WIDTH - 1; i++) {
        for (int j = 1; j < HEIGHT - 1; j++) {
            matrix[i][j] = emptyTile; // Initialize all cells to empty
        }
    }

    matrix[start.x][start.y] = startTile;
    matrix[end.x][end.y] = endTile;
}

void AddBorders()
{
    for (int i = 0; i < WIDTH; i++)
    {
        matrix[i][0] = wallTile; // Top border
        matrix[i][HEIGHT - 1] = wallTile; // Bottom border
    }
    for (int j = 0; j < HEIGHT; j++)
    {
        matrix[0][j] = wallTile; // Left border
        matrix[WIDTH - 1][j] = wallTile; // Right border
    }
}

void DrawMatrix(sf::RenderWindow& window) {

    static sf::RectangleShape rect(sf::Vector2f(DIVISOR, DIVISOR)); 
    rect.setFillColor(sf::Color(56, 56, 56)); // gray
    rect.setOutlineColor(sf::Color::Black);

    static sf::RectangleShape specialRect(sf::Vector2f(DIVISOR, DIVISOR)); 

    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            if (matrix[i][j] == wallTile) 
            {
                rect.setPosition(sf::Vector2f(i * DIVISOR, j * DIVISOR));
                window.draw(rect);
            }
            else if (matrix[i][j] == startTile) { 
                specialRect.setFillColor(sf::Color::Green);
                specialRect.setPosition(sf::Vector2f(i * DIVISOR, j * DIVISOR));
                window.draw(specialRect);
            }
            else if (matrix[i][j] == endTile) {
                specialRect.setFillColor(sf::Color::Red);
                specialRect.setPosition(sf::Vector2f(i * DIVISOR, j * DIVISOR));
                window.draw(specialRect);
            }
            else if (matrix[i][j] > 0 && M_Toggled) { // If it's a distance from the start tile in Lee's algorithm
                specialRect.setFillColor(sf::Color(0, 0, matrix[i][j] * 10 % 256)); // Color based on distance
                specialRect.setPosition(sf::Vector2f(i * DIVISOR, j * DIVISOR));
                window.draw(specialRect);
            }
            else if (matrix[i][j] == emptyTile || (matrix[i][j] > 0 && !M_Toggled)) {  
                specialRect.setFillColor(sf::Color::White); // Empty tile color
                specialRect.setOutlineThickness(1); 
                specialRect.setOutlineColor(sf::Color::Black);
                specialRect.setPosition(sf::Vector2f(i * DIVISOR, j * DIVISOR));
                window.draw(specialRect);

                specialRect.setOutlineThickness(0); 
            }
            else if (matrix[i][j] == pathTile) { // If it's a path tile in Lee's algorithm
                specialRect.setFillColor(sf::Color::Magenta); // Color based on distance
                specialRect.setPosition(sf::Vector2f(i * DIVISOR, j * DIVISOR));
                window.draw(specialRect);
            }
        }
    }
}

// takes the tile index to toggle 
// (wall <-> empty)
void ToggleTile(int x, int y, bool Wall)  {

    // must be bigger than 1 to avoid clicking on the borders
    if (x >= 1 && x < WIDTH - 1 && y >= 1 && y < HEIGHT - 1)
    {   
        
        if (matrix[x][y] == startTile || matrix[x][y] == endTile) {
            return; // Do not toggle special tiles
        }

        // Toggle the cell state
        if (Wall)
            matrix[x][y] = wallTile; 
        else 
            matrix[x][y] = emptyTile; 
    }
}

void HandleTileToggle(sf::RenderWindow& window) {
    
    static bool lShiftPressed = false;
    static bool Old_leftMouseButtonPressed = false;
    static bool NEW_leftMouseButtonPressed = false;
    static sf::Vector2i lastTilePoz = sf::Mouse::getPosition(window) / DIVISOR;
    static sf::Vector2i currentTilePoz = sf::Mouse::getPosition(window) / DIVISOR;

    lShiftPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift);

    NEW_leftMouseButtonPressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

    currentTilePoz = sf::Mouse::getPosition(window) / DIVISOR;

    // for checking if the left mouse button is released and is in toggle mode
    if (Old_leftMouseButtonPressed && !NEW_leftMouseButtonPressed) {
        
        ToggleTile(currentTilePoz.x, currentTilePoz.y, !lShiftPressed); // Toggle the tile at the mouse position
    }

    if (NEW_leftMouseButtonPressed && Old_leftMouseButtonPressed && (currentTilePoz != lastTilePoz)) {

        ToggleTile(currentTilePoz.x, currentTilePoz.y, !lShiftPressed);
    }

    Old_leftMouseButtonPressed = NEW_leftMouseButtonPressed;
    lastTilePoz = currentTilePoz;
}

void GameOfLife() {

    static sf::Clock clock; 

    static bool simulationRunning = false;
    static bool Old_spacePressed = false;
    static bool NEW_spacePressed = false;

    NEW_spacePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

    if (!NEW_spacePressed && Old_spacePressed) {
        simulationRunning = !simulationRunning; // Toggle simulation state
    }

    if (clock.getElapsedTime().asMilliseconds() < 1000) {
        return; // Prevent too frequent updates
    }

    if (!simulationRunning) {

        Old_spacePressed = NEW_spacePressed;
        return; // If simulation is not running, do nothing
    }

    for (int i = 1; i < WIDTH - 1; i++) {
        for (int j = 1; j < HEIGHT - 1; j++) {
            int aliveNeighbours = 0;

            // Check all 8 neighbours
            for (int x = -1; x <= 1; x++) {
                for (int y = -1; y <= 1; y++) {
                    if (x == 0 && y == 0) continue; // Skip the cell itself
                    if (matrix[i + x][j + y] == 1 && (i + x < WIDTH - 1) && (j + y < HEIGHT - 1) && (i + x > 0) && (j + y > 0)) aliveNeighbours++;
                }
            }

            // Toggle the cell based on the number of alive neighbours
            if (aliveNeighbours < 2 || aliveNeighbours > 3) {
                matrix[i][j] = emptyTile; // Cell dies
            } else if (aliveNeighbours == 3) {
                matrix[i][j] = wallTile; // Cell becomes alive
            }
        }
    }

    clock.restart(); 
    Old_spacePressed = NEW_spacePressed;
}

// used for moving the start and finish points in Lee's algorithm
int HandleSpecialTileMoving (sf::RenderWindow& window) {

    static bool old_rightMouseButtonPressed = false;
    static bool new_rightMouseButtonPressed = false; 

    static sf::Vector2i newTilePoz = {1, 1};
    static sf::Vector2i oldTilePoz = {1, 1};
    static sf::Vector2i oldAvailableTilePoz = {1, 1};

    static int selectedSpecialTile = undefinedTile;

    new_rightMouseButtonPressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
    newTilePoz = sf::Mouse::getPosition(window) / DIVISOR;

    if (new_rightMouseButtonPressed && !old_rightMouseButtonPressed) { // to select a special tile
        
        if ((matrix[newTilePoz.x][newTilePoz.y] == startTile || matrix[newTilePoz.x][newTilePoz.y] == endTile) && selectedSpecialTile == undefinedTile) {
            
            selectedSpecialTile = matrix[newTilePoz.x][newTilePoz.y];
        }
        else {
            selectedSpecialTile = undefinedTile;
        }
    }

    if (newTilePoz != oldTilePoz && (matrix[newTilePoz.x][newTilePoz.y] > 0 || matrix[newTilePoz.x][newTilePoz.y] == emptyTile) && selectedSpecialTile != -1) { // to move the selected special tile
        
        if (matrix[oldAvailableTilePoz.x][oldAvailableTilePoz.y] == selectedSpecialTile) // to only delete the special tile itself
            matrix[oldAvailableTilePoz.x][oldAvailableTilePoz.y] = emptyTile;

        matrix[newTilePoz.x][newTilePoz.y] = selectedSpecialTile;
        
        if (selectedSpecialTile == startTile) {
            start = newTilePoz; // Update the start position
        } else if (selectedSpecialTile == endTile) {
            end = newTilePoz; // Update the end position
        }
    }

    old_rightMouseButtonPressed = new_rightMouseButtonPressed;
    oldTilePoz = newTilePoz;

    if (matrix[newTilePoz.x][newTilePoz.y] == emptyTile || matrix[newTilePoz.x][newTilePoz.y] == selectedSpecialTile) 
        oldAvailableTilePoz = newTilePoz; 

    return selectedSpecialTile; 
}

void LeesAlgorithm() {

    static const sf::Vector2i directions[4] = {
        {1, 0},   // Right
        {-1, 0},  // Left
        {0, 1},   // Down
        {0, -1}   // Up
    };

    static bool M_OldPressed = false;
    static bool M_NewPressed = false;

    M_NewPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::M);
    if (M_NewPressed && !M_OldPressed) {
        M_Toggled = !M_Toggled; // Toggle the algorithm
    }
    M_OldPressed = M_NewPressed;

    std::queue<sf::Vector2i> queue;

    queue.push(start); 

    sf::Vector2i current; 

    bool foundEnd = false; 

    // apply the algorithm
    while (!queue.empty() && !foundEnd) {

        current = queue.front();
        queue.pop();

        for (const auto& dir : directions) {
            sf::Vector2i next = current + dir;


            if (matrix[next.x][next.y] == emptyTile) {
                matrix[next.x][next.y] = matrix[current.x][current.y] + 1; // Mark the path with the distance from the start
                queue.push(next); 
                continue;
            }

            if (matrix[next.x][next.y] == endTile) {
                foundEnd = true; 
            }
        }
    }

    if (queue.empty()) {
        return; // No path found
    }

    // the "current" has the steps until the end - 1
    if (matrix[current.x][current.y] == emptyTile || matrix[current.x][current.y] == startTile) {
        return; // No path found or the last tile is not valid
    }

    // backtrack and mark the shortest path
    while (current != start) {
    
        // check all neighbours to find the previous tile in the path
        for (const auto& dir : directions) {
            sf::Vector2i next = current + dir;

            if (matrix[next.x][next.y] == matrix[current.x][current.y] - 1) {
                matrix[current.x][current.y] = pathTile; // Mark the tile
                current = next; // backtrack to the next tile
                break;
            }
        }
    }
}

// turns all tiles into empty tiles / wall tiles / start & end tiles
void SimplifyMatrix() {

    for (int i = 1; i < WIDTH - 1; i++) {
        for (int j = 1; j < HEIGHT - 1; j++) {
            if (matrix[i][j] > 0 || matrix[i][j] == pathTile) 
                matrix[i][j] = emptyTile; 
        }
    }
}

void HandleRemoveWallsAction() {

    static bool Old_BackspacePressed = false;
    static bool New_BackspacePressed = false;

    New_BackspacePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Backspace);
    
    if (New_BackspacePressed && !Old_BackspacePressed) {
        
        for (int i = 1; i < WIDTH - 1; i++) {
            for (int j = 1; j < HEIGHT - 1; j++) {
                if (matrix[i][j] == wallTile) 
                    matrix[i][j] = emptyTile; 
            }
        }
    }
    
    Old_BackspacePressed = New_BackspacePressed;
}

int main()
{

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    
    sf::RenderWindow window(desktop, "My window", sf::Style::Default, sf::State::Fullscreen);

    // sf::View view = window.getView();
    // view.zoom(1.5f);
    // window.setView(view);

    AddBorders();

    emptyMatrix(); 

    while (window.isOpen())
    {

        SimplifyMatrix(); 

        HandleRemoveWallsAction();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Escape)) 
            window.close();

        if (HandleSpecialTileMoving(window) == undefinedTile) // if a special tile isn't selected we can change tiles
            HandleTileToggle(window);

        LeesAlgorithm();

        window.clear(sf::Color::White);

        DrawMatrix(window);

        window.display();
    }

    return 0;
}