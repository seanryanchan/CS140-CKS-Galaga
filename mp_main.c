#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <ncurses.h>
#include <unistd.h>

//DEFINITIONS
#define MAX_THREADS 2048
#define SHIPRATE 50000
#define BULLETRATE 100000 // goes up the screen 10 times a second.
#define REFRESH_RATE 10000 // refreshes screens 100 times a second, via doupdate()
#define BUGRATE 100000
#define DELAY 50000
#define BUGS_ROW 4
#define BUGS_COL 8
#define LEFT -1
#define RIGHT 1
#define ALIVE 1
#define DEAD 0
#define WIN 1
#define LOSE 0
#define RUNNING -1


//DECLARATIONS
pthread_mutex_t gameLock;
pthread_mutex_t ex;

typedef struct Node {
    int bugx;
    int bugy;
    int status;
    struct Node *next;
    char display;
} Node;

typedef struct windowData {//data type that stores all the game data
    int bulX, bulY;
    int shipX, shipY;
    Node bugs[10];//we need to declare their initial coordinates
    int directionBug;
    int maxX;
    int maxY;
    int bulletThread; // for generation of multiple bullets
    pthread_t threads[MAX_THREADS];
    // This contains the threads of the game. Threads 10 to 2047 are reserved for bullets.
    WINDOW *scr; // points to the window
    int bugCount;
    int winState;
} windowData;

void drawShip(windowData *game);
void drawBullet(int bulY, int bulX);
void drawBug(windowData *game);

void eraseSingle(int Y, int X);
void eraseShip(int Y, int X);
void eraseBullet(int Y, int X);
void eraseBugs(windowData *game);

void moveBug(windowData *game);
int killBug(windowData *game, int bulY, int bulX);

void shipThread(windowData *game);
void swarmThread(windowData *game);
void screenThread(windowData *game);
void fireBullet(windowData *game);

void loseCondition(windowData *game);

// ASCII DRAWINGS
// Ship is drawn top to bottom.
char *shipHead = " ^ ";
char *shipBody = "{O}";
// bullet is drawn top to bottom. bulY is the top.
char *bulletHead = " ^ ";
char *bulletBody = " | ";
char *bulletTail = "O O";
// bugHead
char *bugHead = "@";

