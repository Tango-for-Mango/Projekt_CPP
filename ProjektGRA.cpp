#include <iostream>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <ctime>
#include <cstdlib>
#include <SDL_image.h>  // dodanie części biblioteki SDL, SDL_images
#include <windows.h>


const int WIDTH = 800, HEIGHT = 600;
const int TILE_SIZE = 30;
const int MAP_WIDTH = 22;
const int MAP_HEIGHT = 20;
const int BASE_ENEMY_SPEED = 2; // Szybkość wrogów (wartość default, bazowa)

// zmienne SDL 
SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
TTF_Font* gFont = nullptr;


enum EnemyType {
    RED,     // Podstawowy wróg
    BLUE,    // Szybszy wróg
    ORANGE    // Wolniejszy wróg, zabiera więcej życia
};

struct Enemy {
    int x, y;
    int pathIndex = 0;
    int speed;
    int lifeImpact;     // Wpływ wroga na liczbę żyć gracza, po osiągnięciu celu
    int health;         // Życie wroga
    EnemyType type;
    std::vector<SDL_Texture*> textures; // Klatki animacji
    int currentFrame = 0;               // Obecna klatka animacji
    Uint32 lastFrameTime = 0;           // Czas ostatniej zmiany klatki
};

struct Projectile {
    float x, y;           // Aktualna pozycja pocisku
    float speedX, speedY; // Prędkość w X i Y
};

//--------------------------------------------------------------------- ŻYCIE I MONETY --------------------------------------------------------------------------------------

int enemyCompletionCount = 10; // Życia gracza
int coins = 50; // Początkowa liczba monet

SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& filePath) { // Ładowanie tekstur wrogów
    SDL_Surface* tempSurface = IMG_Load(filePath.c_str());
    if (!tempSurface) {
        std::cout << "Nie można załadować obrazu: " << filePath << " - " << SDL_GetError() << std::endl; // sprawdzenie, czy da się załadować plik
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    return texture;
}

// Definicja wymiarów mapy
#define MAP_HEIGHT 20
#define MAP_WIDTH 20

bool gameOver = false;

// Funkcja tworząca pierwszą mapę
std::vector<std::pair<int, int>> createMap1(int map[MAP_HEIGHT][MAP_WIDTH]) {
    std::vector<std::pair<int, int>> path = {
        {2, 0}, {2, 1}, {2, 2}, {3, 2}, {3, 3}, {3, 4},
        {4, 4}, {5, 4}, {6, 4}, {7, 4}, {8, 4}, {9, 4}, {10, 4}, {11, 4}, {12, 4}, {13, 4}, {14, 4}, {15, 4}, {16, 4}, {17, 4},
        {17, 5}, {17, 6}, {17, 7}, {17, 8}, {16, 8}, {15, 8}, {14, 8}, {13, 8}, {12, 8}, {11, 8}, {10, 8}, {9, 8}, {8, 8},
        {8, 9}, {7, 9}, {6, 9}, {6, 10}, {5, 10}, {4, 10}, {4, 11}, {4, 12}, {4, 13}, {5, 13}, {6, 13}, {6, 14}, {6, 15}, {7, 15}, {8, 15}, {8, 16},
        {9, 16}, {10, 16}, {11, 16}, {11, 17}, {12, 17}, {13, 17}, {14, 17}, {15, 17}, {16, 17}, {16, 18}, {17, 18}, {18, 18}, {19, 18},
        {19, 19}
    };

    for (const auto& pos : path) {
        int y = pos.first;
        int x = pos.second;
        map[x][y] = 1;
    }
    return path;
}

// Funkcja tworząca drugą mapę
std::vector<std::pair<int, int>> createMap2(int map[MAP_HEIGHT][MAP_WIDTH]) {
    std::vector<std::pair<int, int>> path = {
        {0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2}, {5, 2}, {6, 2}, {6, 3}, {6, 4}, {7, 4},
        {8, 4}, {9, 4}, {10, 4}, {10, 5}, {10, 6}, {10, 7}, {9, 7}, {8, 7}, {8, 8}, {8, 9},
        {9, 9}, {10, 9}, {11, 9}, {12, 9}, {12, 10}, {12, 11}, {12, 12}, {11, 12}, {10, 12}, {9, 12}, {8, 12},
        {7, 12}, {6, 12}, {5, 12}, {4, 12}, {3, 12}, {2, 12}, {1, 12}, {1, 13}, {1, 14}, {2, 14}, {3, 14}, {4, 14},
        {5, 14}, {6, 14}, {6, 15}, {6, 16}, {7, 16}, {8, 16}, {9, 16}, {10, 16}, {11, 16}, {12, 16}, {13, 16}, {14, 16}, {15, 16},
        {16, 16}, {17, 16}, {17, 17}, {17, 18}, {18, 18}, {19, 18}, {19, 19}
    };

    for (const auto& pos : path) {
        int y = pos.first;
        int x = pos.second;
        map[x][y] = 1;
    }
    return path;
}

// 3-cia mampa
std::vector<std::pair<int, int>> createMap3(int map[MAP_HEIGHT][MAP_WIDTH]) {
    std::vector<std::pair<int, int>> path = {
    {5, 0}, {5, 1}, {5, 2}, {6, 2}, {7, 2}, {7, 3}, {7, 4}, {7, 5}, {6, 5}, {5, 5},
    {4, 5}, {3, 5}, {2, 5}, {1, 5}, {1, 6}, {1, 7}, {2, 7}, {3, 7}, {4, 7}, {5, 7},
    {6, 7}, {7, 7}, {7, 8}, {7, 9}, {6, 9}, {5, 9}, {4, 9}, {3, 9}, {2, 9}, {1, 9},
    {0, 9}, {0, 10},{0,11}, {1, 11}, {2, 11}, {3, 11}, {4, 11}, {5, 11}, {6, 11}, {7, 11},
    {8, 11}, {9, 11}, {10, 11}, {11, 11}, {12, 11}, {12, 12}, {12, 13},
    {11, 13}, {10, 13}, {9, 13}, {8, 13}, {7, 13}, {6, 13}, {5, 13}, {5, 14},
    {5, 15}, {5, 16}, {5, 17}, {5, 18}, {5, 19}, {6, 19}, {7, 19}, {8, 19}, {9, 19}, {10, 19}, {11, 19}, {12, 19}
    };

    for (const auto& pos : path) {
        int y = pos.first;
        int x = pos.second;
        map[x][y] = 1;
    }
    return path;
}
//--------------------------------------------------------------------- WYBIERANIE MAPY -------------------------------------------------------------------------------------

std::vector<std::pair<int, int>> createRandomMap(int map[MAP_HEIGHT][MAP_WIDTH]) {
    std::srand(std::time(nullptr)); // Inicjalizacja generatora liczb losowych
    int randomMapIndex = std::rand() % 3; // Losowanie indeksu mapy (0, 1, 2)

    std::vector<std::pair<int, int>> path;
    switch (randomMapIndex) {
    case 0:
        path = createMap1(map);
        std::cout << "Wylosowano mape 1" << std::endl;  // Komunikaty w konsoli, która mapa została wylosowana
        break;
    case 1:
        path = createMap2(map);
        std::cout << "Wylosowano mape 2" << std::endl;  
        break;
    case 2:
        path = createMap3(map);
        std::cout << "Wylosowano mape 3" << std::endl;  
        break;
    default:
        std::cout << "Blad losowania mapy!" << std::endl;
        break;
    }

    return path;
}



Enemy createRandomEnemy(const std::vector<std::pair<int, int>>& path, SDL_Renderer* renderer) {
    Enemy enemy;
    enemy.x = path[0].first * TILE_SIZE;
    enemy.y = path[0].second * TILE_SIZE;
    enemy.pathIndex = 0;

    int randomType = rand() % 3;
    if (randomType == 0) {
        enemy.type = RED;
        enemy.speed = BASE_ENEMY_SPEED;
        enemy.lifeImpact = 1;
        enemy.health = 3;
        enemy.textures = {
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/slime-move-0.png"),
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/slime-move-1.png"),
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/slime-move-2.png"),
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/slime-move-3.png")
        };
    }
    else if (randomType == 1) {
        enemy.type = BLUE;
        enemy.speed = BASE_ENEMY_SPEED * 2;
        enemy.lifeImpact = 1;
        enemy.health = 2;
        enemy.textures = {
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/Wispy1.png"),
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/Wispy2.png"),
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/Wispy3.png"),
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/Wispy4.png"),
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/Wispy5.png"),
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/Wispy6.png"),
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/Wispy7.png"),
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/Wispy8.png"),
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/Wispy9.png"),
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/Wispy10.png")
        };
    }
    else {
        enemy.type = ORANGE;
        enemy.speed = BASE_ENEMY_SPEED / 2;
        enemy.lifeImpact = 2;
        enemy.health = 5;
        enemy.textures = {
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/SimpleEnemies Bat_Idle_0.png"),
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/SimpleEnemies Bat_Idle_1.png"),
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/SimpleEnemies Bat_Idle_2.png"),
            loadTexture(renderer, "C:/Users/kuert/source/repos/SDLvs/ProjektGRA/x64/Debug/assets/SimpleEnemies Bat_Idle_3.png")
        };
    }

    return enemy;
}


void cleanupEnemies(std::vector<Enemy>& enemies) {  // czyszczenie tekstur wrogow by nie doposcic do wycieku pamieci
    for (auto& enemy : enemies) {
        for (auto& texture : enemy.textures) {
            SDL_DestroyTexture(texture);
        }
    }
}

// Enum Typów Wież
enum TowerType { NORMAL, SNIPER };

struct Tower {
    int x, y;
    int width, height; // Dodano wartości szerokości i wysokości
    bool selected;
    int radius;
    bool bought; // Informacja o tym czy wieża zostala juz kupiona
    int previousX; // Zmienna do śledzenia poprzedniego położenia wieży w osi X

        Tower(int x, int y, int width, int height, int radius) : x(x), y(y), width(width), height(height), selected(false), radius(radius),bought(false), previousX(x) {}

        void draw(SDL_Renderer* renderer) {
            // Główna wieża (prostokąt)
            SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255);
            SDL_Rect towerRect = { x, y, width, height };
            SDL_RenderFillRect(renderer, &towerRect);

            // Rysowanie blanek
            SDL_SetRenderDrawColor(renderer, 120, 60, 0, 255);
            int blankWidth = width / 6;
            int blankHeight = height / 8;
            for (int i = 0; i < 8; i += 2) {
                SDL_Rect blankRect = { x + i * blankWidth, y - blankHeight, blankWidth, blankHeight };
                SDL_RenderFillRect(renderer, &blankRect);
            }
            // Obramowanie wieży
            SDL_SetRenderDrawColor(renderer, selected ? 255 : 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &towerRect);
        }
    };


