#ifndef SERVER_H
#define SERVER_H

#include <sys/epoll.h>
#include <stdbool.h>

extern int listen_fd;
extern int epoll_fd;

// Установка флага O_NONBLOCK
bool set_nonblocking(int fd);

// Обработка события epoll
void handle_event(struct epoll_event *ev);

// Принятие подключений
void accept_connections(void);


#endif /* SERVER_H */