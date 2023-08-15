#pragma once
#ifndef RLE_H
#define RLE_H
#ifndef KERNEL_H
#include "global.h"
#else
#include <kernel.h>
#endif

static uint64_t rle_get_size(void *src, uint64_t size)
{
    if (!src || !size)
        return 0;
    uint64_t rle_size = 2;
    uint8_t *src8 = (uint8_t *)src;
    uint8_t old_byte = *src8, new_byte = *src8, num_bytes = 0;
    for (uint64_t i = 0; i < size; i++)
    {
        new_byte = *src8++;
        if (new_byte != old_byte)
        {
            rle_size += 2;
        }
        else
        {
            if (num_bytes < 255)
            {
                num_bytes++;
            }
            else
            {
                num_bytes = 0, new_byte++;
                continue;
            }
        }
        old_byte = new_byte;
    }
    return rle_size;
}

static void rle_compress(void *dest, void *src, uint64_t size)
{
    if (!dest || !src || !size)
        return;
    uint8_t *dest8 = (uint8_t *)dest, *src8 = (uint8_t *)src;
    uint8_t old_byte = *src8, new_byte = *src8, written_byte = false;
    for (uint64_t i = 0; i < size; i++)
    {
        new_byte = *src8++;
        if (new_byte != old_byte)
        {
            dest8 += 2, written_byte = false;
        }
        else
        {
            if (dest8[0] < 255)
            {
                dest8[0]++;
                if (!written_byte)
                    dest8[1] = new_byte, written_byte = true;
            }
            else
            {
                new_byte++;
                continue;
            }
        }
        old_byte = new_byte;
    }
}

static void rle_decompress(void *dest, void *src, uint64_t size)
{
    if (!dest || !src || !size)
        return;
    uint8_t *dest8 = (uint8_t *)dest, *src8 = (uint8_t *)src;
    for (uint64_t i = 0; i < size; i++)
    {
        for (uint8_t num_bytes = 0; num_bytes < src8[0]; num_bytes++)
            *dest8++ = src8[1];
        src8 += 2;
    }
}
#endif