//-------------------------------------------------------------------- DO DOKOŃCZENIA PO SESJI ------------------------------------------------------------------------------

/*

void shootProjectile(Tower& tower, const Enemy& target, std::vector<Projectile>& projectiles) {
    // Utworzenie pocisku na pozycji wieży
    Projectile projectile;
    projectile.x = tower.x + TILE_SIZE / 2; // Środek wieży
    projectile.y = tower.y + TILE_SIZE / 2; // Środek wieży

    // Obliczenie kierunku w stronę wroga
    float dx = (target.x + TILE_SIZE / 2) - projectile.x;
    float dy = (target.y + TILE_SIZE / 2) - projectile.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // Normalizacja kierunku i ustawienie prędkości pocisku
    projectile.speedX = 10 * (dx / distance);
    projectile.speedY = 10 * (dy / distance);

    // Dodanie pocisku do wektora
    projectiles.push_back(projectile);
}


void updateAndRenderProjectiles(SDL_Renderer* renderer, std::vector<Projectile>& projectiles) {
    for (auto it = projectiles.begin(); it != projectiles.end(); ) {
        // Aktualizacja pozycji pocisku
        it->x += it->speedX;
        it->y += it->speedY;

        // Sprawdzenie, czy pocisk wyszedł poza ekran
        if (it->x < 0 || it->x > WIDTH || it->y < 0 || it->y > HEIGHT) {
            it = projectiles.erase(it);
        }
        else {
            // Rysowanie pocisku
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Żółty kolor pocisku
            SDL_Rect projectileRect = { static_cast<int>(it->x), static_cast<int>(it->y), 5, 5 };
            SDL_RenderFillRect(renderer, &projectileRect);
            ++it;
        }
    }
}

*/

// ---------------------------------------------------------------------- PORUSZANIE SIĘ WROGÓW PO MAPIE --------------------------------------------------------------------

