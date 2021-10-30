/**
 * Copyright (c) 2021 TGT, Inc. All Rights Reserved.
 * Date     : 10/27/21 1:47 PM
 * Author   : Linlin Zhou
 * E-mail   : 461146760@qq.com
 * */

#include "list.h"
#include "timer_task.h"
#include "utils.h"
#include "log.h"
#include "task.h"
#include "security_memory.h"

#include <pthread.h>

LIST_HEAD(timer_task_list);

static void *task_thread(void *arg)
{
    task_t *task, *next;
//    uint32_t task_cnt;

    while (1) {
        usleep(1000 * 1000);
        if (list_empty(&timer_task_list))
            continue;
//        task_cnt = 0;
        list_for_each_entry_safe(task, next, &timer_task_list, task_node) {
//            task_cnt++;
            switch (task->state) {
                case TIMER_TASK_STATE_WORK:
                    if (task->cnt % task->periodic == 0) {
                        task->cnt = 0;
//                        LOGI("timer: %d expiry");
                        if (task->handler) {
                            if (task->task_type == TIMER_TASK_TYPE_THREAD)
                                task_maker(task->handler, task->handler_arg);
                            else
                                task->handler(task->handler_arg);
                        } else {
                            list_del(&task->task_node);
                            security_free(task);
                        }

                        if (task->is_once)
                            task->state = TIMER_TASK_STATE_DIE;
                    }

                    task->cnt++;
                    break;
                case TIMER_TASK_STATE_PAUSE:
                    break;
                case TIMER_TASK_STATE_DIE:
                    list_del(&task->task_node);
                    security_free(task);
                    break;
                default:
                    task->state = TIMER_TASK_STATE_DIE;
                    break;
            }
        }

//        LOGI("TASK_NUM: %d", task_cnt);
    }

    pthread_exit(NULL);
}

void timer_task_add(task_handler_t handler, void *handler_arg, uint32_t periodic, bool is_once, task_type_t task_type)
{
    task_t *task;

    if (!handler) {
        return;
    }

    task = security_zerolloc(sizeof(task_t));

    if (!task) {
        LOGE("No memory!");
        return;
    }

    task->periodic = periodic;
    task->is_once = is_once;
    task->handler = handler;
    task->handler_arg = handler_arg;
    task->task_type = task_type;

    task->state = TIMER_TASK_STATE_WORK;
    task->cnt = 0;

    list_add_tail(&task->task_node, &timer_task_list);
}

void timer_task_pause(task_handler_t handler)
{
    task_t *task;
    list_for_each_entry(task, &timer_task_list, task_node) {
        if (task->handler == handler) {
            task->state = TIMER_TASK_STATE_PAUSE;
            break;
        }
    }
}

void timer_task_cancel(task_handler_t handler)
{
    task_t *task;
    list_for_each_entry(task, &timer_task_list, task_node) {
        if (task->handler == handler) {
            task->state = TIMER_TASK_STATE_DIE;
            break;
        }
    }
}

void timer_task_modify_periodic(task_handler_t handler, uint32_t periodic)
{
    task_t *task;
    list_for_each_entry(task, &timer_task_list, task_node) {
        if (task->handler == handler) {
            task->periodic = periodic;
            break;
        }
    }
}

int timer_task_init(void)
{
    if (0 != task_maker(task_thread, NULL)) {
        LOGI("Create timer service failed!");
        return -1;
    }

    return 0;
}