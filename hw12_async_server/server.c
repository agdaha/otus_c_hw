#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>

#include "client.h"
#include "server.h"
#include "http.h"

bool set_nonblocking(int fd)
{
    const int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
    {
        perror("fcntl");
        return false;
    }
    int rc = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (rc < 0)
    {
        perror("fcntl");
        return false;
    }
    return true;
}

void handle_event(struct epoll_event *ev)
{
    client_t *client = (client_t *)ev->data.ptr;

    if (!client)
        return;

    int close_conn = 0;

    if (client->state == STATE_READING)
    {
        if (read_from_client(client) < 0)
        {
            close_conn = 1;
        }
        else if (client->state == STATE_WRITING)
        {
            ev->events = EPOLLOUT | EPOLLET;
            ev->data.ptr = client;
            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client->fd, ev);
        }
    }
    else if (client->state == STATE_WRITING)
    {
        if (write_to_client(client) < 0)
        {
            close_conn = 1;
        }
    }

    if (close_conn || client->state == STATE_CLOSING)
    {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->fd, NULL);
        free_client(client);
    }
}

void accept_connections(void)
{
    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break;
            }
            return;
        }

        if (!set_nonblocking(client_fd))
        {
            close(client_fd);
            continue;
        }

        client_t *client = create_client(client_fd);
        if (!client)
        {
            close(client_fd);
            continue;
        }

        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.ptr = client;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0)
        {
            free_client(client);
            continue;
        }
    }
}