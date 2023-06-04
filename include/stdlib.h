#ifndef STDLIB_H
#define STDLIB_H
#include <kernel.h>

uint64_t string2hash(char *);
uintptr_t rand();
uintptr_t srand(uintptr_t);
uint32_t atoi(const char *);
void *itoa(char *, int64_t, uint32_t);
char *ftoa(char *, float, uint32_t);
void sleep(uint32_t);
#endif