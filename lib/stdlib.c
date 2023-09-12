#include <stdlib.h>

static uintptr_t rnext = 1;

uint64_t string2hash(char *string)
{
    uint64_t hash = 0;
    while (*string++ != '\0')
        hash = *string + 31 * hash;
    return hash;
}

uintptr_t rand(void)
{
    rnext = rnext * 1103515245 + 12345;
    return (uintptr_t)(rnext / 65536) % 32768;
}

uintptr_t srand(uintptr_t seed)
{
    rnext = seed;
}

uint32_t atoi(const char *str)
{
    uint32_t res = 0;
    uint32_t sign = 1;
    uint32_t i = 0;
    if (str[0] == '-')
    {
        sign = -1;
        i++;
    }
    for (; str[i] != '\0'; i++)
        res = res * 10 + str[i] - '0';
    return sign * res;
}

void *itoa(char *ptr, int64_t value, uint32_t n)
{
    if (!ptr)
        return NULL;
    int8_t size = 1;
    int8_t sign = 1;
    if (value < 0)
        sign = -1;
    if (sign == -1)
    {
        value = -value;
        ptr[size - size] = '-';
    }
    int64_t t = value;
    while (t / 10 != 0)
    {
        t /= 10;
        size++;
    }
    if (size > n)
        return NULL;
    ptr[size] = '\0';
    t = value;
    int32_t i = size - 1;
    while (i >= 0 && n >= 0)
    {
        if (sign < 0)
            ptr[i + 1] = (t % 10) + '0';
        else if (sign > 0)
            ptr[i] = (t % 10) + '0';
        t /= 10;
        i--;
        n--;
    }
    return ptr;
}

char *ftoa(char *ptr, float value, uint32_t n)
{
    if (!ptr)
        return NULL;
    if (n < 16)
        return NULL;
    itoa(ptr, (int64_t)value, n);
    if (value < 0)
        value = -value;
    uint32_t ptr_len = strlen(ptr);
    ptr[ptr_len] = '.';
    ptr[ptr_len + 1] = '\0';
    float tmpv = value - (int64_t)value;
    for (uint32_t i = 0; i < 8 - 1 && i < n - (ptr_len + 1); i++)
    {
        tmpv *= 10.0f;
        if (!tmpv)
            break;
        ptr[ptr_len + 1 + i] = (int64_t)tmpv + '0';
        if (i < (ptr_len + 2) - n)
            ptr[ptr_len + 2 + i] = '\0';
        tmpv -= (int64_t)tmpv;
    }
    if (!ptr[ptr_len + 1])
        ptr[ptr_len] = '\0';
    return ptr;
}

void sleep(uint32_t delay)
{
    pit_t start_ticks = current_task->active_time;
    while (current_task->active_time - start_ticks < delay)
        ;
}