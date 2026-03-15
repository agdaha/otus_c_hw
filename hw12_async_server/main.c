#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <limits.h>
#include <ctype.h>
#include <stdbool.h>

#include "client.h"
#include "server.h"

#define MAX_EVENTS 1024

const char *root_dir = NULL;
int epoll_fd = -1;
int listen_fd = -1;
volatile sig_atomic_t keep_running = 1; 

void handle_sigint(int sig)
{
    (void)sig;
    keep_running = 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <root_directory> <address:port>\n", argv[0]);
        fprintf(stderr, "Example: %s /var/www/html 0.0.0.0:8080\n", argv[0]);
        return EXIT_FAILURE;
    }

    root_dir = argv[1];

    // Проверяем существование директории
    struct stat st;
    if (stat(root_dir, &st) != 0 || !S_ISDIR(st.st_mode))
    {
        fprintf(stderr, "Error: '%s' is not a valid directory\n", root_dir);
        return EXIT_FAILURE;
    }

    // Парсим адрес:порт
    char *addr_port = argv[2];
    char *colon = strrchr(addr_port, ':');
    if (!colon)
    {
        fprintf(stderr, "Error: Invalid address:port format\n");
        return EXIT_FAILURE;
    }

    *colon = '\0';
    const char *addr = addr_port;
    int port = atoi(colon + 1);

    if (port <= 0 || port > 65535)
    {
        fprintf(stderr, "Error: Invalid port number\n");
        return EXIT_FAILURE;
    }


    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, handle_sigint);

    // Создаем сокет
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    if (!set_nonblocking(listen_fd))
    {
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (strlen(addr) == 0 || strcmp(addr, "0.0.0.0") == 0)
    {
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        if (inet_pton(AF_INET, addr, &serv_addr.sin_addr) <= 0)
        {
            fprintf(stderr, "Error: Invalid address '%s'\n", addr);
            close(listen_fd);
            return EXIT_FAILURE;
        }
    }

    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, SOMAXCONN) < 0)
    {
        perror("listen");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    // Создаём epoll
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0)
    {
        perror("epoll_create1");
        close(listen_fd);
        return EXIT_FAILURE;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = NULL;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev) < 0)
    {
        perror("epoll_ctl");
        close(epoll_fd);
        close(listen_fd);
        return EXIT_FAILURE;
    }

    printf("Server listening on %s:%d, serving files from %s\n", addr, port, root_dir);

    struct epoll_event events[MAX_EVENTS];

    while (keep_running)
    {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            if (errno == EINTR) {
                continue; 
            } else {
                perror("epoll_wait fatal error");
            }
        }

        for (int i = 0; i < nfds; i++)
        {
            if (events[i].data.ptr == NULL)
            {
                accept_connections();
            }
            else
            {
                handle_event(&events[i]);
            }
        }
    }

    close(epoll_fd);
    close(listen_fd);

    return EXIT_SUCCESS;
}