void updateEnemyPosition(Enemy& enemy, const std::vector<std::pair<int, int>>& path) {
    if (enemy.pathIndex < path.size() - 1) {
        int targetX = path[enemy.pathIndex + 1].first * TILE_SIZE;
        int targetY = path[enemy.pathIndex + 1].second * TILE_SIZE;

        int dx = targetX - enemy.x;
        int dy = targetY - enemy.y;

        if (abs(dx) > enemy.speed) enemy.x += (dx > 0) ? enemy.speed : -enemy.speed;
        else enemy.x = targetX;

        if (abs(dy) > enemy.speed) enemy.y += (dy > 0) ? enemy.speed : -enemy.speed;
        else enemy.y = targetY;

        if (enemy.x == targetX && enemy.y == targetY) {
            enemy.pathIndex++;
        }
    }
    else {
        enemyCompletionCount -= enemy.lifeImpact;
        enemy.pathIndex = 0;
        enemy.x = path[0].first * TILE_SIZE;
        enemy.y = path[0].second * TILE_SIZE;
    }
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, int x, int y) {
    SDL_Color color = { 255, 255, 255 };
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    int textWidth = 0, textHeight = 0;
    SDL_QueryTexture(texture, NULL, NULL, &textWidth, &textHeight);
    SDL_Rect dstRect = { x, y, textWidth, textHeight };
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void renderEnemies(SDL_Renderer* renderer, std::vector<Enemy>& enemies, Uint32 currentTime) {   // renderowanie wrogów korzystając z plików graficznych w "assets"
    for (auto& enemy : enemies) {
        if (!enemy.textures.empty()) {
            // Aktualizacja animacji
            if (currentTime - enemy.lastFrameTime >= 100) { // Zmiana klatki co 100 ms
                enemy.currentFrame = (enemy.currentFrame + 1) % enemy.textures.size();
                enemy.lastFrameTime = currentTime;
            }

            SDL_Rect dstRect = { enemy.x, enemy.y, TILE_SIZE, TILE_SIZE };

            SDL_RenderCopy(renderer, enemy.textures[enemy.currentFrame], NULL, &dstRect);
        }
    }
}

void renderCoins(SDL_Renderer* renderer, TTF_Font* font) {
    std::string coinsText = "Monety: " + std::to_string(coins);
    renderText(renderer, font, coinsText, 630, 500); // Pozycja w prawym dolnym rogu
}

void purchaseTower(Tower& tower) {
    if (!tower.bought && tower.previousX >= 600 && tower.x < 600 && coins >= 10) {
        tower.bought = true;  // Oznaczamy wieżę jako kupioną
        coins -= 10;  // Odejmujemy 10 monet
    }
}

void sellTower(Tower& tower) {
    if (!tower.bought && tower.previousX <= 600 && tower.x > 600 && coins >= 10) {
        tower.bought = true;  
        coins += 10;  // Dodajemy 10 monet
    }
}


// Opcje menu
std::vector<std::string> menuOptions = { "GRAJ", "SALA REKORDOW", "WYJDZ" };
int selectedOption = 0;

bool initSDL();
void closeSDL();
void rrenderText(const std::string& text, int x, int y, SDL_Color color, bool selected);
void renderMenu();
void handleMenuEvents(bool& quit, bool& startGame);

// ------------------------------------------------------------------ KOORDYNATY WIEŻYCZEK ----------------------------------------------------------------------------------
std::vector<int> Xwiez = { 680, 680, 680 };
std::vector<int> Ywiez = { 100, 250, 400 };


int main(int argc, char* argv[]) {
    srand(static_cast<unsigned>(time(0))); // Inicjalizacja generatora losowego
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    if (!initSDL()) {
        std::cerr << "Failed to initialize SDL!\n";
        return -1;
    }

    bool quit = false;
    bool startGame = false;

    // ------------------------------------------------------------------ PĘTLA GRY -----------------------------------------------------------------------------------------

    while (!quit) {
        if (!startGame) {
            renderMenu();
            handleMenuEvents(quit, startGame); // Obsługa zdarzeń menu
        }
        else {
            // Inicjalizacja gry
            std::vector<Projectile> projectiles;
            std::vector<Tower> towers;      

            SDL_Window* window = SDL_CreateWindow("Mapa SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_ALLOW_HIGHDPI);
            if (NULL == window) {
                std::cout << "Nie można otworzyć okna: " << SDL_GetError() << std::endl;
                break;
            }

            SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if (!renderer) {
                std::cerr << "Nie udało się utworzyć renderer'a: " << SDL_GetError() << std::endl;
                SDL_DestroyWindow(window);
                break;
            }

            TTF_Font* font = TTF_OpenFont("Arial.ttf", 24);
            if (!font) {
                std::cout << "Nie można załadować czcionki: " << TTF_GetError() << std::endl;
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                break;
            }

            int map[MAP_HEIGHT][MAP_WIDTH] = { 0 };
            std::vector<std::pair<int, int>> path = createRandomMap(map);

            std::vector<Enemy> enemies;
            Uint32 lastSpawnTime = SDL_GetTicks();



            bool running = true;
            bool isDragging = false; 
            int selectedTowerIndex = -1; 

            while (running) {
                SDL_Event windowEvent;

                           
                
                //-------------------------------------------------------------- WEKTOR WIEZ -------------------------------------------------------------------------------
                    
                    std::vector<Tower> towers = { 
                          {Xwiez[0], Ywiez[0], 26, 50, 5},
                          {Xwiez[1], Ywiez[1], 26, 50, 5},
                          {Xwiez[2], Ywiez[2], 26, 50, 5}
                          };

                while (SDL_PollEvent(&windowEvent)) {
                    if (windowEvent.type == SDL_QUIT) {
                        running = false;
                        quit = true; // Zakończ całą aplikację
                    }
                    else if (windowEvent.type == SDL_KEYDOWN) {
                        if (windowEvent.key.keysym.sym == SDLK_ESCAPE) {
                            running = false; // Zakończ grę
                            startGame = false; // Powrót do menu
                        }
                    }
                    else if (windowEvent.type == SDL_MOUSEBUTTONDOWN) { //-------------------------- PRZESUWANIE WIEŻ ------------------------------------------------------
                        int mouseX;
                        int mouseY;
                        SDL_GetMouseState(&mouseX, &mouseY); 
                        selectedTowerIndex = -1;
                        for (size_t i = 0; i < towers.size(); i++) {
                            if (mouseX >= towers[i].x && mouseX <= towers[i].x + towers[i].width &&
                                mouseY >= towers[i].y && mouseY <= towers[i].y + towers[i].height) {
                                selectedTowerIndex = i;
                                isDragging = true;
                                break;
                            }
                        }
                    }
                    else if (windowEvent.type == SDL_MOUSEBUTTONUP) {
                        isDragging = false;
                    }
                    else if (windowEvent.type == SDL_MOUSEMOTION) {
                        if (isDragging && selectedTowerIndex != -1) {
                            int mouseX;
                            int mouseY;
                            SDL_GetMouseState(&mouseX, &mouseY);
                            bool validPosition = true;
                            for (size_t i = 0; i < towers.size(); i++) {
                                if (i != selectedTowerIndex) {
                                    int dx = mouseX - towers[i].x;
                                    int dy = mouseY - towers[i].y;
                                    double distance = std::sqrt(dx * dx + dy * dy);
                                    if (distance < (towers[selectedTowerIndex].radius + towers[i].radius)) {
                                        validPosition = false;
                                        break;
                                    }
                                }
                            }
                            if (mouseX >= 0 && mouseX + towers[selectedTowerIndex].width <= 1400 && 
                                mouseY >= 50 && mouseY + towers[selectedTowerIndex].height <= 750 && 
                                validPosition) { 

                                // Przed aktualizacją pozycji zapisujemy poprzednią wartość X
                                towers[selectedTowerIndex].previousX = towers[selectedTowerIndex].x; 

                                // Aktualizujemy pozycję wieży
                                towers[selectedTowerIndex].x = mouseX; 
                                towers[selectedTowerIndex].y = mouseY; 
                                Xwiez[selectedTowerIndex] = mouseX; 
                                Ywiez[selectedTowerIndex] = mouseY; 

                                // Sprawdzenie, czy wieża przekroczyła linię x = 400 i nie była wcześniej kupiona
                                purchaseTower(towers[selectedTowerIndex]); 
                                sellTower(towers[selectedTowerIndex]);
                            }
                        }
                    }
                }


                if (!gameOver) {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderClear(renderer);


                    Uint32 currentTime = SDL_GetTicks();
                    if (currentTime - lastSpawnTime >= 1000 + (rand() % 4000)) {
                        enemies.push_back(createRandomEnemy(path, renderer));
                        lastSpawnTime = currentTime;
                    }

                    for (int i = 0; i < MAP_HEIGHT; ++i) {
                        for (int j = 0; j < MAP_WIDTH; ++j) {
                            if (map[i][j] == 1) {
                                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                            }
                            else {
                                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                            }

                            SDL_Rect tileRect = { j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                            SDL_RenderFillRect(renderer, &tileRect);
                        }
                    }

                    for (const auto& proj : projectiles) {
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                        SDL_Rect projRect = { proj.x, proj.y, 3, 3 };
                        SDL_RenderFillRect(renderer, &projRect);
                    }

                    for (auto it = enemies.begin(); it != enemies.end();) {
                        updateEnemyPosition(*it, path);

                        if (it->pathIndex == 0 && it->x == path[0].first * TILE_SIZE && it->y == path[0].second * TILE_SIZE) {
                            it = enemies.erase(it);
                        }
                        else {
                            ++it;
                        }
                    }

                    renderEnemies(renderer, enemies, SDL_GetTicks());
                    renderText(renderer, font, "ZYCIE: " + std::to_string(enemyCompletionCount), WIDTH - 170, 20); // Rysowanie liczby życia
                    renderCoins(renderer, font); // Rysowanie liczby monet 


                   for (Tower& tower : towers) {
                        tower.draw(renderer); // Rysowanie każdej wieży z wektora
                   }
                    

                    // ---------------------------------------- Aktualizacja i renderowanie pocisków -----------------------------------------------------
                   // updateAndRenderProjectiles(renderer, projectiles);

                    // Warunek dla gameOver
                    if (enemyCompletionCount <= 0) {
                        gameOver = true;
                    }

                    SDL_RenderPresent(renderer);
                }
                else {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderClear(renderer);
                    renderText(renderer, font, "PRZEGRANA", WIDTH / 2 - 50, HEIGHT / 2);
                    SDL_RenderPresent(renderer);
                    SDL_Delay(3000);

                    running = false;
                    startGame = false; // Powrót do menu
                }

                SDL_Delay(16);
            }

            // Cleanup zasobów
            TTF_CloseFont(font);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            cleanupEnemies(enemies);
        }
    }

    closeSDL();
    return EXIT_SUCCESS;
}


bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return false;
    }

    if (TTF_Init() == -1) {
        std::cerr << "TTF could not initialize! TTF_Error: " << TTF_GetError() << "\n";
        return false;
    }

    gWindow = SDL_CreateWindow("Main Menu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!gWindow) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
        return false;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!gRenderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << "\n";
        return false;
    }

    gFont = TTF_OpenFont("arial.ttf", 28);
    if (!gFont) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << "\n";
        return false;
    }

    return true;
}

