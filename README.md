# Wall_Warriors_Game
**Game Description**

The game is a simple 2D shooter where the player controls a group of soldiers moving forward on a road, fighting against enemies and overcoming walls with special properties.
Gameplay

The game gives the illusion of the soldiers moving forward by having the road, enemies, and walls automatically move downwards. The soldiers can move left or right on the road, but they stay at the same vertical position. This creates the effect of the soldiers advancing, while in reality, the environment (road, enemies, and walls) is moving towards them. The soldiers can shoot bullets upwards at a frequency that can be set at the beginning of the game.

Enemies appear at random positions in the upper half of the road and chase the soldiers. If an enemy collides with a soldier, the soldier is removed from the game.

Walls appear on the road and move downwards. Each wall has a special operation associated with it (add, subtract, multiply, or divide). When a soldier hits a wall, the operation is performed on the number of soldiers. For example, if the operation is "add", two soldiers are added. If the operation is "subtract", two soldiers are removed, but there will always be at least one soldier left.

The game ends when the player presses the ESC key.

**Controls**
- Left Arrow: Move soldiers to the left
- Right Arrow: Move soldiers to the right
- Space Bar: Start the game

**Objective**
The objective of the game is to move forward as far as possible, overcome as many walls as possible, and defeat as many enemies as possible. The player needs to strategically control the soldiers to avoid enemies and hit walls at the right time to maximize the number of soldiers.
