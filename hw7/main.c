#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

void test_debug_logger(logger_type *log) {
  LOG_DEBUG(log, "Debug message");
}

void test_warn_logger(logger_type *log) {
  LOG_WARNING(log, "Warning message");
}

int main() {

    logger_type* logger = logger_init("test.log");
    if (!logger) {
        fprintf(stderr, "Failed to initialize logger: %s\n", strerror(errno));
        return 1;
    }
    
    LOG_INFO(logger, "Logger initialized successfully");
    test_debug_logger(logger);
    test_warn_logger(logger);
    
    logger_type* bad_logger = logger_init(NULL);
    if (!bad_logger) {
        LOG_ERROR(logger, "Failed to initialize logger with NULL filename: %s", strerror(errno));
    }
    
    bad_logger = logger_init("/invalid/path/to/log/file.log");
    if (!bad_logger) {
        LOG_ERROR(logger, "Failed to initialize logger with invalid path: %s", strerror(errno));
    }
    
    LOG_INFO(logger, "All tests completed");
    
    logger_close(logger);
    return 0;
}