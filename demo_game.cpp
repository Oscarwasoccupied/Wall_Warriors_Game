#include "ysglfontdata.h"
#include "fssimplewindow.h"
#include "yssimplesound.h"
#include <vector>
#include <cmath>
#include <cstdlib> // for rand and srand
#include <ctime> // for time

#define DEBUG_PRINT 1
#ifdef DEBUG_PRINT
#define DEBUG_PRINT(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif


const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

const int SOLDIER_SIZE = 20;
const int ENEMY_RADIUS = 10;

const int MOVE_STEP = 5; // Define the left/right moving step of the soldier
const int BULLET_RADIUS = 5; // Define the size of the bullet
const int BULLET_SPEED = 10; // Define the speed of the bullet
const int SHOOTING_FREQUENCY = 900; // Frequency of bullet shooting in milliseconds

int CURRENT_ENEMIES = 7;
const int MAX_ENEMIES = 30; // Maximum number of enemies that can be generated
const int WALL_GAP = WINDOW_HEIGHT - WINDOW_HEIGHT / 3; // Distance between sets of walls

const double SPEED = 1.5; // Define the speed of the walls and enemies
double globalSpeedMultiplier = 1;

class SpeedPowerUp {
public:
    double x, y; // position of the speed up
    int radius; // radius of the speedup

    SpeedPowerUp(double x, double y, int radius) : x(x), y(y), radius(radius) {}

    void draw() {
        glColor3ub(0, 255, 0); // color set to green
        glBegin(GL_POLYGON);
        for (int i = 0; i < 360; i++) {
            double angle = i * 3.14159 / 180;
            double fx = x + cos(angle) * radius;
            double fy = y + sin(angle) * radius;
            glVertex2d(fx, fy);
        }
        glEnd();
        glColor3ub(0, 0, 0);
        glRasterPos2i(x - 20, y + 5);
        YsGlDrawFontBitmap8x12("speed!");
    }
    void move(double speed) {
        y += speed;
    }
};

class BulletSpeedPowerUp {
public:
    double x, y; // position of the power-up
    int radius; // radius of the power-up

    BulletSpeedPowerUp(double x, double y, int radius) : x(x), y(y), radius(radius) {}

    void draw() {
        glColor3ub(255, 165, 0); // Orange color
        glBegin(GL_POLYGON);
        for (int i = 0; i < 360; i++) {
            double angle = i * 3.14159 / 180;
            double fx = x + cos(angle) * radius;
            double fy = y + sin(angle) * radius;
            glVertex2d(fx, fy);
        }
        glEnd();
        glColor3ub(0, 0, 0);
        glRasterPos2i(x - 20, y + 5);
        YsGlDrawFontBitmap8x12("bullet speed!");
    }

    void move(double speed) {
        y += speed;
    }
};


// Class representing a bullet in the game
// Each bullet has a position (x, y), a radius, and an angle
// The bullet can be drawn and moved
class Bullet {
public:
    double x, y; // Position of the bullet
    int radius; // Size of the bullet
    double angle; // Direction of the bullet

    // Constructor for the Bullet class
    Bullet(int x, int y, int radius, double angle) : x(x), y(y), radius(radius), angle(angle) {}

    // Method to draw the bullet on the screen
    void draw() {
        glBegin(GL_POLYGON);
        for (int i = 0; i < 360; i++) {
            double angle = i * 3.14159 / 180; // Convert degrees to radians
            double fx = x + cos(angle) * radius; // Calculate the x coordinate
            double fy = y + sin(angle) * radius; // Calculate the y coordinate
            glVertex2d(fx, fy);
        }
        glEnd();
    }

    // Method to move the bullet according to its angle
    void move() {
        y -= BULLET_SPEED * sin(angle);
        x += BULLET_SPEED * cos(angle);
    }
};

// Class representing a soldier in the game
// Each soldier has a position (x, y) and a size
// The soldier can be drawn, moved, and can shoot bullets
class Soldier {
public:
    double x, y; // Position of the soldier
    int size; // Size of the soldier
    double speedMultiplier = 1.0;

