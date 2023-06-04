#ifndef STRING_H
#define STRING_H
#include <kernel.h>

uint32_t strlen(char *str);
void reverse(char *s);
void *memcpy(void *dest, void *src, uint32_t n);
void *memset(void *dest, uint32_t val, uint32_t n);
bool strcmp(char *str1, char *str2);
bool strncmp(const char *s1, const char *s2, unsigned int n);
char *strcpy(char *str1, char *str2);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t num);
#endif