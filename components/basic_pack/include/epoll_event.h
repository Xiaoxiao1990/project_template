#ifndef __EPOLL_EVENT_H__
#define __EPOLL_EVENT_H__

#include <sys/epoll.h>
#include "list.h"

typedef int (*epoll_handle_t)(void *arg);

typedef struct {
    int fd;
    struct epoll_event event;
    void *epoll_arg;
    epoll_handle_t epoll_handle;
    struct list_head event_node;
} event_node_t;

typedef struct{
    int epfd;
    int exit;
    int fd_size;
} event_poll_t;

#ifdef __cplusplus
extern "C"{
#endif

event_poll_t *event_poll_init(void);
int event_register(event_poll_t *et, int fd, uint32_t events, epoll_handle_t callback, void *arg);
int event_unregister(event_poll_t *et, int fd);
void event_poll_exit(event_poll_t *ep);

#ifdef __cplusplus
}
#endif
#endif /* __EPOLL_EVENT_H__ */