    // Constructor for the Soldier class
    Soldier(int x, int y, int size) : x(x), y(y), size(size) {}

    void setToGlobalSpeed() {
        speedMultiplier = globalSpeedMultiplier;
    }

    // Method to draw the soldier on the screen
    void draw() {
        glColor3ub(0, 0, 255);
        glBegin(GL_QUADS);
        glVertex2i(x, y);
        glVertex2i(x + size, y);
        glVertex2i(x + size, y + size);
        glVertex2i(x, y + size);
        glEnd();
        glColor3ub(0, 0, 0);
    }

    // Method to move the soldier left or right within the window
    void move(int dx, int windowWidth, int leftBoundary, int rightBoundary) {
        int newX = x + dx * MOVE_STEP * speedMultiplier; // Calculate the new x coordinate
        if ((newX >= leftBoundary && newX <= rightBoundary - size) || // Check if the new x coordinate is within the window
            (x == leftBoundary && dx > 0) || // Check if the soldier is at the left boundary and the movement is to the right
            (x == rightBoundary - size && dx < 0)) { // Check if the soldier is at the right boundary and the movement is to the left
            x = newX; // Update the x coordinate
        }
    }

    // Method to shoot bullets at different angles
    std::vector<Bullet> shoot(int numBullets) {
        std::vector<Bullet> bullets;
        for (int i = 0; i < numBullets; i++) {
            double angle = (180.0 / (numBullets + 1) * (i + 1)) * 3.14159 / 180;
            bullets.push_back(Bullet(x + size / 2, y, BULLET_RADIUS, angle));
        }
        return bullets;
    }

};

// Class representing an enemy in the game
// Each enemy has a position (x, y), a radius, a speed, and a wallId
// The enemy can be drawn and moved
// The enemy moves downwards and towards the soldier if it has passed the wall with the same wallId
class Enemy {
public:
    double x, y;
    int radius;
    bool isMoving;
    double speed;
    int wallId; // The id of the wall that the enemy is associated with

    // Constructor for the Enemy class
    Enemy(double x, double y, int radius, double speed, int wallId) : x(x), y(y), radius(radius), isMoving(false), speed(speed), wallId(wallId) {}

    // Method to draw the enemy on the screen
    void draw() {
        glColor3ub(255, 0, 0); // Set color to red
        glBegin(GL_POLYGON);
        for (int i = 0; i < 360; i++) {
            double angle = i * 3.14159 / 180;
            double fx = x + cos(angle) * radius;
            double fy = y + sin(angle) * radius;
            glVertex2d(fx, fy);
        }
        glEnd();
        glColor3ub(0, 0, 0); // Set color to red
    }

    // Method to move the enemy
    // The enemy always moves downwards
    // If the enemy has passed the wall with the same wallId, it moves towards the soldier
    void move(double soldierX, double soldierY, int passedWallId) {
        y += speed; // Always move the enemy downwards
        if (wallId <= passedWallId) {
            if (x < soldierX) {
                x += speed; // Move the enemy to the right
            }
            else if (x > soldierX) {
                x -= speed; // Move the enemy to the left
            }
        }
    }
};

// Class representing an obstacle in the game
// Each enemy has a position (x, y), a side and a speed
// The enemy can be drawn and moved
// The enemy moves downwards and towards the soldier if it has passed the wall with the same wallId
class Obstacle {
public:
    double x, y;
    int halfside;
    int life;
    double speed;

    // Constructor for the Enemy class
    Obstacle(double x, double y, int side, double speed, int life) : x(x), y(y), halfside(side/2), speed(speed), life(life) {}

    // Method to draw the enemy on the screen
    void draw() {
        glColor3ub(0, 255, 0); // Set color to green
        glBegin(GL_QUADS);
        glVertex2d(x - halfside, y - halfside);
        glVertex2d(x + halfside, y - halfside);
        glVertex2d(x + halfside, y + halfside);
        glVertex2d(x - halfside, y + halfside);
        glEnd();
        glColor3ub(255, 0, 255);
        glRasterPos2i(x - halfside, y);
        char lifeStr[256];
        sprintf(lifeStr, "Life: %d", life);
        YsGlDrawFontBitmap8x12(lifeStr);
        glColor3ub(0, 0, 0);
    }

