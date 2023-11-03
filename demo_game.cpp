#include "ysglfontdata.h"
#include "fssimplewindow.h"
#include <vector>
#include <cmath>
#include <cstdlib> // for rand and srand
#include <ctime> // for time

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

const int SOLDIER_SIZE = 30;
const int ENEMY_RADIUS = 12;


const int MOVE_STEP = 5; // Define the left/right moving step of the soldier
const int BULLET_RADIUS = 5; // Define the size of the bullet
const int BULLET_SPEED = 10; // Define the speed of the bullet
const int SHOOTING_FREQUENCY = 900; // Frequency of bullet shooting in milliseconds

const int MAX_ENEMIES = 20; // Maximum number of enemies that can be generated
const int WALL_GAP = WINDOW_HEIGHT - WINDOW_HEIGHT / 3; // Distance between sets of walls

class Bullet {
public:
    int x, y, radius;
    double angle;

    Bullet(int x, int y, int radius, double angle) : x(x), y(y), radius(radius), angle(angle) {}

    void draw() {
        glBegin(GL_POLYGON);
        for(int i = 0; i < 360; i++) {
            double angle = i * 3.14159 / 180; // Convert degrees to radians
            double fx = x + cos(angle) * radius; // Calculate the x coordinate
            double fy = y + sin(angle) * radius; // Calculate the y coordinate
            glVertex2d(fx, fy);
        }
        glEnd();
    }

    void move() {
        y -= BULLET_SPEED * sin(angle);
        x += BULLET_SPEED * cos(angle);
    }
};

class Soldier {
public:
    int x, y, size;

    Soldier(int x, int y, int size) : x(x), y(y), size(size) {}

    void draw() {
        glBegin(GL_QUADS);
        glVertex2i(x, y);
        glVertex2i(x + size, y);
        glVertex2i(x + size, y + size);
        glVertex2i(x, y + size);
        glEnd();
    }

    void move(int dx, int windowWidth) {
        int newX = x + dx * MOVE_STEP; // Calculate the new x coordinate
        if (newX >= 0 && newX <= windowWidth - size) { // Check if the new x coordinate is within the window
            x = newX; // Update the x coordinate
        }
        // Remove the line that moves the soldier upwards
    }

    // Shooting Method 1: Shoot bullets straight up only
    // std::vector<Bullet> shoot(int numBullets) {
    //     std::vector<Bullet> bullets;
    //     for(int i = 0; i < numBullets; i++) {
    //         double angle = (180.0 / (numBullets + 1) * (i + 1)) * 3.14159 / 180;
    //         bullets.push_back(Bullet(x, y, BULLET_RADIUS, angle));
    //     }
    //     return bullets;
    // }

    // Shoting Method 2: Shoot bullets at different angles
    std::vector<Bullet> shoot(int numBullets) {
        std::vector<Bullet> bullets;
        for(int i = 0; i < numBullets; i++) {
            double angle = (180.0 / (numBullets + 1) * (i + 1)) * 3.14159 / 180;
            bullets.push_back(Bullet(x + size / 2, y, BULLET_RADIUS, angle));
        }
        return bullets;
    }

};

class Enemy {
public:
    double x, y;
    int radius;
    bool isMoving;
    double speed;
    int wallId; // Add this line

    Enemy(double x, double y, int radius, double speed, int wallId) : x(x), y(y), radius(radius), isMoving(false), speed(speed), wallId(wallId) {}

    void draw() {
        glBegin(GL_POLYGON);
        for(int i = 0; i < 360; i++) {
            double angle = i * 3.14159 / 180;
            double fx = x + cos(angle) * radius;
            double fy = y + sin(angle) * radius;
            glVertex2d(fx, fy);
        }
        glEnd();
    }

    void move(double soldierX, double soldierY, int passedWallId) {
        y += speed; // Always move the enemy downwards
        if (wallId <= passedWallId) {
            if (x < soldierX) {
                x += speed; // Move the enemy to the right
            } else if (x > soldierX) {
                x -= speed; // Move the enemy to the left
            }
        }
    }
};

class Wall {
public:
    int x1, y1, x2, y2;
    int operation; // 0: add, 1: subtract, 2: multiply, 3: divide
    bool isPassed; // flag to track if a soldier has passed through the wall
    int wallId; // Add this line

    Wall(int x1, int y1, int x2, int y2, int operation, int wallId) : x1(x1), y1(y1), x2(x2), y2(y2), operation(operation), isPassed(false), wallId(wallId) {}

