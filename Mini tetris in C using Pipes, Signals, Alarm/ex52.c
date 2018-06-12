// Raz Shenkman
// 311130777


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>

#define SIZE 20

// Status of the game piece.
typedef enum {
    HORIZONTAL, VERTICAL
} status;

// Enum of the command
typedef enum {
    LEFT, RIGHT, DOWN, FLIP
} update;

// Game piece
typedef struct GamePiece {
    status stat;
    int x;
    int y;
} GamePiece;

// Global board and game piece
char gameBoard[SIZE][SIZE];
GamePiece globalPlayer;

/**
 * This method clears the board.
 */
void clearBoard() {
    int i, j;
    i = 0;
    for (i; i < SIZE; i++) {
        j = 0;
        for (j; j < SIZE; j++) {
            if (gameBoard[i][j] == '-') {
                gameBoard[i][j] = ' ';
            }
        }
    }
}

/**
 * This method initializes the board frame.
 */
void initializeBoard() {
    int i, j;
    i = 0;
    for (i; i < SIZE; i++) {
        j = 0;
        for (j; j < SIZE; j++) {
            gameBoard[i][j] = ' ';
            if ((j == 0) || (j == SIZE - 1)) {
                gameBoard[i][j] = '*';
            }
            if (i == SIZE - 1) {
                gameBoard[i][j] = '*';
            }
        }
    }
}

/**
 * This method prints the board.
 */
void printBoard() {
    system("clear");
    int i, j;
    i = 0;
    for (i; i < SIZE; i++) {
        j = 0;
        for (j; j < SIZE; j++) {
            // If game piece is horizontal print
            if (globalPlayer.stat == HORIZONTAL) {
                if (i == globalPlayer.x) {
                    if (j == globalPlayer.y - 1 || j == globalPlayer.y) {
                        printf("-");
                    } else if (j == globalPlayer.y + 1) {
                        printf("-");
                        // if the board square doesn't contain a player
                    } else {
                        printf("%c", gameBoard[i][j]);
                    }
                } else {
                    printf("%c", gameBoard[i][j]);
                }
                // if game piece is vertical print
            } else if (globalPlayer.stat == VERTICAL) {
                if (j == globalPlayer.y) {
                    if (i == globalPlayer.x - 1 || i == globalPlayer.x) {
                        printf("-");
                    } else if (i == globalPlayer.x + 1) {
                        printf("-");
                    } else {
                        printf("%c", gameBoard[i][j]);
                    }
                    // if the board square doesn't contain a player
                } else {
                    printf("%c", gameBoard[i][j]);
                }
            }
            // if we reached the end of the line
            if (j == SIZE - 1) {
                printf("\n");
            }
        }
    }
}

/**
 * This method gets input and changes the player status according to it.
 * @param input (enum) for action.
 */
void updatePlayer(update input) {
    switch (input) {
        // game piece 1 square down
        case DOWN:
            if (globalPlayer.stat == HORIZONTAL) {
                // case we reached the end, initialize new game piece
                if (globalPlayer.x == SIZE - 2) {
                    globalPlayer.x = 0;
                    globalPlayer.y = SIZE / 2;
                } else {
                    globalPlayer.x += 1;
                }
            } else {
                if (globalPlayer.x + 1 == SIZE - 2) {
                    // case we reached the end, initialize new game piece
                    globalPlayer.stat = HORIZONTAL;
                    globalPlayer.x = 0;
                    globalPlayer.y = SIZE / 2;
                } else {
                    globalPlayer.x += 1;
                }
            }
            break;
        case LEFT:
            // left border while horizontal
            if (globalPlayer.stat == HORIZONTAL) {
                if (globalPlayer.y - 1 != 1) {
                    globalPlayer.y -= 1;
                }
                // left border while vertical
            } else {
                if (globalPlayer.y != 1) {
                    globalPlayer.y -= 1;
                }
            }
            break;
        case RIGHT:
            // right border while horizontal
            if (globalPlayer.stat == HORIZONTAL) {
                if (globalPlayer.y + 1 != SIZE - 2) {
                    globalPlayer.y += 1;
                }
                // right border while vertical
            } else {
                if (globalPlayer.y != SIZE - 2) {
                    globalPlayer.y += 1;
                }
            }
            break;
        case FLIP:
            // if we are horizontal flip
            if (globalPlayer.stat == HORIZONTAL) {
                // check if we are in the start or in the end
                if (globalPlayer.x == 0 || globalPlayer.x >= SIZE - 2) { break; }
                globalPlayer.stat = VERTICAL;
            } else {
                // check if we are in the right/left borders
                if (globalPlayer.y <= 1 || globalPlayer.y >= SIZE - 2) { break; }
                globalPlayer.stat = HORIZONTAL;
            }
            break;
    }
    printBoard();
}

/**
 * This method activated each time SIGUSR2 has been sent to this process.
 * @param sig
 */
void signal_hand(int sig) {
    signal(SIGUSR2, signal_hand);
    // read from pipe what sent
    char input = (char) getchar();
    // change the game piece for a,d,s,w
    switch (input) {
        case 's':
            updatePlayer(DOWN);
            break;
        case 'w':
            updatePlayer(FLIP);
            break;
        case 'd':
            updatePlayer(RIGHT);
            break;
        case 'a':
            updatePlayer(LEFT);
            break;
            // for q, close
        case 'q':
            exit(0);
        default:
            break;
    }
    printBoard();
}

/**
 * This method activated each time SIGALRM has been sent to this process.
 * @param sig
 */
void alarm_hand(int sig) {
    signal(SIGALRM, alarm_hand);
    updatePlayer(DOWN);
    alarm(1);
}

/**
 * The main initializes the signal handlers a board and a game piece. also creates an alarm and pause (wait for a
 * signal constantly).
 * @return 0
 */
int main() {
    signal(SIGUSR2, signal_hand);
    signal(SIGALRM, alarm_hand);
    globalPlayer.x = 0;
    globalPlayer.y = SIZE / 2;
    globalPlayer.stat = HORIZONTAL;

    initializeBoard();
    printBoard();
    clearBoard();
    alarm(1);
    while (1) {
        pause();
    }
}