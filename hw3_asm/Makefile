AS = nasm  # NOTE: yasm тоже подойдёт

all: asm-prog c-prog c_ref-prog asm_ref-prog

asm-prog: main.o
	$(CC) -no-pie $^ -o $@

main.o: main.asm
	$(AS) -felf64 $^

asm_ref-prog: main_ref.o
	$(CC) -no-pie $^ -o $@

main_ref.o: main_ref.asm
	$(AS) -felf64 $^


c-prog: main.c 
	$(CC) $^ -o $@ -Wall -Wextra -Wpedantic -std=c11

c_ref-prog: main_ref.c 
	$(CC) $^ -o $@ -Wall -Wextra -Wpedantic -std=c11

clean:
	rm -f *-prog *.o core

.PHONY: clean

