#include "epoll_event.h"
#include "log.h"
#include "security_memory.h"
#include "task.h"

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#ifndef MAX_EPOLL_SIZE
#define MAX_EPOLL_SIZE					30
#endif

LIST_HEAD(event_node_list);

static int epoll_event_fd_set(int epoll_fd, event_node_t *node)
{
	int flags;

	flags = fcntl(node->fd, F_GETFL, 0);

	if (fcntl(node->fd, F_SETFL, flags | O_NONBLOCK) < 0) {
		LOGI("Failed to set the listen fd to nonblock mode(errno:%d).", errno);
		return -1;
	}

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, node->fd, &(node->event)) < 0) {
		LOGI("Failed to add the fd to epoll(errno:%d).", errno);
		return -1;
	}

	return 0;
}

int event_register(event_poll_t *et, int fd, uint32_t events, epoll_handle_t callback, void *arg)
{
	event_node_t *p;

    if (et->fd_size >= MAX_EPOLL_SIZE) {
        LOGI("Too many epoll listen fd.");
        return -1;
    }

	p = (event_node_t *)security_zerolloc(sizeof(event_node_t));

	if (NULL == p) {
		LOGI("No memory to allocate event node.");
		return -1;
	}

	p->fd = fd;
	p->event.data.fd = fd;
	p->event.events = events;
	p->epoll_handle = callback;
	p->epoll_arg = arg;

	if (0 != epoll_event_fd_set(et->epfd, p)) {
	    LOGE("Listen file(%d) failed!", fd);
		security_free(p);
		return -1;
	}

	list_add_tail(&p->event_node, &event_node_list);

	et->fd_size++;

	return 0;
}

int event_unregister(event_poll_t *et, int fd)
{
    event_node_t *event, *next;

    list_for_each_entry_safe(event, next, &event_node_list, event_node) {
        if (event->fd == fd) {
            if (epoll_ctl(et->epfd, EPOLL_CTL_DEL, event->fd, &(event->event)) < 0) {
                LOGI("Failed to add the fd to epoll(errno:%d).", errno);
                return -1;
            }

            list_del(&event->event_node);
            security_free(event);

            et->fd_size--;
            return 0;
        }
    };

    return 0;
}

static void *event_poll_thread(void *arg)
{
    event_poll_t *ep = (event_poll_t *)arg;
	struct epoll_event events[MAX_EPOLL_SIZE];
	int nfds;

wait:
	while (ep->fd_size == 0)
		usleep(100000);

	// epoll_wait loop
	while (!ep->exit) {
		nfds = epoll_wait(ep->epfd, events, MAX_EPOLL_SIZE, 500);

		if (nfds == -1) {
			if (errno == EINTR)
				continue;

			LOGI("epoll_wait() error(%d): %s", errno, strerror(errno));
			break;
		} else if (nfds == 0) {
			continue;
		}

		for (int i = 0; i < nfds; i++) {
			event_node_t *p;
			list_for_each_entry(p, &event_node_list, event_node) {
				if (p->event.data.fd == events[i].data.fd) {
					if (EPOLLIN & events[i].events) {
						p->epoll_handle(p->epoll_arg);
						//p->epoll_handle(p->epoll_arg);
						//event->events = EPOLLIN | EPOLLET | EPOLLONESHOT;
					} else if (EPOLLERR & events[i].events) {
						//TODO
					} else if (EPOLLOUT & events[i].events) {
						//TODO
					}
//                    p->epoll_handle(p->epoll_arg, &events[i]);
//                    if (epoll_ctl(ep->epfd, EPOLL_CTL_MOD, events[i].data.fd, &events[i]) < 0) {
//                        LOGI("Failed to add the fd to epoll(errno:%d).\n", errno);
//                    }
				}
			}
		}

		if (ep->fd_size <= 0)
			goto wait;
	}

	close(ep->epfd);
    ep->fd_size = 0;

	pthread_exit(NULL);
}

event_poll_t *event_poll_init(void)
{
    event_poll_t *ep = (event_poll_t *)security_zerolloc(sizeof(event_poll_t));
	ep->epfd = -1;
	ep->exit = 0;
	ep->fd_size = 0;

	// create epoll fd
	if ((ep->epfd = epoll_create(MAX_EPOLL_SIZE)) < 0) {
		LOGE("Failed to create the epoll instance.");
		security_free(ep);
		return NULL;
	}

	if (task_maker(event_poll_thread, ep) != 0) {
		LOGE("Start uart bus service failure.");
		security_free(ep);
		return NULL;
	}

	return ep;
}

void event_poll_exit(event_poll_t *ep)
{
    ep->exit = 1;
}