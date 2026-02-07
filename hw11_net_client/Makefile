all: ascii_art

ascii_art: main.c
	$(CC) $^ -o $@ -Wall -Wextra -Wpedantic -std=c11

clean:
	rm -f ascii_art

.PHONY: all clean