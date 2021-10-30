//
// Created by smallfish on 6/18/21.
//

#include "inter_thread_comm.h"
#include "security_memory.h"
#include "log.h"
#include "times.h"

inter_thread_comm_t *itc_create(void)
{
    inter_thread_comm_t *itc = security_zerolloc(sizeof(inter_thread_comm_t));

    if (!itc) {
        LOGE("No memory!");
        return itc;
    }

    pthread_mutex_init(&itc->mutex, NULL);
    pthread_cond_init(&itc->cond, NULL);

    return itc;
}

void itc_destroy(inter_thread_comm_t *itc)
{
    pthread_cond_broadcast(&itc->cond);
    pthread_mutex_destroy(&itc->mutex);
    pthread_cond_destroy(&itc->cond);
}

void itc_send(inter_thread_comm_t *itc)
{
    pthread_mutex_lock(&itc->mutex);
    pthread_cond_signal(&itc->cond);
    pthread_mutex_unlock(&itc->mutex);
}

void itc_broadcast(inter_thread_comm_t *itc)
{
    pthread_mutex_lock(&itc->mutex);
    pthread_cond_broadcast(&itc->cond);
    pthread_mutex_unlock(&itc->mutex);
}

int itc_wait(inter_thread_comm_t *itc)
{
    pthread_cond_wait(&itc->cond, &itc->mutex);
//    struct timespec timeout;
//    struct timeval now;
//    int ret;
//
//    gettimeofday(&now, NULL);
//    timeout.tv_sec = now.tv_sec + 30;
//    timeout.tv_nsec = now.tv_usec * 1000;
//
//    pthread_mutex_lock(&itc->mutex);
//    ret = pthread_cond_timedwait(&itc->cond, &itc->mutex, &timeout);
//    pthread_mutex_unlock(&itc->mutex);

    return 0;
}