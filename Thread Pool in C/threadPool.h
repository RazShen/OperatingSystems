// Raz Shenkman
// 311130777
#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include "osqueue.h"

#define ERROR "Error in system call\n"
#define OPERATION_SUCCEEDED 1
#define OPERATION_FAILED -1

void errorToStderrAndExit();

typedef struct thread_pool
{
    int size, stopped;
    pthread_t* threads;
    OSQueue* tasksQueue;
    pthread_mutex_t queueLock, destroyPoolLock;
    void* (*executeTasks)(void *);
} ThreadPool;

typedef struct task
{
    void (*taskFunc)(void *);
    void * args;
} Task;

void* execute(void *args);

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
