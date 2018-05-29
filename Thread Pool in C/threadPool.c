// Raz Shenkman
// 311130777

#include "threadPool.h"


void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks) {
    int i = 0;
    // check if tpDestroy has been called already
    pthread_mutex_lock(&(threadPool->destroyPoolLock));
    if (threadPool->stopped) { return; }
    pthread_mutex_unlock(&(threadPool->destroyPoolLock));
    // check if we in CHECK_STATE
    if (threadPool->tpState == CHECK_STATE) { return; }

    // lock the queue and broadcast to all of thread to continue
    pthread_mutex_lock(&threadPool->queueLock);
    if (pthread_cond_broadcast(&threadPool->pthreadCond) || pthread_mutex_unlock(&threadPool->queueLock)) {
        errorToStderrAndExit();
    }
    // if we should finish the current threads and exit
    if (shouldWaitForTasks == 0) {
        // make all the threads that aren't running task to stop
        threadPool->tpState = STAGE_ONE_ABORT;
        //printf("threadPool->tpState == STAGE_ONE_ABORT;\n");
        // wait for stopped threads and for tasks in queue
        pthread_mutex_lock(&(threadPool->destroyPoolLock));
        threadPool->stopped = 1;
        pthread_mutex_unlock(&(threadPool->destroyPoolLock));
        // wait only for the running threads

        for (i; i < threadPool->size; i++) {
            pthread_join(threadPool->threads[i], NULL);
        }
        // free all tasks that are in the queue
        while (!osIsQueueEmpty(threadPool->tasksQueue)) {
            Task *task;
            if ((task = (Task *) osDequeue(threadPool->tasksQueue)) != NULL) {
                free(task);
            }
        }
        threadPool->tpState = STAGE_TWO_ABORT;
        free(threadPool->threads);
        osDestroyQueue(threadPool->tasksQueue);
        free(threadPool);
    } else {
        // wait for the threads in the queue to finish
        threadPool->tpState = FINAL_ABORT;

        // wait for stopped threads and for tasks in queue
        pthread_mutex_trylock(&(threadPool->destroyPoolLock));
        threadPool->stopped = 1;
        pthread_mutex_unlock(&(threadPool->destroyPoolLock));
        // wait for running threads only
        for (i; i < threadPool->size; i++) {
            pthread_join(threadPool->threads[i], NULL);
        }
        threadPool->tpState = STAGE_TWO_ABORT;

        free(threadPool->threads);
        osDestroyQueue(threadPool->tasksQueue);
        free(threadPool);
    }
}

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param) {
    // don't insert task if the pool has been stopped
    if (threadPool->stopped) {
        return OPERATION_FAILED;
    }
    if (threadPool->tpState == CHECK_STATE) { return OPERATION_FAILED; }

    Task *task;
    if ((task = calloc(1, sizeof(Task))) == NULL) {
        errorToStderrAndExit();
    }
    task->taskFunc = computeFunc;
    task->args = param;

    // add task to the queue
    pthread_mutex_lock(&threadPool->queueLock);
    osEnqueue(threadPool->tasksQueue, task);
    // allow one thread to continue (added 1 task)
    if (pthread_cond_signal(&threadPool->pthreadCond)) {
        errorToStderrAndExit();
    }
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

    if ((resultFD = open("output.txt", O_CREAT | O_TRUNC | O_WRONLY, 0644)) == -1) {
        errorToStderrAndExit();
    }
    if (dup2(resultFD, STDOUT_FILENO) == -1) { errorToStderrAndExit(); }

    ThreadPool *tP;
    if ((tP = (ThreadPool *) calloc(1, sizeof(ThreadPool))) == NULL) {
        errorToStderrAndExit();
    }
    // init mutexes
    if (pthread_mutex_init(&tP->queueLock, NULL)) { errorToStderrAndExit(); }
    if (pthread_mutex_init(&tP->destroyPoolLock, NULL)) { errorToStderrAndExit(); }
    if (pthread_cond_init(&tP->pthreadCond, NULL)) { errorToStderrAndExit(); }
    tP->tasksQueue = osCreateQueue();
    tP->size = numOfThreads;
    tP->executeTasks = execute;
    tP->stopped = 0;
    tP->tpState = RUNNING;
    if ((tP->threads = (pthread_t *) calloc((size_t) numOfThreads, sizeof(pthread_t))) == NULL) {
        errorToStderrAndExit();
    }

    for (i; i < tP->size; i++) {
        pthread_create(tP->threads + i, NULL, execute, tP);
    }

    return tP;
}


void errorToStderrAndExit() {
    write(STDERR_FILENO, ERROR, sizeof(ERROR));
    exit(-1);
}


void *execute(void *args) {
    ThreadPool *tP = (ThreadPool *) args;
    // if should start executing tasks at all (all the name when not after joining threads)
    while (tP->tpState == RUNNING || tP->tpState == STAGE_ONE_ABORT || tP->tpState == FINAL_ABORT
           || tP->tpState == CHECK_STATE) {
        // when running normally and queue is empty, here we wait without busy waiting for new tasks to be in the queue.
        if (osIsQueueEmpty(tP->tasksQueue) && (tP->tpState == RUNNING || tP->tpState == STAGE_ONE_ABORT)) {
            pthread_mutex_lock(&tP->queueLock);
            pthread_cond_wait(&tP->pthreadCond, &tP->queueLock);
            // if the queue is empty and tpDestroy with 0 has been called
        } else if (tP->tpState == FINAL_ABORT && osIsQueueEmpty(tP->tasksQueue)) {
            //printf("finished first");
            break;
            // lock the mutex for any other reason
        } else {
            pthread_mutex_lock(&tP->queueLock);
        }
        // if we aren't finished joining the threads, execute a task if the queue isn't empty
        if (tP->tpState == RUNNING || tP->tpState == STAGE_ONE_ABORT || tP->tpState == FINAL_ABORT
            || tP->tpState == CHECK_STATE) {
            if (!osIsQueueEmpty(tP->tasksQueue)) {
                Task *task = (Task *) osDequeue(tP->tasksQueue);
                pthread_mutex_unlock(&tP->queueLock);
                task->taskFunc(task->args);
                free(task);
                // queue is empty- do nothing
            } else {
                pthread_mutex_unlock(&tP->queueLock);
            }
            // finish the thread (we executed a task/ didn't execute a task and tpDestroy with 0 has been called)
            if (tP->tpState == STAGE_ONE_ABORT) {
                break;
            }
        } else if (tP->tpState == CHECK_STATE) {
            break;
        } else {
            pthread_mutex_unlock(&tP->queueLock);
        }
    }
    //printf("finished second");
}
