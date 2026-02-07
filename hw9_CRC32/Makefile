all: get_crc32

get_crc32: main.c
	$(CC) $^ -o $@ -Wall -Wextra -Wpedantic -std=c11

clean:
	rm -f get_crc32

.PHONY: all clean