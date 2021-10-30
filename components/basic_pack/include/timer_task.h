/**
 * Copyright (c) 2021 TGT, Inc. All Rights Reserved.
 * Date     : 10/27/21 1:47 PM
 * Author   : Linlin Zhou
 * E-mail   : 461146760@qq.com
 * */

#ifndef __LINUX_CPE_TIMER_H__
#define __LINUX_CPE_TIMER_H__

#include "types.h"
#include "list.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef enum {
    TIMER_TASK_STATE_WORK,
    TIMER_TASK_STATE_PAUSE,
    TIMER_TASK_STATE_DIE,
} task_state_t;

typedef enum {
    TIMER_TASK_TYPE_NORMAL,
    TIMER_TASK_TYPE_THREAD
} task_type_t;

typedef void *(*task_handler_t)(void *);

typedef struct {
    /* for user define */
    uint32_t periodic;
    bool is_once;
    task_handler_t handler;
    void *handler_arg;
    task_type_t task_type;

    /* for internal state control */
    uint32_t cnt;
    task_state_t state;
    struct list_head task_node;
} task_t;

int timer_task_init(void);

void timer_task_add(task_handler_t handler, void *handler_arg, uint32_t periodic, bool is_once, task_type_t task_type);
void timer_task_pause(task_handler_t handler);
void timer_task_cancel(task_handler_t handler);
void timer_task_modify_periodic(task_handler_t handler, uint32_t periodic);

#ifdef __cplusplus
}
#endif

#endif //__LINUX_CPE_TIMER_H__
