//
// Created by smallfish on 6/18/21.
//

#ifndef LINUX_CPE_INTER_THREAD_COMM_H
#define LINUX_CPE_INTER_THREAD_COMM_H

#include <pthread.h>
#include "types.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} inter_thread_comm_t;

inter_thread_comm_t *itc_create(void);

void itc_destroy(inter_thread_comm_t *itc);

void itc_send(inter_thread_comm_t *itc);

void itc_broadcast(inter_thread_comm_t *itc);

int itc_wait(inter_thread_comm_t *itc);

#ifdef __cplusplus
}
#endif

#endif //LINUX_CPE_INTER_THREAD_COMM_H
