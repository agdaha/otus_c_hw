CC = gcc
CFLAGS = -Wall -Wextra -g -I.
LDFLAGS = -lpthread -rdynamic

LIB_SOURCES = logger.c
LIB_OBJECTS = $(LIB_SOURCES:.c=.o)
LIB_NAME = liblogger.a

TEST_SOURCE = main.c
TEST_EXECUTABLE = logger_test

all: $(LIB_NAME) $(TEST_EXECUTABLE)

$(LIB_NAME): $(LIB_OBJECTS)
	ar rcs $@ $^

$(TEST_EXECUTABLE): $(TEST_SOURCE) $(LIB_NAME)
	$(CC) $(CFLAGS) -o $@ $< -L. -llogger $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(LIB_OBJECTS) $(LIB_NAME) $(TEST_EXECUTABLE)

.PHONY: all clean