    // Method to move the obstacle
    void move(double speed) {
        y += speed; // Always move the obstacle downwards
    }
};

// Class representing an debuff obstacle in the game
// 
class debuffObstacle {
public:
    double x, y;
    int halfside;
    int life;
    double speed;

    // Constructor for the Enemy class
    debuffObstacle(double x, double y, int side, double speed, int life) : x(x), y(y), halfside(side / 2), speed(speed), life(life) {}

    // Method to draw the enemy on the screen
    void draw() {
        glColor3ub(255, 0, 0); // Set color to red
        glBegin(GL_QUADS);
        glVertex2d(x - halfside, y - halfside);
        glVertex2d(x + halfside, y - halfside);
        glVertex2d(x + halfside, y + halfside);
        glVertex2d(x - halfside, y + halfside);
        glEnd();
        glColor3ub(0, 0, 255);
        glRasterPos2i(x - halfside, y);
        char lifeStr[256];
        sprintf(lifeStr, "Life: %d", life);
        YsGlDrawFontBitmap8x12(lifeStr);
        glColor3ub(0, 0, 0);
    }

    // Method to move the debuff obstacle
    void move(double speed) {
        y += speed; // Always move the debuff obstacle downwards
    }
};

// Class representing a wall in the game
// Each wall has two points (x1, y1) and (x2, y2), an operation, a flag to track if a soldier has passed through the wall, and a wallId
// The wall can be drawn, moved, and perform an operation on the number of soldiers
class Wall {
public:
    double x1, y1, x2, y2; // Change y1 and y2 to double
    int operation; // 0: add, 1: subtract, 2: multiply, 3: divide
    bool isPassed; // flag to track if a soldier has passed through the wall
    int wallId; // Add this line
    bool operationPerformed; // Flag to check if the operation has been performed
    bool canChangeSoldiers; // Flag to check if the number of soldiers can be changed
    // Constructor for the Wall class
    Wall(double x1, double y1, double x2, double y2, int operation, int wallId)
        : x1(x1), y1(y1), x2(x2), y2(y2), operation(operation), wallId(wallId), isPassed(false), operationPerformed(false), canChangeSoldiers(true) {}

    // Method to draw the wall on the screen
    void draw() {
        glBegin(GL_LINES);
        glVertex2i(x1, y1);
        glVertex2i(x2, y2);
        glEnd();

        // Draw operation text
        char operationText[10];
        switch (operation) {
        case 0: strcpy(operationText, "+2"); break;
        case 1: strcpy(operationText, "-2"); break;
        case 2: strcpy(operationText, "x2"); break;
        case 3: strcpy(operationText, "/2"); break;
        }
        glColor3ub(0, 0, 0); // Set color to black
        glRasterPos2i((x1 + x2) / 2, y1 + 20); // Position the text below the wall
        YsGlDrawFontBitmap16x20(operationText); // Draw the text
    }

    // Method to perform the operation on the number of soldiers
    int performOperation(int numSoldiers) {
        int newNumSoldiers;
        switch (operation) {
        case 0:
            newNumSoldiers = numSoldiers + 2; // add 2 soldiers
            DEBUG_PRINT("Passing through a +2 wall. Number of soldiers: %d\n", newNumSoldiers);
            break;
        case 1:
            newNumSoldiers = numSoldiers > 2 ? numSoldiers - 2 : 1; // subtract 2 soldiers, but ensure there's at least 1 soldier
            DEBUG_PRINT("Passing through a -2 wall. Number of soldiers: %d\n", newNumSoldiers);
            break;
        case 2:
            newNumSoldiers = numSoldiers * 2; // multiply by 2
            DEBUG_PRINT("Passing through a *2 wall. Number of soldiers: %d\n", newNumSoldiers);
            break;
        case 3:
            newNumSoldiers = numSoldiers > 1 ? numSoldiers / 2 : 1; // divide by 2, but ensure there's at least 1 soldier
            DEBUG_PRINT("Passing through a /2 wall. Number of soldiers: %d\n", newNumSoldiers);
            break;
        default:
            newNumSoldiers = numSoldiers;
        }
        return newNumSoldiers;
    }

