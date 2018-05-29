// Raz Shenkman
// 311130777
#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include "osqueue.h"
#include "pthread.h"
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <zconf.h>
#include <pthread.h>
#include <fcntl.h>
#define ERROR "Error in system call\n"
#define OPERATION_SUCCEEDED 1
#define OPERATION_FAILED -1


typedef enum ThreadPoolState {
    STAGE_ONE_ABORT, STAGE_TWO_ABORT, RUNNING, FINAL_ABORT, CHECK_STATE
} state;

/**
 * Write error to the stderr.
 */
void errorToStderrAndExit();

/**
 * Struct of a thread pool.
 */
typedef struct thread_pool {
    int size, stopped;
    pthread_t *threads;
    OSQueue *tasksQueue;
    pthread_cond_t pthreadCond;
    pthread_mutex_t queueLock, destroyPoolLock;
    void *(*executeTasks)(void *);
    state tpState;
} ThreadPool;

/**
 * Struct of a task.
 */
typedef struct task {
    void (*taskFunc)(void *);

    void *args;
} Task;

/**
 * Main function for every thread to constantly run, checks the queue for tasks and execute if found.
 * @param args the thread pool
 * @return void*
 */
void *execute(void *args);

/**
 * Create a new thread pull with <numOfThreads> threads.
 * @param numOfThreads number of threads for the pool.
 * @return a pointer to the thread pool.
 */
ThreadPool *tpCreate(int numOfThreads);

/**
 * Stop and free a thread pool (stop all running threads) and free from memory.
 * @param threadPool pool to stop.
 * @param shouldWaitForTasks- 0 if we shouldn't wait for all tasks in queue to finish, not 0 if we should wait for
 * the tasks in the queue.
 */
void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks);

/**
 * Insert a task to the queue
 * @param threadPool the pool which has the queue
 * @param computeFunc function to perform.
 * @param param parameters to the computeFunc
 * @return success (1), or not success (-1)
 */
int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param);

#endif
