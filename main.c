#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mmemory.h"

void test_thrashing() {
    printf("--- ТЕСТ 1: Пробуксовка (Thrashing) ---\n");
    int n = 1000; // 1000 страниц всего (RAM вмещает 250)
    int page_size = 128;
    int ops = 200000; // 200 тысяч операций чтения/записи
    int percents[] = {10, 25, 50, 100}; // Процент использования памяти

    for (int p = 0; p < 4; p++) {
        init_memory(n, page_size);
        int active_pages = (n * percents[p]) / 100;
        VA *ptrs = malloc(active_pages * sizeof(VA));
        char buf[16] = "testdata";

        // Выделяем память
        for (int i = 0; i < active_pages; i++) {
            _malloc(&ptrs[i], 16);
        }

        // Нагружаем память случайным доступом
        clock_t start = clock();
        for (int i = 0; i < ops; i++) {
            int idx = rand() % active_pages;
            if (i % 2 == 0) _write(ptrs[idx], buf, 16);
            else _read(ptrs[idx], buf, 16);
        }
        clock_t end = clock();

        double ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
        printf("Рабочий набор: %3d%% от DISK | Время %d операций: %7.2f мс\n", percents[p], ops, ms);
        free(ptrs);
    }
    printf("\n");
}

void test_fragmentation() {
    printf("--- ТЕСТ 2: Фрагментация аллокатора ---\n");
    int node_counts[] = {100, 1000, 5000, 10000};

    for (int c = 0; c < 4; c++) {
        init_memory(20000, 64);
        int count = node_counts[c];
        VA *ptrs = malloc(count * sizeof(VA));

        // Создаем много мелких блоков
        for (int i = 0; i < count; i++) {
            _malloc(&ptrs[i], 16);
        }

        // Освобождаем каждый второй (создаем эффект "шахматной доски" из занятых и свободных блоков)
        for (int i = 0; i < count; i += 2) {
            _free(ptrs[i]);
        }

        // Замеряем, сколько времени займет выделение 1000 новых блоков
        // в такой "дырявой" памяти (First-Fit будет искать место всё дольше)
        clock_t start = clock();
        for (int i = 0; i < 1000; i++) {
            VA tmp;
            _malloc(&tmp, 8);
        }
        clock_t end = clock();

        double ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
        printf("Узлов в списке: %5d | Время 1000 вызовов _malloc: %7.2f мс\n", count, ms);
        free(ptrs);
    }
}

int main() {
    srand(42); // Фиксируем сид для одинаковых условий
    test_thrashing();
    test_fragmentation();
    return 0;
}