    // Method to move the wall
    void move(double speed) {
        y1 += speed; // Move the wall downwards
        y2 += speed;
    }
};

// This function generates a set of walls and enemies for the game.
// The function first generates two walls with random operations and adds them to the vector of walls.
// Then, it generates a random number of enemies (up to MAX_ENEMIES) with random x and y coordinates and adds them to the vector of enemies.
void generateSet(std::vector<Wall>& walls, std::vector<Enemy>& enemies, std::vector<Obstacle>& obstacles, std::vector<debuffObstacle>& debuffobstacles, int y, int setId) {
    int operation = rand() % 4; // Random operation (0, 1, 2, or 3)
    walls.push_back(Wall(110, y, 390, y, operation, setId)); // Wall with random operation

    operation = rand() % 4; // Random operation (0, 1, 2, or 3)
    walls.push_back(Wall(410, y, 690, y, operation, setId)); // Wall with random operation

    // int numEnemies = rand() % MAX_ENEMIES + 1; // Random number of enemies up to MAX_ENEMIES
    int numEnemies = rand() % CURRENT_ENEMIES + 3; // Random number of enemies up to CURRENT_ENEMIES
    if (CURRENT_ENEMIES < MAX_ENEMIES) {
        CURRENT_ENEMIES += 2;
    }
    for (int i = 0; i < numEnemies; i++) {
        int enemyX = rand() % (700 - 100 - 2 * ENEMY_RADIUS) + 100 + ENEMY_RADIUS; // Random x coordinate between the road
        int enemyY = y - WALL_GAP / 3 - rand() % (2 * WALL_GAP / 3 - 2 * ENEMY_RADIUS) - ENEMY_RADIUS; // Random y coordinate between the wall and two thirds to the next wall
        double enemySpeed = 1; // Set the speed of the enemy
        enemies.push_back(Enemy(enemyX, enemyY, ENEMY_RADIUS, SPEED, setId)); // Set the speed of the enemy, Set the wallId field to the setId
    }
    int randomlife = rand() % 3 + 3;// random from 3 - 5
    int randomObstacle = rand() % 2;// random number 0 or 1
    int obstacleX = (randomObstacle == 0) ? 550 : 250; //// Set the x-coordinate based on the random number
    int debuffobstacleX = 800 - obstacleX;
    obstacles.push_back(Obstacle(obstacleX, y + 40, 40, 1, randomlife));
    debuffobstacles.push_back(debuffObstacle(debuffobstacleX, y + 40, 40, 1, randomlife-2));//debuffobstacle easier to break

}

