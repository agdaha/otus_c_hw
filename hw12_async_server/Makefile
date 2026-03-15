CFLAGS = -Wall -Wextra -Wpedantic -std=c11
SRCS = main.c client.c server.c http.c
OBJS = $(SRCS:.c=.o)
TARGET = asyc_server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean