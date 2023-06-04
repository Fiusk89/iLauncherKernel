#include <string.h>

uint32_t strlen(char *str)
{
    uint32_t len = 0;
    while (*str++)
        len++;
    return len;
}

bool strncmp(const char *s1, const char *s2, unsigned int n)
{
    if (n == 0)
    {
        return true;
    }
    do
    {
        if (*s1 != *s2++)
        {
            return true;
        }
        if (*s1++ == 0)
        {
            break;
        }
    } while (--n != 0);
    return false;
}

uintptr_t uint(intptr_t num)
{
    if (num < 0)
        return (uintptr_t)-num;
    else
        return (uintptr_t)num;
}

float ufloat(float num)
{
    if (num < 0)
        return -num;
    else
        return num;
}

void *memcpy(void *dest, void *src, uint32_t n)
{
    uint32_t num_dwords = n / sizeof(uint64_t);
    uint32_t num_bytes = n % sizeof(uint64_t);
    uint64_t *dest64 = (uint64_t *)dest;
    uint8_t *dest8 = ((uint8_t *)dest) + num_dwords * sizeof(uint64_t);
    uint64_t *src64 = (uint64_t *)src;
    uint8_t *src8 = ((uint8_t *)src) + num_dwords * sizeof(uint64_t);
    for (uint32_t i = 0; i < num_dwords; i++)
        *dest64++ = *src64++;
    for (uint32_t i = 0; i < num_bytes; i++)
        *dest8++ = *src8++;
    return dest;
}

void *memset(void *dest, uint32_t val, uint32_t n)
{
    uint32_t num_dwords = n / sizeof(uint64_t);
    uint32_t num_bytes = n % sizeof(uint64_t);
    uint64_t *dest64 = (uint64_t *)dest;
    uint8_t *dest8 = ((uint8_t *)dest) + num_dwords * sizeof(uint64_t);
    uint8_t val8 = (uint8_t)val;
    uint64_t val64 = (val & 0xFFULL);
    val64 |= ((val64 << 8) & 0xFF00ULL);
    val64 |= ((val64 << 16) & 0xFFFF0000ULL);
    val64 |= ((val64 << 32) & 0xFFFFFFFF00000000ULL);
    for (uint32_t i = 0; i < num_dwords; i++)
        *dest64++ = val64;
    for (uint32_t i = 0; i < num_bytes; i++)
        *dest8++ = val8;
    return dest;
}

bool strcmp(char *str1, char *str2)
{
    if (strlen(str1) != strlen(str2))
        return true;
    else
        for (int i = 0; i < strlen(str1); i++)
            if (str1[i] != str2[i])
                return true;
    return false;
}

char *strcpy(char *str1, char *str2)
{
    int i = 0;
    int len = strlen(str2);
    for (i = 0; i < len; i++)
    {
        str1[i] = str2[i];
    }
    return str1;
}

char *strcat(char *dest, const char *src)
{
    uint32_t len1 = strlen(dest);
    uint32_t len2 = strlen(src);
    for (uint32_t i = 0; i < len2; i++)
        dest[i + len1] = src[i];
    dest[strlen(dest)] = '\0';
    return dest;
}

char *strncat(char *destination, const char *source, size_t num)
{
    char *ptr = destination + strlen(destination);
    while (*source != '\0' && num--)
    {
        *ptr++ = *source++;
    }
    *ptr = '\0';
    return destination;
}