#include "client.h"
#include <stdlib.h>
#include <unistd.h>

client_t *create_client(int fd)
{
    client_t *client = calloc(1, sizeof(client_t));
    if (!client)
        return NULL;
    client->fd = fd;
    client->state = STATE_READING;
    client->file_fd = -1;
    return client;
}

void free_client(client_t *client)
{
    if (!client)
        return;
    if (client->file_fd >= 0)
    {
        close(client->file_fd);
    }
    close(client->fd);
    free(client);
}

