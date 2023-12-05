# Wall Warriors Game
### Game Description

The game is a simple 2D shooter where the player controls a group of soldiers moving forward on a road, fighting against enemies and overcoming walls with special properties.
![Demo](demo_video.gif)
### Gameplay 

The game gives the illusion of the soldiers moving forward by having the road, enemies, and walls automatically move downwards. The soldiers can move left or right on the road, but they stay at the same vertical position. This creates the effect of the soldiers advancing, while in reality, the environment (road, enemies, and walls) is moving towards them. The soldiers can shoot bullets upwards at a frequency that can be set at the beginning of the game.

Enemies appear at random positions in the upper half of the road and chase the soldiers. If an enemy collides with a soldier, the soldier is removed from the game.

Walls appear on the road and move downwards. Each wall has a special operation associated with it (add, subtract, multiply, or divide). When a soldier hits a wall, the operation is performed on the number of soldiers. For example, if the operation is "add", two soldiers are added. If the operation is "subtract", two soldiers are removed, but there will always be at least one soldier left.

### Game Logic

1. Initialization: The game window is initialized with a specific width and height. A soldier is created at the center bottom of the window. Infinite set of walls and enemies are generated at a certain height in the window.

2. Game Loop: The game runs in a loop until the ESC key is pressed or there are no soldiers left. In each iteration of the loop, the following actions are performed:

- Drawing: The road, soldiers, bullets, enemies, and walls are drawn on the screen.

- User Input: The game checks for user input. If the left or right arrow key is pressed, the soldier is moved to the left or right respectively. If no key is pressed, the soldier keeps moving upwards.

- Shooting: Every certain number of milliseconds (defined by SHOOTING_FREQUENCY), the soldier shoots a number of bullets equal to the number of soldiers. The bullets are shot at different angles.

- Moving: The bullets, soldiers, enemies, and walls are moved. The bullets move according to their angle, the soldiers move according to the user input, the enemies move downwards and towards the soldier if they have passed a wall, and the walls move downwards.

- Collision Detection: The game checks for collisions between soldiers and enemies, bullets and enemies, and bullets and walls. If a collision is detected, the corresponding soldier, bullet, or enemy is removed.

- Wall Passing: The game checks if a soldier has passed a wall. If a soldier has passed a wall, the wall's operation is performed on the number of soldiers, which can increase or decrease the number of soldiers. If a soldier has completely passed a wall, the wall's operation can be performed again when the soldier passes the next wall.

- Set Generation: If a wall has moved past the end of the window, a new set of walls and enemies is generated at the top of the window.

- Removal: Enemies and walls that have moved past the end of the window are removed.

### Controls

- Left Arrow: Move soldiers to the left
- Right Arrow: Move soldiers to the right
- ESC: Exit the game

### Objective

The objective of the game is to move forward as far as possible, overcome as many walls as possible, and defeat as many enemies as possible. The player needs to strategically control the soldiers to avoid enemies and hit walls at the right time to maximize the number of soldiers.
