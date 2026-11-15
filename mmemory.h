#ifndef MMEMORY_H
#define MMEMORY_H

typedef int VA;

int init_memory(int n, int szPage);

int _malloc(VA* ptr, size_t szBlock);

int _free(VA ptr);

int _read(VA ptr, void* pBuffer, size_t szBuffer);

int _write(VA ptr, void* pBuffer, size_t szBuffer);

#endif