char *replace = " ";
//START OF CODE
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
    windowData game; //game is the variable that has the ship's location, the bug's location
    getmaxyx(stdscr, game.maxY, game.maxX);
    game.scr = stdscr;
    game.bulletThread = 1; // threads 10 to MAX_THREADS - 1 are for bullets

    // ship and bullet initial locations
    game.bulY = -5;
    game.bulX = -5;
    game.shipX = game.maxX / 2;
    game.shipY = game.maxY - 3;

    // initial swarm coordinate declaration
    game.directionBug = RIGHT;
    game.bugCount = 10;

    // GAME STATE
    game.winState = RUNNING;

    // BUG SWARM POPULATION
    for(int i = 0; i < 10; i++) {
        game.bugs[i].bugx = i * 3;
        game.bugs[i].bugy = 1;
        game.bugs[i].display = '@';
        game.bugs[i].status = ALIVE; //all alive
    }

    pthread_create(&(game.threads[0]), NULL, (void *)shipThread, (void *)&game);

    //create swarm -> swarm is spawned and controlled by thread 1.
    pthread_create(&(game.threads[1]), NULL, (void *)swarmThread, (void *)&game);

    pthread_create(&(game.threads[2]), NULL, (void *)screenThread, (void *)&game);

    int ch;
    while(true) {
        // win evaluator
        if(game.bugCount <= 0)
            game.winState = WIN;

        loseCondition(&game);

        switch (game.winState) {
            case WIN:
                erase();
                while(true){
                    mvprintw(game.maxY/2,game.maxX/2, "You won!");
                    refresh();
                }
                break;
            case LOSE:
                erase();
                while(true){
                    mvprintw(game.maxY/2,game.maxX/2, "You have lost the game!");
                    mvprintw((game.maxY/2 )+ 1,game.maxX/2, "Try again!");
                    refresh();
                }
                break;
        }

        ch = getch();

        if('f' == ch) {// The character to fire a bullet
            pthread_mutex_lock(&gameLock);
            game.bulX = game.shipX;
            game.bulY = game.shipY-3;
            // bullet threads -> handled by threads 10 to 2047.
            pthread_create(&(game.threads[game.bulletThread]), NULL, (void *)fireBullet, (void *)&game);
            pthread_mutex_unlock(&gameLock);
            if (game.bulletThread == (MAX_THREADS - 1)){
                pthread_mutex_lock(&gameLock);
                game.bulletThread = 10;
                pthread_mutex_unlock(&gameLock);
            } else{
                pthread_mutex_lock(&gameLock);
                game.bulletThread += 1;
                pthread_mutex_unlock(&gameLock);
            }
        } else if (KEY_UP == ch) {
            if(game.shipY > 0){
                pthread_mutex_lock(&gameLock);
                game.shipY -=1;
                pthread_mutex_unlock(&gameLock);
            }
        } else if (KEY_DOWN == ch) {
            if(game.shipY < (game.maxY - 2)){
                pthread_mutex_lock(&gameLock);
                game.shipY +=1;
                pthread_mutex_unlock(&gameLock);
            }
        } else if (KEY_LEFT == ch) {
            if(game.shipX > 0){
                pthread_mutex_lock(&gameLock);
                game.shipX -=1;
                pthread_mutex_unlock(&gameLock);
            }
        } else if (KEY_RIGHT == ch) {
            if(game.shipX < game.maxX - 3){
                pthread_mutex_lock(&gameLock);
                game.shipX +=1;
                pthread_mutex_unlock(&gameLock);
            }
        }

        ch = '\0';
    }

    endwin();
    return 0;
}

void shipThread(windowData *game){
    int oldY;
    int oldX;
    while(true){

        pthread_mutex_lock(&gameLock);
        drawShip(game);
        oldY = game->shipY;
        oldX = game->shipX;
        pthread_mutex_unlock(&gameLock);

        usleep(REFRESH_RATE);

        pthread_mutex_lock(&gameLock);
        eraseShip(oldY, oldX);
        pthread_mutex_unlock(&gameLock);

        switch (game->winState){
            case WIN:
                eraseShip(oldY, oldX);
                pthread_exit((void *)0);
                return;
            case LOSE:
                eraseShip(oldY, oldX);
                pthread_exit((void *)0);
                return;

        }

    }
    pthread_exit((void*) 0);
}

void drawShip(windowData *game){
    mvprintw(game->shipY, game->shipX, shipHead);
    mvprintw(game->shipY +1, game->shipX, shipBody);
}

void drawBullet(int bulY, int bulX){
    mvprintw(bulY, bulX, bulletHead);
    mvprintw(bulY + 1, bulX, bulletBody);
    mvprintw(bulY + 2, bulX, bulletTail);
}

void fireBullet(windowData *game){
    int err;
    int bulX = game->shipX;
    int bulY = game->shipY - 3;
    int bulmaxY = -3;
    pthread_join(game->threads[2], (void *) 0);
    // join all bullets with the ship thread.
    int deadbug = 0;
    while(true){
        pthread_mutex_lock(&gameLock);
        drawBullet(bulY, bulX);
        deadbug = killBug(game, bulY, bulX);
        pthread_mutex_unlock(&gameLock);

        usleep(BULLETRATE);

        pthread_mutex_lock(&gameLock);
        eraseBullet(bulY, bulX);
        pthread_mutex_unlock(&gameLock);

        if(bulY <= bulmaxY){
            pthread_mutex_lock(&gameLock);
            eraseBullet(bulY, bulX);
            pthread_mutex_unlock(&gameLock);
            pthread_exit((void *) 0);
            return;
        }
        if(deadbug){
            pthread_mutex_lock(&gameLock);
            eraseBullet(bulY, bulX);
            pthread_mutex_unlock(&gameLock);
            pthread_exit((void *) 0);
            return;
        }
        if(game->winState == WIN || game->winState == LOSE){
            eraseBullet(bulY, bulX);
            break;
        }
        bulY -= 1;
    }
    pthread_exit((void *) 0);
    return;
}

