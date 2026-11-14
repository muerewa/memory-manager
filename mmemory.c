#include <stdio.h>
#include <stdlib.h>
#include "mmemory.h"

#include <string.h>

typedef struct Block {
    int va; // стартовый адрес
    size_t size;
    int free; // bool занято или нет
    struct Block *next;
} Block;

Block *head = NULL;

typedef struct Page {
    int frame_num;
    int present; // bool в озу или на диске
    int dirty; // bool меняли или нет
    unsigned long last_access; // таймер для LRU
} Page;

Page *pg = NULL;

char* ram = NULL; // Физическая память
char* disk = NULL;  // Виртуальная (вся) память
int page_size = 0;  // Размер страницы
int virt_pages = 0; // Кол-во виртуальных страниц
int ram_pages = 0;  // Кол-во физических страниц
unsigned long tick = 0; // Глобальные часы для LRU

int init_memory(int n, int pagesize) {
    page_size = pagesize;
    virt_pages = n;
    ram_pages = n / 4;
    disk = malloc(n * pagesize);
    ram = malloc(n * pagesize / 4);
    pg = calloc(n, sizeof(Page));
    for (int i = 0; i < n; i++) {
        pg[i].frame_num = -1;
    }
    head = malloc(sizeof(Block));
    head->next = NULL;
    head->va = 0;
    head->size = n * pagesize;
    head->free = 1;
    tick = 0;
    return 0;
}

int _malloc(VA* ptr, size_t sz) {
    Block *b = head;
    while (b != NULL) {
        if (b->free == 1 && b->size >= sz) {
            if (b->size > sz) {
                Block *remain = malloc(sizeof(Block));
                remain->va = b->va + sz;
                remain->size = b->size - sz;
                remain->free = 1;
                remain->next = b->next;
                b->next = remain;
            }
            b->free = 0;
            b->size = sz;
            *ptr = b->va;
            return 0;
        }
        b = b->next;
    }
    return -1;
}

int _free(VA ptr) {
    Block *b = head;
    Block *prev = NULL;
    while (b != NULL) {
        if (b->va == ptr) {
            if (b->free == 1) {
                return -1;
            }
            b->free = 1;
            if (b->next != NULL && b->next->free == 1) {
                Block *to_delete = b->next;
                b->size += to_delete->size;
                b->next = to_delete->next;
                free(to_delete);
            }
            if (prev != NULL && prev->free == 1) {
                prev->size += b->size;
                prev->next = b->next;
                free(b);
            }
            return 0;
        }
        prev = b;
        b = b->next;
    }
    return -1;
}

int swap_page(int v_page) {
    if (pg[v_page].present == 1) {
        pg[v_page].last_access = ++tick;
        return pg[v_page].frame_num;
    }
    int frame = -1;
    for (int i = 0; i < ram_pages; i++) {
        int isBusy = 0;
        for (int j = 0; j < virt_pages; j++) {
            if (pg[j].present == 1 && pg[j].frame_num == i) {
                isBusy = 1;
                break;
            }
        }
        if (isBusy == 0) {
            frame = i;
            break;
        }
    }
    if (frame == -1) {
        int lru_page = -1;
        unsigned long min_time = -1;
        for (int i = 0; i < virt_pages; i++) {
            if (pg[i].present == 1 && pg[i].last_access < min_time) {
                min_time = pg[i].last_access;
                lru_page = i;
            }
        }
        frame = pg[lru_page].frame_num;
        if (pg[lru_page].dirty == 1) {
            memcpy((char *)(disk + lru_page * page_size), (char *)(ram + pg[lru_page].frame_num * page_size), page_size);
        }
        pg[lru_page].present = 0;
    }
    memcpy((char*)(ram + frame *page_size), (char *)(disk + v_page * page_size), page_size);
    pg[v_page].present = 1;
    pg[v_page].frame_num = frame;
    pg[v_page].last_access = ++tick;
    pg[v_page].dirty = 0;
    return frame;
}