int main() {
    bool gameEnded = false;

    int bulletShootingFrequency = SHOOTING_FREQUENCY;

    int numSoldiers = 1; // number of soldiers
    int enemiesDefeated = 0; // eneny defeated

    YsSoundPlayer player;
    YsSoundPlayer::SoundData bgmData;

    if (YSOK != bgmData.LoadWav("source/bgm.wav")) {
        printf("Failed to read background music\n");
        return 1;
    }

    YsSoundPlayer::SoundData hitSound;
    if (YSOK != hitSound.LoadWav("source/hit.wav")) {
        printf("Failed to read hit sound\n");
        return 1;
    }

    srand(time(0)); // Seed the random number generator
    int windowWidth = WINDOW_WIDTH;
    int windowHeight = WINDOW_HEIGHT;

    FsOpenWindow(0, 0, windowWidth, windowHeight, 1, "Warrior Game");

    std::vector<Soldier> soldiers;
    soldiers.push_back(Soldier(windowWidth / 2, windowHeight - SOLDIER_SIZE, SOLDIER_SIZE));

    std::vector<Wall> walls;
    std::vector<Enemy> enemies;
    std::vector<Obstacle> obstacles;
    std::vector<debuffObstacle> debuffobstacles;
    int setId = 0;
    generateSet(walls, enemies, obstacles,debuffobstacles, 400, setId++);

    std::vector<Bullet> bullets;
    int lastShotTime = FsSubSecondTimer();

    int lastPassedWallId = -1;

    bool powerUpVisible = false;
    bool bulletSpeedPowerUpVisible = false;


    int powerUpTimer = 0;
    const int POWER_UP_INTERVAL = 10000;
    SpeedPowerUp speedPowerUp(0, 0, 10);
    BulletSpeedPowerUp bulletSpeedPowerUp(0, 0, 10);

    int lastFrameTime = FsSubSecondTimer();

    // play music
    player.Start();
    player.PlayBackground(bgmData);
    while (FsInkey() != FSKEY_ESC) {

        if (gameEnded) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glColor3ub(100, 100, 100);
            glRasterPos2i(170, 300);
            char endStr[256];
            sprintf(endStr, "Game Over! Enemies defeated: %d", enemiesDefeated);
            YsGlDrawFontBitmap16x20(endStr);

            glRasterPos2i(300, 340);
            YsGlDrawFontBitmap12x16("Press ESC to exit...");

            FsSwapBuffers();
            FsPollDevice();
            if (FSKEY_ESC == FsInkey()) {
                break;
            }
            FsSleep(25);
            continue;
        }

        int currentFrameTime = FsSubSecondTimer();
        int deltaTime = currentFrameTime - lastFrameTime; // »ñÈ¡Ö¡¼ä¸ô
        lastFrameTime = currentFrameTime;

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

        if (FSKEY_ESC == key) // if the user press ESC key
        {
            break;  // Exit the game
        }

        // Create bullets every bulletShootingFrequency milliseconds
        // Shoting Method 2: Shoot bullets at different angles 
        if (FsSubSecondTimer() - lastShotTime >= bulletShootingFrequency) {
            int bulletsPerSoldier = soldiers.size(); // The number of bullets is equal to the number of soldiers
            auto newBullets = soldiers[0].shoot(bulletsPerSoldier); // Shoot bullets from the first soldier
            bullets.insert(bullets.end(), newBullets.begin(), newBullets.end());
            lastShotTime = FsSubSecondTimer();
        }

        // Draw and move bullets
        for (auto& bullet : bullets) {
            bullet.draw();
            bullet.move();
        }

        int leftBoundary = 100; // Left boundary of the road
        int rightBoundary = 700; // Right boundary of the road

        // Draw and move soldiers
        for (auto& soldier : soldiers) {
            soldier.draw();
            if (key == FSKEY_LEFT) {
                soldier.move(-1, windowWidth, leftBoundary, rightBoundary); // Move the soldier to the left
            }
            else if (key == FSKEY_RIGHT) {
                soldier.move(1, windowWidth, leftBoundary, rightBoundary); // Move the soldier to the right
            }
            else {
                soldier.move(0, windowWidth, leftBoundary, rightBoundary); // Keep moving upwards
            }
        }

        // Draw and move walls
        for (auto& obstacle : obstacles) {
            if (obstacle.life > 0)
            {
                obstacle.draw();
                obstacle.move(SPEED);
            }
        }
        for (auto& debuffobstacle : debuffobstacles) {
            if (debuffobstacle.life > 0)
            {
                debuffobstacle.draw();
                debuffobstacle.move(SPEED);
            }
        }

        // Draw and move enemies
        for (auto& enemy : enemies) {
            enemy.draw();
            if (soldiers.size() > 0) {
                enemy.move(soldiers[0].x, soldiers[0].y, lastPassedWallId);
            }
        }

        // Draw and move walls
        for (auto& wall : walls) {
            wall.draw();
            wall.move(SPEED);
        }
        
        // Check for collisions between soldiers and enemies
        for (auto it = soldiers.begin(); it != soldiers.end(); ) {
            bool isHit = false;
            std::vector<Enemy>::iterator hitEnemy;

            for (auto jt = enemies.begin(); jt != enemies.end(); ++jt) {
                if (abs(it->x - jt->x) < it->size && abs(it->y - jt->y) < jt->radius) {
                    isHit = true;
                    hitEnemy = jt;
                    break;
                }
            }

            if (isHit) {
                it = soldiers.erase(it);
                enemies.erase(hitEnemy);
            }
            else {
                ++it;
            }
        }

        if (!powerUpVisible && powerUpTimer >= POWER_UP_INTERVAL) {
            int randomX = rand() % (windowWidth - 20) + 10;
            int randomY = rand() % (windowHeight - 20) + 10;

            randomX = rand() % (700 - 100 - 2 * ENEMY_RADIUS) + 100 + ENEMY_RADIUS; // Random x coordinate between the road
            randomY = randomY - WALL_GAP / 3 - rand() % (2 * WALL_GAP / 3 - 2 * ENEMY_RADIUS) - ENEMY_RADIUS; // Random y coordinate between the wall and two thirds to the next wall

            speedPowerUp = SpeedPowerUp(randomX, randomY, 10);
            powerUpVisible = true;
            powerUpTimer = 0;
        }

        if (!bulletSpeedPowerUpVisible && rand() % 1000 < 5 && !walls.empty()) {
            int wallIndex = rand() % (walls.size() - 1);
            int randomY = (walls[wallIndex].y1 + walls[wallIndex + 1].y1) / 2;
            int randomX = rand() % (700 - 100) + 100;

            bulletSpeedPowerUp = BulletSpeedPowerUp(randomX, randomY, 10);
            bulletSpeedPowerUpVisible = true;
        }


        if (bulletSpeedPowerUpVisible) {
            for (auto& soldier : soldiers) {
                if (abs(soldier.x - bulletSpeedPowerUp.x) < SOLDIER_SIZE &&
                    abs(soldier.y - bulletSpeedPowerUp.y) < bulletSpeedPowerUp.radius) {
                    bulletShootingFrequency = std::max(100, bulletShootingFrequency - 100);
                    bulletSpeedPowerUpVisible = false;
                    break;
                }
            }
        }


        if (powerUpVisible) {
            speedPowerUp.draw();
            speedPowerUp.move(SPEED);
            if (speedPowerUp.y > windowHeight) {
                powerUpVisible = false;
                powerUpTimer = 0;
            }
        }

        if (bulletSpeedPowerUpVisible) {
            bulletSpeedPowerUp.draw();
            bulletSpeedPowerUp.move(SPEED);
            if (bulletSpeedPowerUp.y > windowHeight) {
                bulletSpeedPowerUpVisible = false;
            }
        }

        if (!powerUpVisible) {
            powerUpTimer += deltaTime;
        }

        int speedupflag = false;
        for (auto& soldier : soldiers) {
            if (powerUpVisible && abs(soldier.x - speedPowerUp.x) < soldier.size && abs(soldier.y - speedPowerUp.y) < speedPowerUp.radius) {
                speedupflag = true;
                powerUpVisible = false;
                powerUpTimer = 0;
            }
        }

        if (speedupflag == true) {
            globalSpeedMultiplier *= 1.25;
            if (globalSpeedMultiplier > 4) {
                globalSpeedMultiplier = 4;
            }
        }

        for (auto& soldier : soldiers) {
            soldier.setToGlobalSpeed();
        }

        // Check for collisions between bullets and enemies
        for (auto it = bullets.begin(); it != bullets.end(); ) {
            // Initialize a flag to check if a bullet hits an enemy
            bool isHit = false;
            // Iterate over all enemies
            for (auto jt = enemies.begin(); jt != enemies.end(); ) {
                // Check if the distance between the bullet and the enemy is less than the sum of their radii (collision detection)
                if (abs(it->x - jt->x) < it->radius + jt->radius && abs(it->y - jt->y) < it->radius + jt->radius) {
                    // If a collision is detected, set the hit flag to true and remove the enemy
                    isHit = true;
                    jt = enemies.erase(jt);

                    player.PlayOneShot(hitSound);
                    enemiesDefeated++;
                }
                else {
                    // If no collision is detected, move to the next enemy
                    ++jt;
                }
            }
            // If a collision is detected, remove the bullet
            if (isHit) {
                it = bullets.erase(it);
            }
            else {
                // If no collision is detected, move to the next bullet
                ++it;
            }
        }

        // Check for collisions between bullets and obstacles
        for (auto it = bullets.begin(); it != bullets.end(); ) {
            // Initialize a flag to check if a bullet hits a obstacles
            bool isHit = false;
            // Iterate over all obstacles
            for (auto& obstacle : obstacles) {
                if (obstacle.life <= 0)
                {
                    continue;
                }
                // Check if the bullet is within the boundaries of the wall (collision detection)
                if (it->y <= obstacle.y + obstacle.halfside && it->x >= obstacle.x - obstacle.halfside &&
                    it->x <= obstacle.x + obstacle.halfside && it->y >= obstacle.y - obstacle.halfside) {
                    // If a collision is detected, set the hit flag to true and break the loop
                    isHit = true;
                    obstacle.life--;
                    if (obstacle.life == 0)
                    {
                        obstacle.life = -1;
                        bulletShootingFrequency = bulletShootingFrequency - 200;
                    }
                    break;
                }
            }
            // If a collision is detected, remove the bullet
            if (isHit) {
                it = bullets.erase(it);
            }
            // If no collision is detected, move to the next bullet
            else {
                ++it;
            }
        }

        // Check for collisions between bullets and debuffobstacles
        for (auto it = bullets.begin(); it != bullets.end(); ) {
            // Initialize a flag to check if a bullet hits a debuffobstacles
            bool isHit = false;
            // Iterate over all debuffobstacles
            for (auto& debuffobstacle : debuffobstacles) {
                if (debuffobstacle.life <= 0)
                {
                    continue;
                }
                // Check if the bullet is within the boundaries of the wall (collision detection)
                if (it->y <= debuffobstacle.y + debuffobstacle.halfside && it->x >= debuffobstacle.x - debuffobstacle.halfside &&
                    it->x <= debuffobstacle.x + debuffobstacle.halfside && it->y >= debuffobstacle.y - debuffobstacle.halfside) {
                    // If a collision is detected, set the hit flag to true and break the loop
                    isHit = true;
                    debuffobstacle.life--;
                    if (debuffobstacle.life == 0)
                    {
                        debuffobstacle.life = -1;
                        bulletShootingFrequency = bulletShootingFrequency + 200;

                    }
                    break;
                }
            }
            // If a collision is detected, remove the bullet
            if (isHit) {
                it = bullets.erase(it);
            }
            // If no collision is detected, move to the next bullet
            else {
                ++it;
            }
        }

        // Check for collisions between soldiers and obstacles
        for (auto it = soldiers.begin(); it != soldiers.end(); ) {
            bool isHit = false;
            std::vector<Enemy>::iterator hitObstacle;

            for (auto jt = obstacles.begin(); jt != obstacles.end(); ++jt) {
                if (abs(it->x - jt->x) < it->size && abs(it->y - jt->y) < jt->halfside) {
                    isHit = true;
                    break;
                }
            }
            for (auto jt = debuffobstacles.begin(); jt != debuffobstacles.end(); ++jt) {
                if (abs(it->x - jt->x) < it->size && abs(it->y - jt->y) < jt->halfside) {
                    isHit = true;
                    break;
                }
            }
            if (isHit) {
                it = soldiers.erase(it);
            }
            else {
                ++it;
            }
        }

        // Check for collisions between bullets and walls
        for (auto it = bullets.begin(); it != bullets.end(); ) {
            // Initialize a flag to check if a bullet hits a wall
            bool isHit = false;
            // Iterate over all walls
            for (auto& wall : walls) {
                // Check if the bullet is within the boundaries of the wall (collision detection)
                if (it->y <= wall.y1 && it->x >= wall.x1 && it->x <= wall.x2) {
                    // If a collision is detected, set the hit flag to true and break the loop
                    isHit = true;
                    break;
                }
            }
            // If a collision is detected, remove the bullet
            if (isHit) {
                it = bullets.erase(it);
            }
            // If no collision is detected, move to the next bullet
            else {
                ++it;
            }
        }

        // Check if soldier passed a wall
        for (auto& wall : walls) {
            // Check if the soldier has passed the y-coordinate of a wall and start the enemies moving
            if (!wall.isPassed && soldiers.size() > 0 && soldiers[0].y <= wall.y1) {
                wall.isPassed = true; // Mark the wall as passed
                lastPassedWallId = wall.wallId; // Update the ID of the last passed wall

                // Set the enemies associated with the passed wall to start moving
                for (auto& enemy : enemies) {
                    if (enemy.wallId == wall.wallId) {
                        enemy.isMoving = true;
                    }
                }
            }

            // Check if the soldier has passed through a wall and perform the wall's operation
            if (wall.isPassed && !wall.operationPerformed && soldiers.size() > 0 && soldiers[0].y <= wall.y1 && soldiers[0].x >= wall.x1 && soldiers[0].x <= wall.x2) {
                int numSoldiers = soldiers.size();
                int newNumSoldiers = wall.performOperation(numSoldiers);

                // Add new soldiers if the number of soldiers increased
                while (soldiers.size() < newNumSoldiers) {
                    int startX;
                    if (soldiers.back().x + SOLDIER_SIZE + 1 <= rightBoundary) { // If there's room on the right
                        startX = soldiers.back().x + SOLDIER_SIZE + 1; // Add new soldiers on the right side of the existing soldiers
                    }
                    else { // If there's no room on the right
                        startX = soldiers.front().x - SOLDIER_SIZE - 1; // Add new soldiers on the left side of the existing soldiers
                    }

                    // Ensure the new soldier is within the road boundaries
                    if (startX >= leftBoundary && startX + SOLDIER_SIZE <= rightBoundary) {
                        soldiers.push_back(Soldier(startX, soldiers[0].y, SOLDIER_SIZE));
                    }
                    else {
                        break; // If there's no room to add new soldiers, break the loop
                    }
                }

                // Remove soldiers if the number of soldiers decreased
                while (soldiers.size() > newNumSoldiers && soldiers.size() > 1) {
                    soldiers.pop_back();
                }

                wall.operationPerformed = true; // Mark the operation as performed
            }

            // Check if the soldier has completely passed a wall and reset the operationPerformed flag
            if (wall.isPassed && wall.operationPerformed && soldiers.size() > 0 && soldiers[0].y + soldiers[0].size < wall.y1) {
                wall.operationPerformed = false; // Allow the operation to be performed again when they pass the next wall
            }
        }

        // Check if a wall has moved past the end of the window
        if (walls.back().y1 > WALL_GAP) {
            generateSet(walls, enemies, obstacles,debuffobstacles, 0, setId++);
        }

        // Remove enemies that have moved past the end of the window
        for (auto it = enemies.begin(); it != enemies.end(); ) {
            if (it->y > windowHeight) {
                it = enemies.erase(it);
            }
            else {
                ++it;
            }
        }

        // Remove walls that have moved past the end of the window
        for (auto it = walls.begin(); it != walls.end(); ) {
            if (it->y1 > windowHeight) {
                it = walls.erase(it);
            }
            else {
                ++it;
            }
        }


        // Remove enemies that have moved past the end of the window
        for (auto it = enemies.begin(); it != enemies.end(); ) {
            if (it->y > windowHeight) {
                it = enemies.erase(it);
            }
            else {
                ++it;
            }
        }

        // Remove walls that have moved past the end of the window
        for (auto it = walls.begin(); it != walls.end(); ) {
            if (it->y1 > windowHeight) {
                it = walls.erase(it);
            }
            else {
                ++it;
            }
        }

        numSoldiers = soldiers.size();
        glColor3ub(255, 0, 255);
        glRasterPos2i(10, 20);
        char soldierStr[256];
        sprintf(soldierStr, "Soldier: %d", numSoldiers);
        YsGlDrawFontBitmap8x12(soldierStr);

        glRasterPos2i(10, 40);
        char enemyStr[256];
        sprintf(enemyStr, "Enemy defeated: %d", enemiesDefeated);
        YsGlDrawFontBitmap8x12(enemyStr);
        glColor3ub(0, 0, 0);

        if (soldiers.empty()) {
            gameEnded = true;
            // break; // Exit the game
        }

        FsSwapBuffers();
        FsSleep(25);
    }
    player.End();
    return 0;
}