void swarmThread(windowData *game) {
    // pthread_join(game->threads[2], (void *) 0);

    while(true) {
        pthread_mutex_lock(&gameLock);
        drawBug(game);
        pthread_mutex_unlock(&gameLock);
        usleep(BUGRATE);
        pthread_mutex_lock(&gameLock);
        eraseBugs(game);
        moveBug(game);
        pthread_mutex_unlock(&gameLock);

        if(game->winState == WIN || game->winState == LOSE){
            erase();
            break;
        }
    }
    pthread_exit((void *) 0);
}

void drawBug(windowData *game) {
    int bugCountTemp = 0;
    for(int number = 0; number < 10; number++) {
        if(game->bugs[number].status == ALIVE) {
            bugCountTemp++;
            mvprintw(game->bugs[number].bugy, game->bugs[number].bugx, &(game->bugs[number].display));
        }
    }
    game->bugCount = bugCountTemp;
    //pwede ba to kasi
}

void moveBug(windowData *game) {

    for(int i = 0; i < 10; i++){
        if(game->directionBug == RIGHT){ // move to the right
            game->bugs[i].bugx= game->bugs[i].bugx + 1;
        }
        else if(game->directionBug == LEFT){ // move to the left
            game->bugs[i].bugx= game->bugs[i].bugx - 1;
        }

        //checking if it is in the end
        if((game->bugs[i].bugx == game->maxX || game->bugs[i].bugx == 0)&&(game->bugs[i].status == ALIVE)){
            game->directionBug = game->directionBug*(-1);
            //when changing direction, it goes down
            for(int j = 0 ; j < 10; j++){
                game->bugs[j].bugy = game->bugs[j].bugy + 1;
            }
        }
    }
}

int killBug(windowData *game, int bulY, int bulX) {
    for(int i = 0; i < 10; i++) {
        if((game->bugs[i].bugx == bulX) && (game->bugs[i].bugy >= bulY)){
            game->bugs[i].status = DEAD; //bug has died x_x
            game->bugs[i].display = '\0';
            return 1; //bullet collided with bug
        }
    }
    return 0;
}

void eraseBugs(windowData *game) {
    for(int number = 0; number < 10; number++) {
        eraseSingle(game->bugs[number].bugy, game->bugs[number].bugx);
    }
}


//when bug hits ship, kill bug

void eraseSingle(int Y, int X){
    mvprintw(Y, X, replace);
}

void eraseShip(int Y, int X){
    // ship is a 3x2 object
    mvprintw(Y, X, replace);
    mvprintw(Y, X+1, replace);
    mvprintw(Y, X+2, replace);
    mvprintw(Y+1, X, replace);
    mvprintw(Y+1, X+1, replace);
    mvprintw(Y+1, X+2, replace);
}

void eraseBullet(int Y, int X){
    // bullet is a 3x3 object
    mvprintw(Y, X, replace);
    mvprintw(Y, X+1, replace);
    mvprintw(Y, X+2, replace);
    mvprintw(Y+1, X, replace);
    mvprintw(Y+1, X+1, replace);
    mvprintw(Y+1, X+2, replace);
    mvprintw(Y+2, X, replace);
    mvprintw(Y+2, X+1, replace);
    mvprintw(Y+2, X+2, replace);
}

void screenThread(windowData *game){
    while(true){
        pthread_mutex_lock(&gameLock);
        wnoutrefresh(stdscr);
        doupdate();
        pthread_mutex_unlock(&gameLock);

        usleep(REFRESH_RATE);

    }
}

void loseCondition(windowData *game) {
    for(int i = 0; i < 10; i++) {
        if((game->bugs[i].status) == ALIVE && (game->bugs[i].bugy >= game->maxY)) {
            game->winState = LOSE;
        }
    }
}
