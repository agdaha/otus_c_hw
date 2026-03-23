all: stat_db_table

stat_db_table: main.c
	$(CC) $^ -o $@ -Wall -Wextra -Wpedantic -std=c11 -I`pg_config --includedir` -L`pg_config --libdir` -lsqlite3 -lpq
clean:
	rm -f stat_db_table

.PHONY: all clean