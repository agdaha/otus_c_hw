#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <execinfo.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_BACKTRACE_DEPTH 128

typedef struct logger_type
{
  FILE *file;
  pthread_mutex_t mutex;
  char *filename;
} logger_type;

logger_type *logger_init(const char *filename)
{
  if (!filename)
  {
    errno = EINVAL;
    return NULL;
  }

  logger_type *logger = malloc(sizeof(logger_type));
  if (!logger)
  {
    errno = ENOMEM;
    return NULL;
  }

  logger->filename = strdup(filename);
  if (!logger->filename)
  {
    free(logger);
    return NULL;
  }

  logger->file = fopen(filename, "a");
  if (!logger->file)
  {
    int saved_errno = errno;
    free(logger->filename);
    free(logger);
    errno = saved_errno;
    return NULL;
  }

  setvbuf(logger->file, NULL, _IOLBF, 0);

  int mutex_result = pthread_mutex_init(&logger->mutex, NULL);
  if (mutex_result != 0)
  {
    fclose(logger->file);
    free(logger->filename);
    free(logger);
    errno = mutex_result;
    return NULL;
  }

  return logger;
}

void logger_close(logger_type *logger)
{
  if (!logger)
    return;

  pthread_mutex_lock(&logger->mutex);

  if (logger->file)
  {
    fflush(logger->file);
    fclose(logger->file);
    logger->file = NULL;
  }

  pthread_mutex_unlock(&logger->mutex);
  pthread_mutex_destroy(&logger->mutex);

  free(logger->filename);
  free(logger);
}

static void get_current_time(char *buffer, size_t buffer_size)
{
  time_t now = time(NULL);
  struct tm tm_now;
  localtime_r(&now, &tm_now);
  strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", &tm_now);
}

// Запись стека вызовов в файл
static void log_backtrace(logger_type *logger)
{
  void *buffer[MAX_BACKTRACE_DEPTH];
  int nptrs = backtrace(buffer, MAX_BACKTRACE_DEPTH);

  fprintf(logger->file, "Backtrace:\n");
  backtrace_symbols_fd(buffer + 1, nptrs - 1, fileno(logger->file));
}

void logger_log(logger_type *logger, log_level level, const char *file, int line, const char *func, const char *format, ...)
{
  if (!logger)
    return;

  pthread_mutex_lock(&logger->mutex);

  if (!logger->file)
  {
    logger->file = fopen(logger->filename, "a");
    if (!logger->file)
    {
      pthread_mutex_unlock(&logger->mutex);
      return;
    }
    setvbuf(logger->file, NULL, _IOLBF, 0);
  }

  const char *level_str = "";

  switch (level)
  {
  case L_DEBUG:
    level_str = "DEBUG";
    break;
  case L_INFO:
    level_str = "INFO";
    break;
  case L_WARNING:
    level_str = "WARNING";
    break;
  case L_ERROR:
    level_str = "ERROR";
    break;
  }

  char timestamp[32];
  get_current_time(timestamp, sizeof(timestamp));

  fprintf(logger->file, "[%s] %s: %s:%d (%s) - ", timestamp, level_str, file, line, func);

  va_list args;
  va_start(args, format);
  vfprintf(logger->file, format, args);
  va_end(args);

  fprintf(logger->file, "\n");

  if (level == L_ERROR)
  {
    log_backtrace(logger);
  }

  pthread_mutex_unlock(&logger->mutex);
}