    void draw() {
        glBegin(GL_LINES);
        glVertex2i(x1, y1);
        glVertex2i(x2, y2);
        glEnd();

        // Draw operation text
        char operationText[10];
        switch(operation) {
            case 0: strcpy(operationText, "+2"); break;
            case 1: strcpy(operationText, "-2"); break;
            case 2: strcpy(operationText, "x2"); break;
            case 3: strcpy(operationText, "/2"); break;
        }
        glColor3ub(0, 0, 0); // Set color to black
        glRasterPos2i((x1 + x2) / 2, y1 + 20); // Position the text below the wall
        YsGlDrawFontBitmap16x20(operationText); // Draw the text
    }

    int performOperation(int numSoldiers) {
        switch(operation) {
            case 0: return numSoldiers + 2; // add 2 soldiers
            case 1: return numSoldiers > 2 ? numSoldiers - 2 : 1; // subtract 2 soldiers, but ensure there's at least 1 soldier
            case 2: return numSoldiers * 2; // multiply by 2
            case 3: return numSoldiers > 1 ? numSoldiers / 2 : 1; // divide by 2, but ensure there's at least 1 soldier
            default: return numSoldiers;
        }
    }

    void move() {
        y1 += 1; // Move the wall downwards
        y2 += 1;
    }
};

// This function generates a set of walls and enemies for the game.
// The function first generates two walls with random operations and adds them to the vector of walls.
// Then, it generates a random number of enemies (up to MAX_ENEMIES) with random x and y coordinates and adds them to the vector of enemies.
void generateSet(std::vector<Wall>& walls, std::vector<Enemy>& enemies, int y, int setId) {
    int operation = rand() % 4; // Random operation (0, 1, 2, or 3)
    walls.push_back(Wall(110, y, 390, y, operation, setId)); // Wall with random operation

    operation = rand() % 4; // Random operation (0, 1, 2, or 3)
    walls.push_back(Wall(410, y, 690, y, operation, setId)); // Wall with random operation

    int numEnemies = rand() % MAX_ENEMIES + 1; // Random number of enemies up to MAX_ENEMIES
    for (int i = 0; i < numEnemies; i++) {
        int enemyX = rand() % (700 - 100 - 2 * ENEMY_RADIUS) + 100 + ENEMY_RADIUS; // Random x coordinate between the road
        int enemyY = y - WALL_GAP/3 - rand() % (2*WALL_GAP/3 - 2 * ENEMY_RADIUS) - ENEMY_RADIUS; // Random y coordinate between the wall and two thirds to the next wall
        double enemySpeed = 1; // Set the speed of the enemy
        enemies.push_back(Enemy(enemyX, enemyY, ENEMY_RADIUS, enemySpeed, setId)); // Set the wallId field to the setId
    }
}

