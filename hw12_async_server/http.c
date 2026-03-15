#define _GNU_SOURCE
#include "http.h"
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

void send_dir_listing(client_t *client, const char *path, const char *decoded_path)
{
    DIR *dir = opendir(path);
    if (!dir)
    {
        const char *body = "Forbidden";
        send_response(client, 403, "Forbidden", "text/plain", body, strlen(body));
        return;
    }

    char body[BUFFER_SIZE];
    size_t offset = 0;
    int ret = snprintf(body + offset, sizeof(body) - offset,
                       "<!DOCTYPE html>\n"
                       "<html>\n"
                       "<head><title>Index of %s</title></head>\n"
                       "<body>\n"
                       "<h1>Index of %s</h1>\n"
                       "<hr>\n"
                       "<ul>\n",
                       decoded_path, decoded_path);
    if (ret < 0 || (size_t)ret >= sizeof(body) - offset)
    {
        closedir(dir);
        client->state = STATE_CLOSING;
        return;
    }
    offset += (size_t)ret;

    struct dirent *entry;
    struct stat st;
    char full_path[PATH_MAX];

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0)
            continue;

        const char *display_name = entry->d_name;
        char link_path[MAX_PATH_LEN];
        
        if (strcmp(decoded_path, "/") == 0)
        {
            ret = snprintf(link_path, sizeof(link_path), "/%s", entry->d_name);
        }
        else
        {
            ret = snprintf(link_path, sizeof(link_path), "%s/%s", decoded_path, entry->d_name);
        }
        
        if (ret < 0 || (size_t)ret >= sizeof(link_path))
            continue;

        ret = snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (ret < 0 || (size_t)ret >= sizeof(full_path))
            continue;

        if (stat(full_path, &st) == 0)
        {
            const char *suffix = S_ISDIR(st.st_mode) ? "/" : "";
            ret = snprintf(body + offset, sizeof(body) - offset,
                           "<li><a href=\"%s%s\">%s%s</a></li>\n",
                           link_path, suffix, display_name, suffix);
            if (ret < 0 || (size_t)ret >= sizeof(body) - offset)
                break;
            offset += (size_t)ret;
        }
    }

    closedir(dir);

    ret = snprintf(body + offset, sizeof(body) - offset,
                   "</ul>\n"
                   "<hr>\n"
                   "</body>\n"
                   "</html>\n");
    if (ret < 0 || (size_t)ret >= sizeof(body) - offset)
    {
        client->state = STATE_CLOSING;
        return;
    }
    offset += (size_t)ret;

    send_response(client, 200, "OK", "text/html", body, offset);
}

const char *get_mime_type(const char *path)
{
    const char *ext = strrchr(path, '.');
    if (!ext)
        return "application/octet-stream";

    if (strcasecmp(ext, ".html") == 0 || strcasecmp(ext, ".htm") == 0)
        return "text/html";
    if (strcasecmp(ext, ".css") == 0)
        return "text/css";
    if (strcasecmp(ext, ".js") == 0)
        return "application/javascript";
    if (strcasecmp(ext, ".json") == 0)
        return "application/json";
    if (strcasecmp(ext, ".xml") == 0)
        return "application/xml";
    if (strcasecmp(ext, ".txt") == 0)
        return "text/plain";
    if (strcasecmp(ext, ".png") == 0)
        return "image/png";
    if (strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext, ".jpeg") == 0)
        return "image/jpeg";
    if (strcasecmp(ext, ".gif") == 0)
        return "image/gif";
    if (strcasecmp(ext, ".svg") == 0)
        return "image/svg+xml";
    if (strcasecmp(ext, ".ico") == 0)
        return "image/x-icon";
    if (strcasecmp(ext, ".pdf") == 0)
        return "application/pdf";

    return "application/octet-stream";
}

