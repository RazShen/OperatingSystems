// Raz Shenkman
// 311130777

#include <stdio.h>
#include <fcntl.h>
#include <zconf.h>
#include <memory.h>
#include <ctype.h>

#define BYTE 1
#define ERRORBUFF 128
#define TRUE 1

int compareFiles(int fd1, int fd2);

int skippedChar(char buff);

void closeFD(int fd1, int fd2);

/**
 * Main function, open file descriptors and comparing them.
 * @param argc number or arguments
 * @param args arguments
 * @return -1 if error, 1 if not similar, 2 if similar, 3 if identical.
 */
int main(int argc, char *args[]) {

    char buff[ERRORBUFF];
    char buff2[ERRORBUFF];

    if (argc < 2) {
        printf("Enter 2 paths and try again\n");
        return 0;
    }
    int fd1 = open(args[1], O_RDONLY);
    if (fd1 == -1) {
        strcpy(buff, "Can't open file 1\n");
        write(2, buff, strlen(buff));
        return -1;
    }
    int fd2 = open(args[2], O_RDONLY);
    if (fd2 == -1) {
        strcpy(buff, "Can't open file 2\n");
        write(2, buff, strlen(buff));
        strcpy(buff2, "Can't close file number 1\n");
        if (close(fd1) == -1) {
            write(2, buff, strlen(buff));
        }
        return -1;
    }

    int result = compareFiles(fd1, fd2);
    closeFD(fd1, fd2);
    return result;
}

/**
 * This function compare the files by their file descriptor.
 * @param fd1 file descriptor of file 1
 * @param fd2 file descriptor of file 2
 * @return -1 if error, 1 if not similar, 2 if similar, 3 if identical.
 */
int compareFiles(int fd1, int fd2) {
    char buff[ERRORBUFF];
    strcpy(buff, "Can't read file\n");
    char buffFD1;
    char buffFD2;
    ssize_t readFD1;
    ssize_t readFD2;
    int result = 3;

    while (TRUE) {
        readFD1 = read(fd1, &buffFD1, BYTE);
        readFD2 = read(fd2, &buffFD2, BYTE);
        if (readFD1 == -1 || readFD2 == -1) {
            write(2, buff, strlen(buff));
            result = -1;
            return result;
        } else {
            if (readFD1 == 0 && readFD2 == 0) {
                //reached end of files
                return result;
            } else if (readFD1 == 0 && readFD2 == 1) {
                //reached end of file 1 and continue reading 2, if found char != ' ' or '\n' than result is 1
                // otherwise result is 2
                result = 2;
                do {
                    readFD2 = read(fd2, &buffFD2, BYTE);
                    if (readFD2 == -1) {
                        write(2, buff, strlen(buff));
                        result = -1;
                        return result;
                    }
                    if (skippedChar(buffFD2)) {
                        continue;
                    } else {
                        result = 1;
                        return result;
                    }
                } while (readFD2 != 0);
            } else if (readFD1 == 1 && readFD2 == 0) {
                //reached end of file 2 and continue reading 1, if found char != ' ' or '\n' than result is 1
                // otherwise result is 2
                result = 2;
                do {
                    readFD1 = read(fd1, &buffFD1, BYTE);
                    if (readFD1 == -1) {
                        write(2, buff, strlen(buff));
                        result = -1;
                        return result;
                    }
                    if (skippedChar(buffFD1)) {
                        continue;
                    } else {
                        result = 1;
                        return result;
                    }
                } while (readFD1 != 0);
            } else {
                // resultFD2 == 1 && resultFD1 == 1
                if (!skippedChar(buffFD1) && !skippedChar(buffFD2)) {
                    // if not equal stop comparing
                    if (buffFD1 == buffFD2) {
                        continue;
                    } else if (tolower(buffFD1) == tolower(buffFD2)) {
                        result = 2;
                        continue;
                    } else {
                        result = 1;
                        return result;
                    }
                } else if (buffFD1 == ' ' && buffFD2 == ' ') {
                    continue;
                } else if (buffFD1 == '\n' && buffFD2 == '\n') {
                    continue;
                } else if (buffFD1 == ' ' && buffFD2 == '\n') {
                    result = 2;
                    continue;
                } else if (buffFD1 == '\n' && buffFD2 == ' ') {
                    result = 2;
                    continue;
                } else if (skippedChar(buffFD1) && !skippedChar(buffFD2)) {
                    result = 2;
                    //continue reading until finding a char in file1 which isn't a space
                    while (readFD1 != 0) {
                        readFD1 = read(fd1, &buffFD1, BYTE);
                        if (readFD1 == -1) {
                            write(2, buff, strlen(buff));
                            result = -1;
                            return result;
                        }
                        if (readFD1 == 0 && tolower(buffFD1) != tolower(buffFD2)) {
                            result = 1;
                            return result;
                        }
                        if (skippedChar(buffFD1)) {
                            continue;
                        } else {
                            if (buffFD1 == buffFD2) {
                                break;
                            } else if (tolower(buffFD1) == tolower(buffFD2)) {
                                result = 2;
                                break;
                            } else {
                                result = 1;
                                return result;
                            }
                        }
                    }
                } else if (!skippedChar(buffFD1) && skippedChar(buffFD2)) {
                    result = 2;
                    // continue reading until finding a char in file2 which isn't a space
                    while (readFD2 != 0) {
                        readFD2 = read(fd2, &buffFD2, BYTE);
                        if (readFD2 == -1) {
                            write(2, buff, strlen(buff));
                            result = -1;
                            return result;
                        }
                        if (readFD2 == 0 && tolower(buffFD1) != tolower(buffFD2)) {
                            result = 1;
                            return result;
                        }
                        if (skippedChar(buffFD2)) {
                            continue;
                        } else {
                            if (buffFD1 == buffFD2) {
                                break;
                            } else if (tolower(buffFD1) == tolower(buffFD2)) {
                                result = 2;
                                break;
                            } else {
                                result = 1;
                                return result;
                            }
                        }
                    }
                }
            }
        }
    }
}


/**
 * Find if char is space or new line
 * @param buff char
 * @return if char is space or new line
 */
int skippedChar(char buff) {
    if (buff == ' ' || buff == '\n') {
        return 1;
    } else {
        return 0;
    }
}

/**
 * Close both file descriptors.
 * @param fd1 file descriptor 1
 * @param fd2 file desriptor 2
 */
void closeFD(int fd1, int fd2) {
    char buff[ERRORBUFF];
    strcpy(buff, "Can't close file\n");
    if (close(fd1) == -1) {
        write(2, buff, strlen(buff));
    }
    if (close(fd2) == -1) {
        write(2, buff, strlen(buff));
    }
}