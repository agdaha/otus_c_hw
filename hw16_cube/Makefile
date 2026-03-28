all: cube

cube: main.c
	$(CC) $^ -o $@ -Wall -Wextra -Wpedantic -std=c11 -lGL -lGLU -lglut

clean:
	rm -f cube

.PHONY: all clean