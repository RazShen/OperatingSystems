// Raz Shenkman
// 311130777

#include <stdio.h>
#include <fcntl.h>
#include <zconf.h>
#include <malloc.h>
#include <dirent.h>
#include <memory.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <wait.h>

#define BYTE 1
#define BUFFSIZE 256
#define EXECVPLEN 10
#define SLEEPTIME 5
#define ERRORWRITE "Error in system call\n"

/**
 * This method fills the paths by the configuration file
 * @param fd file descriptor to the configuration file
 * @param path path to fill
 * @return if successfully filled
 */
int fillPaths(int fd, char *path);

/**
 * This method will scan will run from the mail and will operate all the other function in order to
 * scan the folders for c files, run the a.out and write to results.csv.
 * @param students main dir of the c projects.
 * @param searchPath the path of the students dir
 * @param input input to a.out
 * @param correctOutputPath currect output
 * @param resultFD filedescriptor of result.csv
 * @return int.
 */
int scanRunAndWrite(DIR *students, char *searchPath, char *input, char *correctOutputPath, int resultFD);

/**
 * Scan for c file recursively.
 * @param path to scanm
 * @param finalPath of result.
 * @return if found/didn't found.
 */
int searchRecursivelyForFile(char *path, char *finalPath);

/**
 * Check if file exists.
 * @param fileName name.
 * @return exists/doesn't.
 */
int checkIfFileCreated(char *fileName);

/**
 * Run the a.out and comp.out and compare output to the correctoutput.
 * @param inputPath for the input.
 * @param correctOutPath correct output.
 * @return compare result.
 */
int compareFiles(char *inputPath, char *correctOutPath);

/**
 * Write error to stderr.
 */
void writeErrorToStderr();

/**
 * Main function (operates the scanRunAndWrite and create its parameters).
 * @param argc number of args
 * @param args configuration file
 * @return result
 */
