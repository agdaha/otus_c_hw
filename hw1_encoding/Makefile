
all: toUTF8

toUTF8: main.c
	$(CC) $^ -o $@ -Wall -Wextra -Wpedantic -std=c11

clean:
	rm -f toUTF8 core

.PHONY: all clean