int main() {
    srand(time(0)); // Seed the random number generator
    int windowWidth = WINDOW_WIDTH;
    int windowHeight = WINDOW_HEIGHT;

    FsOpenWindow(0, 0, windowWidth, windowHeight, 1);

    std::vector<Soldier> soldiers;
    soldiers.push_back(Soldier(windowWidth / 2, windowHeight - SOLDIER_SIZE, SOLDIER_SIZE));
    
    std::vector<Wall> walls;
    std::vector<Enemy> enemies;
    int setId = 0;
    generateSet(walls, enemies, 400, setId++);

    std::vector<Bullet> bullets;
    int lastShotTime = FsSubSecondTimer();

    int lastPassedWallId = -1;
    while(FsInkey() != FSKEY_ESC) {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        // Draw road
        glBegin(GL_LINES);
        glVertex2i(100, 0);
        glVertex2i(100, 600);
        glVertex2i(700, 0);
        glVertex2i(700, 600);
        glEnd();

        FsPollDevice();  // Check for user input
        auto key = FsInkey();
        
        if(FSKEY_ESC == key) // if the user press ESC key
        {
            break;  // Exit the game
        }

        // Create bullets every SHOOTING_FREQUENCY milliseconds
        // Shooting Method 1: Shoot bullets straight up only
        // if (FsSubSecondTimer() - lastShotTime >= SHOOTING_FREQUENCY) {
        //     int bulletsPerSoldier = 1; // Each soldier shoots one bullet
        //     for(auto& soldier : soldiers) {
        //         auto newBullets = soldier.shoot(bulletsPerSoldier);
        //         bullets.insert(bullets.end(), newBullets.begin(), newBullets.end());
        //     }
        //     lastShotTime = FsSubSecondTimer();
        // }

        
        // Create bullets every SHOOTING_FREQUENCY milliseconds
        // Shoting Method 2: Shoot bullets at different angles 
        if (FsSubSecondTimer() - lastShotTime >= SHOOTING_FREQUENCY) {
            int bulletsPerSoldier = soldiers.size(); // The number of bullets is equal to the number of soldiers
            auto newBullets = soldiers[0].shoot(bulletsPerSoldier); // Shoot bullets from the first soldier
            bullets.insert(bullets.end(), newBullets.begin(), newBullets.end());
            lastShotTime = FsSubSecondTimer();
        }

        // Draw and move bullets
        for(auto& bullet : bullets) {
            bullet.draw();
            bullet.move();
        }
        
        // Draw and move soldiers
        for(auto& soldier : soldiers) {
            soldier.draw();
            if(key == FSKEY_LEFT) {
                soldier.move(-1, windowWidth); // Move the soldier to the left
            } else if(key == FSKEY_RIGHT) {
                soldier.move(1, windowWidth); // Move the soldier to the right
            } else {
                soldier.move(0, windowWidth); // Keep moving upwards
            }
        }

        // Draw and move enemies
        for(auto& enemy : enemies) {
            enemy.draw();
            if (soldiers.size() > 0) {
                enemy.move(soldiers[0].x, soldiers[0].y, lastPassedWallId);
            }
        }

        // Draw and move walls
        for(auto& wall : walls) {
            wall.draw();
            wall.move();
        }

        // Check for collisions between soldiers and enemies
        for (auto it = soldiers.begin(); it != soldiers.end(); ) {
            bool isHit = false;
            for (auto& enemy : enemies) {
                if (abs(it->x - enemy.x) < it->size && abs(it->y - enemy.y) < enemy.radius) {
                    isHit = true;
                    break;
                }
                
            }
            if (isHit) {
                it = soldiers.erase(it);
            } else {
                ++it;
            }
        }

        // Check for collisions between bullets and enemies
        for (auto it = bullets.begin(); it != bullets.end(); ) {
            bool isHit = false;
            for (auto jt = enemies.begin(); jt != enemies.end(); ) {
                if (abs(it->x - jt->x) < it->radius + jt->radius && abs(it->y - jt->y) < it->radius + jt->radius) {
                    isHit = true;
                    jt = enemies.erase(jt);
                } else {
                    ++jt;
                }
            }
            if (isHit) {
                it = bullets.erase(it);
            } else {
                ++it;
            }
        }

        // Check for collisions between bullets and walls
        for (auto it = bullets.begin(); it != bullets.end(); ) {
            bool isHit = false;
            for (auto& wall : walls) {
                if (it->y <= wall.y1 && it->x >= wall.x1 && it->x <= wall.x2) {
                    isHit = true;
                    break;
                }
            }
            if (isHit) {
                it = bullets.erase(it); // Remove the bullet if it hits a wall
            } else {
                ++it;
            }
        }
       // Check if soldier passed a wall
        for(auto& wall : walls) {
            if(!wall.isPassed && soldiers.size() > 0 && soldiers[0].y <= wall.y1 && soldiers[0].x >= wall.x1 && soldiers[0].x <= wall.x2) {
                wall.isPassed = true; // set the flag to true
                lastPassedWallId = wall.wallId; // update the last passed wall's ID

                for(auto& enemy : enemies) {
                    if (enemy.wallId == wall.wallId) {
                        enemy.isMoving = true; // set the flag to true for the enemies after the passed wall
                    }
                }

                int numSoldiers = soldiers.size();
                numSoldiers = wall.performOperation(numSoldiers);
                while(soldiers.size() < numSoldiers) {
                    int startX = soldiers[0].x + (soldiers.size() % 2 == 0 ? -SOLDIER_SIZE - 1 : SOLDIER_SIZE + 1) * ((soldiers.size() + 1) / 2);
                    soldiers.push_back(Soldier(startX, soldiers[0].y, SOLDIER_SIZE)); // add new soldiers at the same y position as the existing soldier
                }
                while(soldiers.size() > numSoldiers && soldiers.size() > 1) {
                    soldiers.pop_back(); // remove soldiers from the end
                }
                break; // Exit the loop after applying the wall's effect
            }
        }

        // Check if a wall has moved past the end of the window
        if (walls.back().y1 > WALL_GAP) {
            generateSet(walls, enemies, 0, setId++);
        }

        // Remove enemies that have moved past the end of the window
        for (auto it = enemies.begin(); it != enemies.end(); ) {
            if (it->y > windowHeight) {
                it = enemies.erase(it);
            } else {
                ++it;
            }
        }

        // Remove walls that have moved past the end of the window
        for (auto it = walls.begin(); it != walls.end(); ) {
            if (it->y1 > windowHeight) {
                it = walls.erase(it);
            } else {
                ++it;
            }
        }

        
        // Remove enemies that have moved past the end of the window
        for (auto it = enemies.begin(); it != enemies.end(); ) {
            if (it->y > windowHeight) {
                it = enemies.erase(it);
            } else {
                ++it;
            }
        }

        // Remove walls that have moved past the end of the window
        for (auto it = walls.begin(); it != walls.end(); ) {
            if (it->y1 > windowHeight) {
                it = walls.erase(it);
            } else {
                ++it;
            }
        }
        
        FsSwapBuffers();
        FsSleep(25);
    }

    return 0;
}