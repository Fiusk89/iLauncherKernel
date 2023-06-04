#ifndef CTYPE_H
#define CTYPE_H
#define bool _Bool
#define true 1
#define false 0
#define NULL false

typedef union uint24
{
    struct
    {
        unsigned char r, g, b;
    } rgb;
    unsigned int value : 24 __attribute__((packed));
} uint24_t;

typedef union vga2b
{
    struct
    {
        unsigned char a, b;
    } u2b;
    unsigned short int value : 16 __attribute__((packed));
} vga2b_t;


typedef unsigned long long int uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short int uint16_t;
typedef unsigned char uint8_t;
typedef signed long long int int64_t;
typedef signed int int32_t;
typedef signed short int int16_t;
typedef signed char int8_t;
typedef uint64_t uintptr_t;
typedef uintptr_t size_t;
typedef int64_t intptr_t;
#endif