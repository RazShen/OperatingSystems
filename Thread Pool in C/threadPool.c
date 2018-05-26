// Raz Shenkman
// 311130777
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <zconf.h>
#include <pthread.h>
#include <fcntl.h>
#include "threadPool.h"

//TODO: Unlink output.txt?


void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks) {
    if (threadPool->stopped) { return ;}
    int i = 0;
    pthread_mutex_lock(&(threadPool->queueLock));
    pthread_mutex_lock(&(threadPool->destroyPoolLock));
    threadPool->stopped = 1;
    if (shouldWaitForTasks != 0) {
        // wait for stopped threads and for tasks in queue

    } else {
        // wait for stopped threads only
//        for (i; i < threadPool->size; i++) {
//            pthread_join(threadPool->threads[i], NULL);
//        }
//        osDestroyQueue(threadPool->tasksQueue);
//        free(threadPool->tasksQueue);
//        for (i = 0; i < threadPool->size; i++) {
//            free(threadPool->threads + i);
//        }
        // TODO: osDes
    }
    pthread_mutex_unlock(&(threadPool->queueLock));
    pthread_mutex_destroy(&(threadPool->queueLock));
    pthread_mutex_unlock(&(threadPool->destroyPoolLock));
    pthread_mutex_destroy(&(threadPool->destroyPoolLock));
}

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param) {
    if (threadPool->stopped) {
        return OPERATION_FAILED;
    }
    Task *task;
    if ((task = calloc(1, sizeof(Task))) == NULL) {
        errorToStderrAndExit();
    }
    task->taskFunc = computeFunc;
    task->args = param;
    pthread_mutex_lock(&threadPool->queueLock);
    osEnqueue(threadPool->tasksQueue, task);
    pthread_mutex_unlock(&threadPool->queueLock);
    return OPERATION_SUCCEEDED;
}

/**
 * Creating the thread pool with <numOfThreads> threads, also initializing the threads and mutexes.
 * @param numOfThreads number of
 * @return
 */
ThreadPool *tpCreate(int numOfThreads) {
    int i = 0, resultFD;
    if((resultFD = open("output.txt", O_CREAT | O_TRUNC | O_WRONLY, 0644)) == -1) {
        errorToStderrAndExit();
    }
    dup2(resultFD, STDOUT_FILENO);

    ThreadPool *tP;
    if ((tP = (ThreadPool *) calloc(1, sizeof(ThreadPool))) == NULL) {
        errorToStderrAndExit();
    }
    tP->tasksQueue = osCreateQueue();
    tP->size = numOfThreads;
    tP->executeTasks = execute;
    tP->stopped = 0;
    if ((tP->threads = (pthread_t *) calloc((size_t) numOfThreads, sizeof(pthread_t))) == NULL) {
        errorToStderrAndExit();
    }

    for (i; i < tP->size; i++) {
        pthread_create(tP->threads + i, NULL, execute, tP);
    }
    if (pthread_mutex_init(&tP->queueLock, NULL)) { errorToStderrAndExit(); };
    if (pthread_mutex_init(&tP->destroyPoolLock, NULL)) { errorToStderrAndExit(); };
    // TODO: Add lock destruct
    return tP;
}


void errorToStderrAndExit() {
    write(STDERR_FILENO, ERROR, sizeof(ERROR));
    exit(-1);
}


void *execute(void *args) {
    ThreadPool *tP = (ThreadPool *) args;
    while (!tP->stopped) {
        //TODO: Avoid busy waiting
        while (osIsQueueEmpty(tP->tasksQueue)) { break; }
        //sem_wait(!osIsQueueEmpty(tP->tasksQueue));
        if (!tP->stopped & !osIsQueueEmpty(tP->tasksQueue)) {
            pthread_mutex_trylock(&tP->queueLock);
            if (!osIsQueueEmpty(tP->tasksQueue)) {
                Task* task = (Task *) osDequeue(tP->tasksQueue);
                pthread_mutex_unlock(&tP->queueLock);
                task->taskFunc(task->args);
                free(task);
            }
        }
    }
}