void closeSDL() {
    TTF_CloseFont(gFont);
    gFont = nullptr;

    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gRenderer = nullptr;
    gWindow = nullptr;

    TTF_Quit();
    SDL_Quit();
}

void rrenderText(const std::string& text, int x, int y, SDL_Color color, bool selected) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, text.c_str(), selected ? SDL_Color{ 255, 255, 0, 255 } : color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);

    SDL_Rect renderQuad = { x, y, textSurface->w, textSurface->h };
    SDL_RenderCopy(gRenderer, textTexture, nullptr, &renderQuad);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderMenu() {
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);

    SDL_Color textColorW = { 255, 255, 255, 255 }; // kolor czcionki (biała)
    SDL_Color textColorO = { 255, 165, 0, 255 };   // kolor czcionki (pomarańczowa)
    // Renderowanie instrukcji w menu głównym
    int instructionY = 40;
    rrenderText("Instrukcja", WIDTH / 2 - 350, instructionY, textColorO, false);
    rrenderText("ESC - wyjscie do menu", WIDTH / 2 - 350, instructionY + 30, textColorW, false);
    rrenderText("LPM - przesuwanie wiez", WIDTH / 2 - 350, instructionY + 60, textColorW, false);
    rrenderText("Strz. w gore/dol - poruszanie po menu", WIDTH / 2 - 350, instructionY + 90, textColorW, false);
    rrenderText("Koszt wiezy: 10 monet", WIDTH / 2 - 350, instructionY + 120, textColorW, false);

    // Renderowanie opcji menu
    int y = 200;
    for (size_t i = 0; i < menuOptions.size(); ++i) {
        rrenderText(menuOptions[i], WIDTH / 2 - 50, y, textColorW, i == selectedOption);
        y += 50;
    }

    SDL_RenderPresent(gRenderer);
}

void handleMenuEvents(bool& quit, bool& startGame) {
    SDL_Event e;

    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        }
        else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
            case SDLK_UP:
                selectedOption = (selectedOption - 1 + menuOptions.size()) % menuOptions.size();
                break;
            case SDLK_DOWN:
                selectedOption = (selectedOption + 1) % menuOptions.size();
                break;
            case SDLK_RETURN:
                if (menuOptions[selectedOption] == "GRAJ") {
                    startGame = true;
                    enemyCompletionCount = 10;
                    gameOver = false;
                }
                else if (menuOptions[selectedOption] == "WYJDZ") {
                    quit = true;
                }
                break;
            }
        }
    }
}