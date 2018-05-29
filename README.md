 //separate textfile
CS 140 SQRUV MP Group
Azcarraga, Jose Carlos Rodrigo J.   201508037   SQRUV
Chan, Sean Ryan S.                  201600012   SQRUV
Kopio, Katrina Mae D.               201601510   SQRUV

This is the readme for the 140 ME

Game Specs:

The game is called: GALAGA iPens made by the ACK group. The goal of the game is to destroy the enemy swarm, composed of bugs. The user will be able to control the ship with their arrow keys, and shoot out bullets to defend against the swarm.


Playing Instructions:
1. The player has a ship that can move from left, right, up down by pressing the corresponding arrow keys (LEFT,RIGHT,UP,DOWN).

2. The enemy has a swarm of bugs, and it shifts from left to right and goes down every time it goes to the end of each line. When the enemy line has forwarded to the lower end of the screen, the user loses.

3. To fire the special bullet, press 'F'.

4. Every time the bullet hits a bug, that bug dies. Once all bugs are dead, the player will win.



Running the Game:

Environment Specs:
    -Linux system
    -Linux posix threads library installed
    -Linux ncurses library installed

Execution Instructions
1. Make sure ncurses and pthreads is installed
2. In terminal open the game folder
3. Then fix permissions by typing:
    chmod +x runMP.sh
3. To open the game type:
    ./runMP.sh
