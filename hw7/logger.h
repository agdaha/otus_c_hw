#ifndef LOGGER_H
#define LOGGER_H

#include <stddef.h>
#include <stdio.h>
#include <pthread.h>

typedef enum
{
  L_DEBUG,
  L_INFO,
  L_WARNING,
  L_ERROR
} log_level;

typedef struct logger_type logger_type;

logger_type *logger_init(const char *file_name);
void logger_close();
void logger_log(logger_type *logger, log_level level, const char *file, int line, const char *func, const char *format, ...);

#define LOG_DEBUG(logger,...) logger_log(logger, L_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_INFO(logger,...) logger_log(logger,L_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_WARNING(logger,...) logger_log(logger,L_WARNING, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define LOG_ERROR(logger,...) logger_log(logger,L_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)

#endif