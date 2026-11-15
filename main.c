#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mmemory.h"

int main() {
    printf("=== ЗАПУСК ТЕСТОВ МЕНЕДЖЕРА ВИРТУАЛЬНОЙ ПАМЯТИ ===\n\n");

    int n = 12;
    int page_size = 100;
    assert(init_memory(n, page_size) == 0);
    printf("[OK] Инициализация: %d вирт. страниц по %d байт (RAM = %d страниц)\n\n", n, page_size, n / 4);

    VA ptr1;
    const char *msg1 = "Hello, Virtual Memory!";
    size_t len1 = strlen(msg1) + 1;

    assert(_malloc(&ptr1, 50) == 0);
    assert(_write(ptr1, (void*)msg1, len1) == 0);

    char buf1[50] = {0};
    assert(_read(ptr1, buf1, len1) == 0);
    assert(strcmp(msg1, buf1) == 0);
    printf("[OK] Тест 1 (Базовый I/O): записано и прочитано \"%s\" по VA=%d\n", buf1, ptr1);

    VA ptr2;
    size_t big_size = 250;
    assert(_malloc(&ptr2, big_size) == 0);

    char write_big[250];
    for (size_t i = 0; i < big_size; i++) {
        write_big[i] = (char)('A' + (i % 26));
    }
    assert(_write(ptr2, write_big, big_size) == 0);

    char read_big[250] = {0};
    assert(_read(ptr2, read_big, big_size) == 0);
    assert(memcmp(write_big, read_big, big_size) == 0);
    printf("[OK] Тест 2 (Пересечение страниц): 250 байт успешно разбиты и собраны (VA=%d)\n", ptr2);

    VA blocks[5];
    for (int i = 0; i < 5; i++) {
        assert(_malloc(&blocks[i], 100) == 0);
        char temp[100];
        snprintf(temp, sizeof(temp), "Data inside LRU page #%d", i);
        assert(_write(blocks[i], temp, strlen(temp) + 1) == 0);
    }

    char check_lru[100] = {0};
    assert(_read(blocks[0], check_lru, 30) == 0);
    assert(strcmp(check_lru, "Data inside LRU page #0") == 0);

    char check_first[50] = {0};
    assert(_read(ptr1, check_first, len1) == 0);
    assert(strcmp(check_first, msg1) == 0);
    printf("[OK] Тест 3 (LRU и Dirty Write-Back): вытесненные страницы восстановлены без потерь!\n");

    assert(_free(blocks[1]) == 0);
    assert(_free(blocks[2]) == 0);
    assert(_free(blocks[1]) == -1);
    assert(_write(blocks[1], "hack", 5) == -1);

    VA merged_ptr;
    assert(_malloc(&merged_ptr, 200) == 0);
    assert(merged_ptr == blocks[1]);
    printf("[OK] Тест 4 (Защита и дефрагментация): соседние блоки слились в один на 200 байт (VA=%d)\n", merged_ptr);

    printf("\n=== ВСЕ ТЕСТЫ УСПЕШНО ПРОЙДЕНЫ! ===\n");
    return 0;
}