#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <wait.h>
#include <stdlib.h>
#include <unistd.h>


#define MAXLEN 100
#define TRUE 1
#define PRESSEDEXIT 5

/**
 * Struct of job hold a name, process ID and points to the next job.
 */
typedef struct job {
    char *name;
    pid_t pid;
    struct job* next;
} job;

/**
 * This method print the jobs that are currently running.
 * @param head of the jobs list.
 */
void printJobsList(job *head);

/**
 * Delete all the jobs (when exit() is activated)
 * @param head of the jobs list.
 */
void deleteJobs(job* head);

/**
 * Add a new job to the list.
 * @param head head of the jobs list.
 * @param name of the job.
 * @param pid process ID.
 */
void addJobToEndOfList(job *head, char *name, int pid);

/**
 * Parse new command inputted.
 * @param argv arguments.
 * @param fullCommand the string of the command
 * @return background/exit/cd command.
 */
int getCommand(char **argv, char* fullCommand);

/**
 * Clear the finished jobs from the list.
 * @param head of the jobs list.
 */
void ClearFinishedJobs(job *head);

/**
 * Delete specific process from the jobs list.
 * @param head of the jobs list.
 * @param pid- process ID.
 */
void deleteProcessFromJobList(job* head, pid_t pid);

int main() {
    char nameJob[MAXLEN];
    char input[MAXLEN] = "";
    job* head = malloc(sizeof(job));;
    if (head == NULL) {
        return 1;  // couldn't create jobs list, finish this program.
    }
    head->name = calloc(6,1);
    if (head->name == NULL) {
        return 1;  // couldn't create jobs list, finish this program.
    }
    strcpy(head->name, "Head");
    head->next = NULL;
    while (TRUE) {
        char PressedEnterDummy;
        input[0] = 0;
        printf("prompt> ");
        char *argv[MAXLEN];
        scanf("%[^\n]s", input);
        scanf("%c", &PressedEnterDummy);
        if(input[0] == 0) {
            input[0] = 1;   // user only pressed enter
            continue;
        }
        strcpy(nameJob, input);
        int isBackGround = getCommand(argv, input);
        if (isBackGround == PRESSEDEXIT) {
            deleteJobs(head);
            exit(0);
        } else {
            if (strcmp(argv[0], "cd") == 0) {
                char *cd = (char *) calloc(strlen(argv[1]) + 1, sizeof(char));
                strcpy(cd, argv[1]);
                chdir(cd);
                free(cd);
            } else if (strcmp(argv[0], "jobs") == 0) {
                ClearFinishedJobs(head);
                printJobsList(head);
            } else {
                pid_t pid = fork();
                if (pid == -1) {
                    fprintf(stderr, "Couldn't create new process\n");
                } else if (pid > 0) {
                    // We are in the father's process
                    printf("%d\n", pid);
                    if (!isBackGround) {
                        wait(NULL);
                    } else {
                        addJobToEndOfList(head, nameJob, pid);
                    }
                } else if (pid == 0) {
                    // We are in the son's process
                    char base[MAXLEN] = "/bin/";
                    // Create command
                    strcat(base, argv[0]);
                    if (execv(base, argv) == -1) {
                        fprintf(stderr, "Error in system call\n");
                    }
                    return 0;
                }
            }
        }
    }
}

int getCommand(char **argv, char* fullCommand) {
    char *word;
    char copy[MAXLEN];
    int spacesIter = 0;
    strncpy(copy, fullCommand, strlen(fullCommand));
    word = strtok(fullCommand, " ");
    while (word != NULL) {
        argv[spacesIter] = (char*)calloc(strlen(word) + 1, 1);
        strncpy(argv[spacesIter], word, strlen(word));
        spacesIter++;
        word = strtok(NULL, " ");
    }
    argv[spacesIter] = NULL; //argv ends with NULL
    if (strcmp(argv[0], "exit") == 0) {
        return PRESSEDEXIT;
    }
    if (strcmp(argv[spacesIter - 1], "&") == 0) {
        argv[spacesIter-1] = NULL;
        return 1;
    }
    return 0;
}

void printJobsList(job *head) {
    job * current = head;
    current = current->next;
    while (current != NULL) {
        printf("%d %s\n", current->pid, current->name);
        current = current->next;
    }
}

void addJobToEndOfList(job *head, char *name, int pid) {
    job * current = head;
    // go to the end of the list
    while (current->next != NULL) {
        current = current->next;
    }
    // link new job to the last node
    current->next = calloc(sizeof(job), sizeof(char));
    name[strlen(name) - 1] = 0;
    name[strlen(name) - 1] = 0;
    current->next->name = calloc(sizeof(name)+ 1, 1);
    strcpy(current->next->name, name);
    current->next->pid = pid;
    current->next->next = NULL;
}

void ClearFinishedJobs(job *head) {
    int status;
    job* last = head;
    while (last->next != NULL) {
        last = last->next;
        pid_t returnedPid = waitpid(last->pid, &status, WNOHANG);
        if (returnedPid == last->pid) {
            deleteProcessFromJobList(head, returnedPid);
        }
    }
}



void deleteProcessFromJobList(job* head, pid_t pid) {
    job* beforeLast = head;
    job* last = head;
    while (last->next != NULL) {
        last = last->next;
        if (last->pid == pid) {
            // delete last
            beforeLast->next = last->next;
            free(last->name);
            free(last);
            break;
        }
        beforeLast = beforeLast->next;
    }
}


void deleteJobs(job* head) {
    job* current = head;
    job* next;
    while (current != NULL) {
        next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
}