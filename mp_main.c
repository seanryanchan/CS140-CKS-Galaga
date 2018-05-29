#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <ncurses.h>
#include <unistd.h>

/*
   atom://teletype/portal/41eac518-2e39-4494-985b-3b92f82d20a9
 */

#define MAX_THREADS 2048
#define SHIPRATE 50000
#define BULLETRATE 75000
#define REFRESH_RATE 10000 // refreshes screens 100 times a second, via doupdate()
#define DELAY 50000
#define BUGS_ROW 4
#define BUGS_COL 8
#define BUG_ONE "."
#define BUG_TWO "o"

pthread_mutex_t gameLock;
pthread_mutex_t ex;

typedef struct Node {
        int bugx;
        int bugy;
        int status;
        struct Node *next;
} Node;

typedef struct windowData {//data type that stores all the game data
        int bulX, bulY;
        int shipX, shipY;
        Node bugs[10];//we need to declare their initial coordinates
        int maxX;
        int maxY;
        int bulletThread; // for generation of multiple bullets
        pthread_t threads[MAX_THREADS];
        // This contains the threads of the game. Threads 10 to 2047 are reserved for bullets.
        WINDOW *scr; // points to the window
} windowData;

void drawShip(windowData *game);
void drawBullet(int bulY, int bulX);
void fireBullet(windowData *game);
void shipSpawn(windowData *game); //NOT USED

// Ship is drawn top to bottom.
char *shipHead = " ^ ";
char *shipBody = "{O}";
// bullet is drawn top to bottom. bulY is the top.
char *bulletHead = " ^ ";
char *bulletBody = " | ";
char *bulletTail = "O O";
// P.S. this is my actual size x100


int main(int argc, char const *argv[]) {
    // setlocale(LC_ALL, **);
    initscr();
    cbreak();
    noecho();
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, true);
    curs_set(0); // hide the cursor
    pthread_mutex_init(&gameLock, NULL);
    pthread_mutex_init(&ex, NULL);

//GAME DECLARATION
    windowData game; //game is the variable that has the ships location, the bullets location
    getmaxyx(stdscr, game.maxY, game.maxX);
    game.scr = stdscr;
    game.bulletThread = 1; // threads 10 to MAX_THREADS - 1 are for bullets

    // ship and bullet initial locations
    game.bulY = -5;
    game.bulX = -5;
    game.shipX = game.maxX / 2;
    game.shipY = game.maxY / 2;

/*
swarmdeclaration

for i < 10
game.bugs[i].bugsx = i
game.bugs[i].bu

*/


    pthread_create(&(game.threads[0]), NULL, (void *)shipSpawn, (void *)&game);
    // Ship is spawned and controlled by thread 0.
    int ch;
    while(true) {
        ch = getch();
        if('f' == ch) {
            game.bulX = game.shipX;
            game.bulY = game.shipY-3;
            pthread_create(&(game.threads[game.bulletThread]), NULL, (void *)fireBullet, (void *)&game);
            if (game.bulletThread == MAX_THREADS -1)
                game.bulletThread = 10;
            else
                game.bulletThread += 1;
        } else if (KEY_UP == ch) {
            if(game.shipY > 0)
                    game.shipY -=1;
        } else if (KEY_DOWN == ch) {
            if(game.shipY < (game.maxY - 2))
                    game.shipY +=1;
        } else if (KEY_LEFT == ch) {
            if(game.shipX > 0)
                game.shipX -=1;
        } else if (KEY_RIGHT == ch) {
            if(game.shipX < game.maxX - 3)
                game.shipX +=1;
        }
        ch = '\0';
    }

    endwin();
    return 0;
}

void keyboardListener(windowData *game){
    int ch;

}

//draws ship by
void drawShip(windowData *game){
    mvprintw(game->shipY, game->shipX, shipHead);
    mvprintw(game->shipY +1, game->shipX, shipBody);
}

void drawBullet(int bulY, int bulX){
    mvprintw(bulY, bulX, bulletHead);
    mvprintw(bulY + 1, bulX, bulletBody);
    mvprintw(bulY + 2, bulX, bulletTail);
}

void drawBug(int currentY, int currentX) {
    /* FOR PRINTING
    for bug_number < 10
        if bugs[bug_number].status == 1
            print (character "*", bugs[bug_number].bugx , bugs[bug_number].bugy)
    */

        /* int y = currentY;
           int x = currentX;

           for (int i = 0; i < BUG_ROW; i++) {
            for (int j = 0; j < BUG_COL; j++) {
                if (swarm[i][j]) {
                    mvprintw(currentY+i, currentX+j, BUG_ONE);
                }
                //if(fertilized), then print BUG_TWO
            }
            printf("\n");
           }*/
}

void shipSpawn(windowData *game){
    while(true) {
        pthread_mutex_lock(&gameLock);
        drawShip(game);
        // drawBullet(game);
        refresh();
        pthread_mutex_unlock(&gameLock);

        usleep(SHIPRATE);

        pthread_mutex_lock(&gameLock);
        erase();
        pthread_mutex_unlock(&gameLock);

        pthread_join(game->threads[game->bulletThread], (void *) 0);
        // This thread is stopped temporarily if there is a bullet fired.
    }
    pthread_exit((void *) 0);
}

void fireBullet(windowData *game){
    int err;
    int bulX = game->shipX;
    int bulY = game->shipY - 3;
    while(true) {
        pthread_mutex_lock(&gameLock);
        drawBullet(bulY, bulX);
        drawShip(game);
        refresh();
        pthread_mutex_unlock(&gameLock);

        usleep(SHIPRATE);

        pthread_mutex_lock(&gameLock);
        erase();
        pthread_mutex_unlock(&gameLock);

        // game -> bulY -= 2;
        //
        // if(game ->bulY == game -> _y)
        //  break;
        // if (game -> bulY <= -3)
        //     break;

        bulY -= 2;

        if (bulY <= -3)
                break;
    }

    // game -> bulY = -5;
    // game -> bulX = -5;
    pthread_exit((void *) 0);
}

void swarmThread(windowData *game) {

    while(true) {
        pthread_mutex_lock(&gameLock);
        //drawBug(game);
        //moveBug(game);
        //killBug(game);
        refresh();
        pthread_mutex_unlock(&gameLock);

        usleep(SHIPRATE);

        pthread_mutex_lock(&gameLock);
        erase();
        pthread_mutex_unlock(&gameLock);

        //pthread_join(game->threads[game->bulletThread], (void *) 0);
        // This thread is stopped temporarily if there is a bullet fired.
    }
    pthread_exit((void *) 0);
}

/*
    game pseudocode

    there are 10 bugs in one line

 **********

    //drawBug();
    //moveBug()
    //killBug();

    for simple movement of each bug
        move to right:
        for i < 10
            bugs[i].bugx = bugs[i].bugx + 1

        move to left:
        for i < 10
            bugs[i].bugx = bugs[i].bugx - 1

        move up:
        for i < 10
            bugs[i].bugy = bugs[i].bugy + 1

        move down:
        for i < 10
            bugs[i].bugy = bugs[i].bugy - 1



    for movement of the swarm
        how to change direction if it hit the end
            get coordinates of dulo right.x, compare to maxX
            get coordinates of dulo left.x, compare to minX


    for killing bugs
        do every time bullet moves
        for i < 10
            if (bugs[i].bugX == game.bulX && bugs[i].bugY == game.bulY)
                bugs[i].status = 0 DEATH
 */
