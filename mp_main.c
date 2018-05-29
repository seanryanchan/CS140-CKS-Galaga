#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <ncurses.h>
#include <unistd.h>
// #include "mp_bullet.c"
// #include "mp_ship.c"
// #include "helpers.h"

/*
atom://teletype/portal/41eac518-2e39-4494-985b-3b92f82d20a9
*/
// sean -> fix


#define MAX_THREADS 2048
#define SHIPRATE 50000
#define BULLETRATE 75000
#define REFRESH_RATE
#define BUG_ROW
#define BUG_COL

pthread_mutex_t gameLock;
pthread_mutex_t ex;

typedef struct winData {
	int bulX, bulY, shipX, shipY, _x, _y;
	int maxX;
	int maxY;
	int bulletThread;
	int dir2;
	WINDOW *scr;
    pthread_t threads[MAX_THREADS];
} winData;

typedef struct Node {
    char bug;
    int bugx;
    int bugy;
    struct Node *next;
} Node;

void drawShip(winData *game);
void drawBullet(int bulY, int bulX);
void fireBullet(winData *game);
void shipSpawn(winData *game);

// alternate ship

char *shipHead = " ^ ";
char *shipBody = "{O}";

char *bulletHead = " ^ ";
char *bulletBody = " | ";
char *bulletTail = "O O";
// this is my actual size x100


int main(int argc, char const *argv[])
{
	// setlocale(LC_ALL, **);
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, true);

	// hide the cursor
	curs_set(0);

	pthread_mutex_init(&gameLock, NULL);
	pthread_mutex_init(&ex, NULL);


	winData game;
	getmaxyx(stdscr, game.maxY, game.maxX);
	game.bulY = -5;
	game.bulX = -5;
	game.shipX = game.maxX / 2;
	game.shipY = game.maxY / 2;
	game._y = game._x = 0;
	game.bulletThread = 1;  // threads 10 to MAX_THREADS - 1 are for bullets
	game.scr = stdscr;



	// optional window partitioning
	// WINDOW * win = newwin(game.maxY, game.maxX, 0, 0);

	pthread_create(&(game.threads[0]), NULL, (void *)shipSpawn, (void *)&game);


    int ch;
	while(true){
		ch = getch();

		if('f' == ch){
			game.bulX = game.shipX;
			game.bulY = game.shipY-3;
			pthread_create(&(game.threads[game.bulletThread]), NULL, (void *)fireBullet, (void *)&game);
            if (game.bulletThread == MAX_THREADS -1)
                game.bulletThread = 10;
            else
                game.bulletThread += 1;
		} else if (KEY_UP == ch){
			if(game.shipY > 0)
				game.shipY -=1;
		} else if (KEY_DOWN == ch){
			if(game.shipY < (game.maxY - 2))
				game.shipY +=1;
		} else if (KEY_LEFT == ch){
			if(game.shipX > 0)
				game.shipX -=1;
		}else if (KEY_RIGHT == ch){
			if(game.shipX < game.maxX - 3)
				game.shipX +=1;
		}

        ch = '\0';

		// mvprintw(0, game.b1x, "O");
		// mvprintw(game.b2y, 0, "X");
	}

	// getch();
	endwin();
	return 0;
}


void drawShip(winData *game){
	mvprintw(game -> shipY, game -> shipX, shipHead);
	mvprintw(game -> shipY +1, game -> shipX, shipBody);
}

void drawBullet(int bulY, int bulX){
	mvprintw(bulY, bulX, bulletHead);
	mvprintw(bulY + 1, bulX, bulletBody);
	mvprintw(bulY + 2, bulX, bulletTail);
}

void drawBug(currentY, currentX) {
	int y = currentY;
	int x = currentX;

	for (int i = 0; i < SWARM_ROWS; i++) {
		for (int j = 0; j < SWARM_COLS; j++) {
			if (swarm[i][j])
				mvprintw(topleft_y+i, topleft_x+j, SWARM_CHAR);
		}
		printf("\n");
	}
}

void shipSpawn(winData *game){
	while(true){
		pthread_mutex_lock(&gameLock);
		drawShip(game);
        // drawBullet(game);
		refresh();
		pthread_mutex_unlock(&gameLock);

		usleep(SHIPRATE);

		pthread_mutex_lock(&gameLock);
		erase();
		pthread_mutex_unlock(&gameLock);

        pthread_join(game -> threads[game -> bulletThread], (void *) 0);
        // This thread is stopped temporarily if there is a bullet fired.
	}
	pthread_exit((void *) 0);
}

void fireBullet(winData *game){
	int err;

    int bulX = game -> shipX;
    int bulY = game -> shipY - 3;

	while(true){
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
		// 	break;
		// if (game -> bulY <= -3)
        //     break;

        bulY -= 2;

		if(bulY == game -> _y)
			break;
		if (bulY <= -3)
            break;
	}

    // game -> bulY = -5;
    // game -> bulX = -5;
	pthread_exit((void *) 0);
}

void swarmThread(winData *game) {

}
