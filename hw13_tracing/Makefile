all: trace_me

trace_me: main.c
	$(CC) $^ -o $@ -Wall -Wextra -Wpedantic -std=c11

clean:
	rm -f trace_me

.PHONY: all clean