void send_response(client_t *client, int status, const char *status_text,
                   const char *content_type, const char *body, size_t body_len)
{
    char header[512];
    int header_len = snprintf(header, sizeof(header),
                              "HTTP/1.1 %d %s\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %zu\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              status, status_text, content_type, body_len);

    if (header_len > 0 && (size_t)header_len <= sizeof(client->response))
    {
        memcpy(client->response, header, (size_t)header_len);
        client->response_len = (size_t)header_len;
        if (body && body_len > 0 && client->response_len + body_len <= sizeof(client->response))
        {
            memcpy(client->response + client->response_len, body, body_len);
            client->response_len += body_len;
        }
        client->response_sent = 0;
        client->state = STATE_WRITING;
    }
    else
    {
        client->state = STATE_CLOSING;
    }
}

void send_file_response(client_t *client, int status, const char *status_text,
                        const char *content_type, int fd, off_t size)
{
    char header[512];
    int header_len = snprintf(header, sizeof(header),
                              "HTTP/1.1 %d %s\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %ld\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              status, status_text, content_type, (long)size);

    if (header_len > 0 && (size_t)header_len <= sizeof(client->response))
    {
        memcpy(client->response, header, (size_t)header_len);
        client->response_len = (size_t)header_len;
        client->response_sent = 0;
        client->file_fd = fd;
        client->file_size = size;
        client->file_sent = 0;
        strncpy(client->content_type, content_type, sizeof(client->content_type) - 1);
        client->state = STATE_WRITING;
    }
    else
    {
        close(fd);
        client->state = STATE_CLOSING;
    }
}

int url_decode(char *dst, const char *src, size_t dst_size)
{
    size_t i = 0, j = 0;
    while (src[i] && j < dst_size - 1)
    {
        if (src[i] == '%')
        {
            if (src[i + 1] && src[i + 2] && isxdigit((unsigned char)src[i + 1]) &&
                isxdigit((unsigned char)src[i + 2]))
            {
                char hex[3] = {src[i + 1], src[i + 2], 0};
                dst[j++] = (char)strtol(hex, NULL, 16);
                i += 3;
                continue;
            }
        }
        else if (src[i] == '+')
        {
            dst[j++] = ' ';
            i++;
            continue;
        }
        dst[j++] = src[i++];
    }
    dst[j] = '\0';
    return 0;
}

void handle_request(client_t *client)
{
    if (strncmp(client->request, "GET ", 4) != 0)
    {
        const char *body = "Method Not Allowed";
        send_response(client, 405, "Method Not Allowed", "text/plain", body, strlen(body));
        return;
    }

    char *path_start = client->request + 4;
    char *path_end = strchr(path_start, ' ');
    if (!path_end)
    {
        const char *body = "Bad Request";
        send_response(client, 400, "Bad Request", "text/plain", body, strlen(body));
        return;
    }

    size_t path_len = (size_t)(path_end - path_start);
    if (path_len >= MAX_PATH_LEN)
    {
        const char *body = "URI Too Long";
        send_response(client, 414, "URI Too Long", "text/plain", body, strlen(body));
        return;
    }

    char decoded_path[MAX_PATH_LEN];
    char raw_path[MAX_PATH_LEN];
    memcpy(raw_path, path_start, path_len);
    raw_path[path_len] = '\0';

    if (url_decode(decoded_path, raw_path, sizeof(decoded_path)) != 0)
    {
        const char *body = "Bad Request";
        send_response(client, 400, "Bad Request", "text/plain", body, strlen(body));
        return;
    }

    if (strstr(decoded_path, "..") != NULL)
    {
        const char *body = "Forbidden";
        send_response(client, 403, "Forbidden", "text/plain", body, strlen(body));
        return;
    }

    char full_path[PATH_MAX];
    int ret = snprintf(full_path, sizeof(full_path), "%s%s", root_dir, decoded_path);
    if (ret < 0 || (size_t)ret >= sizeof(full_path))
    {
        const char *body = "URI Too Long";
        send_response(client, 414, "URI Too Long", "text/plain", body, strlen(body));
        return;
    }

    struct stat st;
    if (stat(full_path, &st) == 0)
    {
        if (S_ISDIR(st.st_mode))
        {
            if (strcmp(decoded_path, "/") == 0)
            {
                send_dir_listing(client, full_path, decoded_path);
                return;
            }
            
            char index_path[PATH_MAX];
            ret = snprintf(index_path, sizeof(index_path), "%s/index.html", full_path);
            if (ret < 0 || (size_t)ret >= sizeof(index_path))
            {
                const char *body = "URI Too Long";
                send_response(client, 414, "URI Too Long", "text/plain", body, strlen(body));
                return;
            }
            if (stat(index_path, &st) != 0)
            {
                send_dir_listing(client, full_path, decoded_path);
                return;
            }
            strncpy(full_path, index_path, sizeof(full_path) - 1);
        }
    }

    if (stat(full_path, &st) != 0)
    {
        const char *body = "Not Found";
        send_response(client, 404, "Not Found", "text/plain", body, strlen(body));
        return;
    }

    if (!S_ISREG(st.st_mode))
    {
        const char *body = "Forbidden";
        send_response(client, 403, "Forbidden", "text/plain", body, strlen(body));
        return;
    }

    if (access(full_path, R_OK) != 0)
    {
        const char *body = "Forbidden";
        send_response(client, 403, "Forbidden", "text/plain", body, strlen(body));
        return;
    }

    int fd = open(full_path, O_RDONLY);
    if (fd < 0)
    {
        const char *body = "Forbidden";
        send_response(client, 403, "Forbidden", "text/plain", body, strlen(body));
        return;
    }

    const char *mime_type = get_mime_type(full_path);
    send_file_response(client, 200, "OK", mime_type, fd, st.st_size);
}

int read_from_client(client_t *client)
{
    size_t remaining = sizeof(client->request) - client->request_len - 1;
    if (remaining == 0)
    {
        return -1; 
    }

    ssize_t n = read(client->fd, client->request + client->request_len, remaining);
    if (n < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return 0; 
        }
        return -1; 
    }

    if (n == 0)
    {
        return -1; 
    }

    client->request_len += (size_t)n;
    client->request[client->request_len] = '\0';

    if (strstr(client->request, "\r\n\r\n") != NULL)
    {
        handle_request(client);
    }

    return 0;
}

int write_to_client(client_t *client)
{
    if (client->response_sent < client->response_len)
    {
        ssize_t n = write(client->fd,
                          client->response + client->response_sent,
                          client->response_len - client->response_sent);
        if (n < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                return 0;
            }
            return -1;
        }
        client->response_sent += (size_t)n;

        if (client->response_sent < client->response_len)
        {
            return 0;
        }

        if (client->file_fd < 0)
        {
            return -1;
        }
    }

    if (client->file_fd >= 0)
    {
        char buffer[8192];
        off_t remaining = client->file_size - client->file_sent;

        while (remaining > 0)
        {
            size_t to_read = sizeof(buffer);
            if ((off_t)to_read > remaining)
            {
                to_read = (size_t)remaining;
            }

            ssize_t n = read(client->file_fd, buffer, to_read);
            if (n < 0)
            {
                return -1;
            }
            if (n == 0)
            {
                break;
            }

            ssize_t written = write(client->fd, buffer, (size_t)n);
            if (written < 0)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    lseek(client->file_fd, -n, SEEK_CUR);
                    return 0;
                }
                return -1;
            }

            client->file_sent += written;

            if (written < n)
            {
                lseek(client->file_fd, -(n - written), SEEK_CUR);
                return 0;
            }
        }

        if (client->file_sent >= client->file_size)
        {
            return -1; 
        }
    }

    return 0;
}