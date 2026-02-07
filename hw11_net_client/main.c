#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define HOST_NAME "telehack.com"
#define PORT "23"
#define BUFFER_SIZE 4096
#define TIMEOUT_SEC 1

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <font_name> <your_text>\n", argv[0]);
        fprintf(stderr, "Example: %s banner \"Hello World\"\n", argv[0]);
        return EXIT_FAILURE;
    }
    const char *font = argv[1];
    const char *text = argv[2];

    char command[256];
    snprintf(command, sizeof(command), "figlet /%s %s\r\n", font, text);

    struct addrinfo hints, *res, *p;
    int sock_fd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(HOST_NAME, PORT, &hints, &res);
    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("socket");
            continue;
        }
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = 0;

        if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        {
            perror("setsockopt");
            close(sock_fd);
            continue;
        }

        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) == -1)
        {
            perror("connect");
            continue;
        }
        break;
    }

    freeaddrinfo(res);

    if (p == NULL)
    {
        fprintf(stderr, "Failed to connect to %s\n", HOST_NAME);
        return EXIT_FAILURE;
    }

    unsigned char response[BUFFER_SIZE];
    int r;
    int len = 0;
    while ((r = recv(sock_fd, &response[len], BUFFER_SIZE-len, 0)) > 0){
        len +=r;
    }

    if (send(sock_fd, command, strlen(command), 0) < 0)
    {
        perror("send");
        close(sock_fd);
        return EXIT_FAILURE;
    }

    len = 0;
    while ((r = recv(sock_fd, &response[len], BUFFER_SIZE-len, 0)) > 0){
        len +=r;
    }

    char *start = strstr((char *)response, "\r\n");
    for (int i = start - (char *)&response[0] + 2; i < len; i++)
    {
        putchar(response[i]);
    }

    shutdown(sock_fd, SHUT_RDWR);
    close(sock_fd);
    return EXIT_SUCCESS;
}