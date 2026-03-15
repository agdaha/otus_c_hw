#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>

int main()
{
    // 1. Управление памятью
    size_t size = 4096;
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    // 2. Открытие файла
    int fd = open("trace_test.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);

    // 3. Запись в файл
    write(fd, "Ftrace analysis\n", 16);
    close(fd);

    // 4. Создание процесса
    pid_t pid = fork();

    if (pid == 0)
    {
        // Дочерний процесс
        // Пауза
        struct timespec ts = {0, 500000000}; // 0.5 сек
        nanosleep(&ts, NULL);
        exit(0);
    }
    else
    {
        // Ожидание завершения
        wait(NULL);
    }

    // Освобождение памяти
    munmap(ptr, size);

    return 0;
}