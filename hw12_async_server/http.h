#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>
#include "client.h"

#define MAX_PATH_LEN 4096

extern const char *root_dir;

// Отправка списка файлов в директории
void send_dir_listing(client_t *client, const char *path, const char *decoded_path);

// Определение MIME-типа по расширению
const char *get_mime_type(const char *path);

// Отправка HTTP-ответа 
void send_response(client_t *client, int status, const char *status_text,
                   const char *content_type, const char *body, size_t body_len);

// Отправка файла 
void send_file_response(client_t *client, int status, const char *status_text,
                        const char *content_type, int fd, off_t size);

// URL-декодирование 
int url_decode(char *dst, const char *src, size_t dst_size);

// Обработка HTTP-запроса 
void handle_request(client_t *client);

// Чтение данных от клиента 
int read_from_client(client_t *client);

// Запись данных клиенту 
int write_to_client(client_t *client);

#endif // HTTP_H 