int main(int argc, char *args[]) {
     // Read the file and parse the paths

    char *searchPath = calloc(BUFFSIZE, sizeof(char));
    char *inputPath = calloc(BUFFSIZE, sizeof(char));
    char *correctOutputPath = calloc(BUFFSIZE, sizeof(char));
    if (searchPath == NULL || inputPath == NULL || correctOutputPath == NULL) {
        writeErrorToStderr();
    }
    if (argc <= 1) {
        writeErrorToStderr();
    }
    // open the configuration and parse
    int fd = open(args[1], O_RDONLY);
    if (fd == -1) {
        writeErrorToStderr();
    }
    if (fillPaths(fd, searchPath) == -1) {
        writeErrorToStderr();
    }
    if (fillPaths(fd, inputPath) == -1) {
        writeErrorToStderr();
    }
    if (fillPaths(fd, correctOutputPath) == -1) {
        writeErrorToStderr();
    }
    if (close(fd) == -1) {
        writeErrorToStderr();
    }

    // Open the results.csv file
    int resultFD = open("results.csv", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (resultFD == -1) {
        writeErrorToStderr();
    }
    // Open the folder for scanning
    DIR *students;
    if ((students = opendir(searchPath)) == NULL) {
        writeErrorToStderr();
    }
    // Return the result of scanning and checking
    int result = scanRunAndWrite(students, searchPath, inputPath, correctOutputPath, resultFD);
    close(resultFD);
    free(inputPath);
    free(correctOutputPath);
    free(searchPath);
    if (closedir(students) == -1) {
        writeErrorToStderr();
    }
    return result;
}

int fillPaths(int fd, char *path) {
    int counter = 0;
    ssize_t checkRead;
    char chr = '0';
    while (chr != '\n' && chr != EOF) {
        checkRead = read(fd, &chr, BYTE);
        if (checkRead == -1) {
            writeErrorToStderr();
        }
        path[counter] = chr;
        counter++;
    }
    //remove \n
    if (path[counter - 1] == '\n') {
        path[counter - 1] = '\0';
    }
    return 0;
}

int scanRunAndWrite(DIR *students, char *searchPath, char *input, char *correctOutputPath, int resultFD) {
    struct dirent *studentsDirent;
    size_t len = strlen(searchPath);
    // for every sub folder
    while ((studentsDirent = readdir(students)) != NULL) {
        if (strcmp(studentsDirent->d_name, ".") != 0 && strcmp(studentsDirent->d_name, "..") != 0) {
            char *basePath = calloc(BUFFSIZE, sizeof(char));
            strcpy(basePath, searchPath);
            basePath[len] = '/';
            strcat(basePath, studentsDirent->d_name);
            char writeRes[BUFFSIZE];
            char *resultPath = calloc(BUFFSIZE, sizeof(char));
            // scan each subfolder of the main folder
            int result = searchRecursivelyForFile(basePath, resultPath);
            if (result == -1) {
                writeErrorToStderr();
            } else if (result == 0) {
                // didn't find any c file
                strcpy(writeRes, studentsDirent->d_name);
                strcat(writeRes, ",0,NO_C_FILE\n");
                if(write(resultFD, writeRes, strlen(writeRes)) == -1) {
                    writeErrorToStderr();
                }
            } else if (result == 1) {
                // found c file (result path) now compile
                char *argv[EXECVPLEN];
                argv[0] = calloc(BUFFSIZE, sizeof(char));
                argv[1] = calloc(BUFFSIZE, sizeof(char));
                strcpy(argv[0], "gcc");
                strcpy(argv[1], resultPath);
                argv[2] = NULL;
                if (checkIfFileCreated("a.out")) {
                    unlink("a.out");
                }
                pid_t pid = fork();
                if (pid == -1) {
                    writeErrorToStderr();
                } else if (pid > 0) {
                    // We are in the father's process
                    int status = 0;
                    waitpid(pid, &status, 0);
                    if (checkIfFileCreated("a.out")) {
                        // run comparison and stuff
                        int finalRes = compareFiles(input, correctOutputPath);
                        if (finalRes == -1) {
                            //timeout
                            strcpy(writeRes, studentsDirent->d_name);
                            strcat(writeRes, ",0,TIMEOUT\n");
                            if(write(resultFD, writeRes, strlen(writeRes))==-1) {
                                writeErrorToStderr();
                            }
                        } else if (finalRes == 1) {
                            //bad output
                            strcpy(writeRes, studentsDirent->d_name);
                            strcat(writeRes, ",60,BAD_OUTPUT\n");
                            if(write(resultFD, writeRes, strlen(writeRes)) ==-1) {
                                writeErrorToStderr();
                            }
                        } else if (finalRes == 2) {
                            // similar output
                            strcpy(writeRes, studentsDirent->d_name);
                            strcat(writeRes, ",80,SIMILAR_OUTPUT\n");
                            if(write(resultFD, writeRes, strlen(writeRes))==-1) {
                                writeErrorToStderr();
                            }
                        } else if (finalRes == 3) {
                            // great job
                            strcpy(writeRes, studentsDirent->d_name);
                            strcat(writeRes, ",100,GREAT_JOB\n");
                            if(write(resultFD, writeRes, strlen(writeRes))==-1) {
                                writeErrorToStderr();
                            }
                        }
                        if (unlink("a.out") == -1) {
                            writeErrorToStderr();
                        }
                    } else {
                        // couldn't compile
                        strcpy(writeRes, studentsDirent->d_name);
                        strcat(writeRes, ",0,COMPILATION_ERROR\n");
                        if(write(resultFD, writeRes, strlen(writeRes))==-1) {
                            writeErrorToStderr();
                        }
                    }

                } else if (pid == 0) {
                    // We are in the son's process
                    if (execvp(argv[0], &argv[0]) == -1) {
                        // error in compilation
                        writeErrorToStderr();
                    }
                }
            }
            free(basePath);
            free(resultPath);
        }
    }
}

void writeErrorToStderr() {
    write(STDERR_FILENO, ERRORWRITE, strlen(ERRORWRITE));
    exit(-1);
}

int searchRecursivelyForFile(char *path, char *finalPath) {
    struct dirent *entry;
    DIR *dp;

    // Try to open the folder
    if ((dp = opendir(path)) == NULL) {
        return -1;
    }
    size_t len = strlen(path);
    while ((entry = readdir(dp))) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char *basePath = calloc(BUFFSIZE, sizeof(char));
            strcpy(basePath, path);
            basePath[len] = '/';
            strcat(basePath, entry->d_name);
            struct stat entryPath_stat;
            if (stat(basePath, &entryPath_stat) == -1) {
                return -1;
            }
            if (S_ISDIR(entryPath_stat.st_mode)) {
                // the recursive search in all subfolders
                if (searchRecursivelyForFile(basePath, finalPath)) {
                    return 1;
                }
            } else {
                // check if file ends with .c
                if (strcmp(basePath + strlen(basePath) - 2, ".c") == 0) {
                    strcpy(finalPath, basePath);
                    return 1;
                }
            }
            free(basePath);
        }
    }
    if (closedir(dp) == -1) {
        writeErrorToStderr();
    }
    return 0;
}

