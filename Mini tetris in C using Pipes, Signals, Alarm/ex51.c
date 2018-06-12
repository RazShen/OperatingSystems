// Raz Shenkman
// 311130777

#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>

#define ERRORWRITE "Error in system call\n"

/**
 * This method reads a char pressed by the user on the keyboard.
 * @return char.
 */
char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
}

/**
 * Writes error to stderr.
 */
void writeErrorToStderr() {
    write(STDERR_FILENO, ERRORWRITE, strlen(ERRORWRITE));
    exit(-1);
}

/**
 * Check if pressed key is one of the keys to pass to the pipe.
 * @param ch char to check
 * @return if should pass the char to the pipe.
 */
int validKey(char ch) {
    switch (ch) {
        case 'w':
        case 'd':
        case 'q':
        case 'a':
        case 's':
            return 1;
        default:
            return 0;
    }
}

/**
 * Main of the program, creates a child which runs draw.out
 * @return
 */
int main() {
    int Pipe[2];
    // initialize the pipe.
    pipe(Pipe);
    int childPID;

    if ((childPID = fork()) < 0) {
        // error
        writeErrorToStderr();
    }

    if (childPID == 0) {
        // we're in child
        //close(0);
        dup2(Pipe[0], 0);
        char *args[32] = {"./draw.out", NULL};

        /* Execute our command */
        execvp(args[0], &args[0]);
        writeErrorToStderr();
    }
    // Constantly read from the keyboard and write to the pipe if char is one of w,a,s,d,q.
    while (1) {
        char input = getch();
        if (!validKey(input)) { continue; } // check if key is valid
        if (write(Pipe[1], &input, 1) < 0) { writeErrorToStderr(); } // write to pipe
        if (kill(childPID, SIGUSR2) < 0) { writeErrorToStderr(); } // send signal to child
        if (input == 'q') {
            break;
        }
    }
    return 0;
}