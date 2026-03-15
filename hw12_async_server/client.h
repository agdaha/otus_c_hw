#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <stdint.h>
#include <signal.h>


#define BUFFER_SIZE 8192

// Состояние клиентского соединения
typedef enum
{
    STATE_READING,
    STATE_WRITING,
    STATE_CLOSING
} client_state_t;

typedef struct
{
    int fd;
    client_state_t state;
    char request[BUFFER_SIZE];
    size_t request_len;
    char response[BUFFER_SIZE];
    size_t response_len;
    size_t response_sent;
    int file_fd;
    off_t file_size;
    off_t file_sent;
    char content_type[64];
} client_t;

client_t *create_client(int fd);

void free_client(client_t *client);

#endif /* CLIENT_H */