int checkIfFileCreated(char *fileName) {
    DIR *dip;
    struct dirent *dit;
    char cwd[BUFFSIZE];
    // find the directory path
    getcwd(cwd, sizeof(cwd));
    if ((dip = opendir(cwd)) == NULL) {
        writeErrorToStderr();
    }

    while ((dit = readdir(dip)) != NULL) {
        if (strcmp(dit->d_name, fileName) == 0) {
            return 1;
        }
    }
    if (closedir(dip) == -1) {
        writeErrorToStderr();
    }
    return 0;
}

int compareFiles(char *inputPath, char *correctOutPath) {
    // first run the a.out with the input path argument, need to switch the stdin to be inputPath
    // need to switch the output path to the stdout, then we can compare the output file to the correct output.
    int outputFD, inputFD;
    // open the output file (will be user of out)
    outputFD = open("output", O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (outputFD == -1) {
        writeErrorToStderr();
    }
    // now stdout "points" at output FD
    dup2(outputFD, STDOUT_FILENO);
    // open the inputpath for the stdin
    inputFD = open(inputPath, O_RDONLY);
    if (inputFD == -1) {
        writeErrorToStderr();
    }
    // now stdin "points" at input FD
    dup2(inputFD, STDIN_FILENO);
    char *argv[EXECVPLEN];
    argv[0] = calloc(BUFFSIZE, sizeof(char));
    // the a.out will use the inputFD as stdin
    strcpy(argv[0], "./a.out");
    argv[1] = NULL;
    pid_t pid = fork();
    if (pid == -1) {
        writeErrorToStderr();
    } else if (pid > 0) {
        // We are in the father's process
        int status = 0;
        // sleep for 5 seconds
        sleep(SLEEPTIME);
        // if the process still running- the process result is timeout
        pid_t returnedPid = waitpid(pid, &status, WNOHANG);
        if (returnedPid == 0) {
            if (checkIfFileCreated("output")) {
                if(unlink("output") == -1) {
                    writeErrorToStderr();
                }
            }
            return -1;
        }
    } else if (pid == 0) {
        // We are in the son's process
        if (execvp(argv[0], &argv[0]) == -1) {
            writeErrorToStderr();
        }
    }
    close(inputFD);
    close(outputFD);
    free(argv[0]);
    argv[0] = calloc(BUFFSIZE, sizeof(char));
    argv[1] = calloc(BUFFSIZE, sizeof(char));
    argv[2] = calloc(BUFFSIZE, sizeof(char));
    strcpy(argv[0], "./comp.out");
    strcpy(argv[1], "output");
    strcpy(argv[2], correctOutPath);
    argv[3] = NULL;
    pid = fork();
    if (pid == -1) {
        writeErrorToStderr();
    } else if (pid > 0) {
        // We are in the father's process
        int status = 0;
        if (waitpid(pid, &status, 0) > 0) {
            if (checkIfFileCreated("output")) {
                if(unlink("output") == -1) {
                    writeErrorToStderr();
                }
                // return the exit status of the comp.out
                return WEXITSTATUS(status);
            }
        }
    } else if (pid == 0) {
        // We are in the son's process
        if (execvp(argv[0], &argv[0]) == -1) {
            writeErrorToStderr();
        }
    }
}
