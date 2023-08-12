#ifndef MATH_H
#define MATH_H
#include <ctype.h>
#define abs(a) (((a) < 0) ? -(a) : (a))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define limit(a, b) (((a) > (b)) ? (b) : (a))
#define sign(x) ((x < 0) ? -1 : ((x > 0) ? 1 : 0))

static int64_t pow(int64_t x, int32_t n)
{
    int64_t pow = 1;
    for (int32_t i = 0; i < n; i++)
    {
        pow = pow * x;
    }
    return pow;
}

static int64_t round(float x)
{
    if (x == 0)
        return 0;
    float diff = +x - (int64_t) + x;
    if (x < 0)
    {
        return (int64_t)(+diff >= 0.5 ? x + (1 - diff) : x + (-1 - diff));
    }
    else
    {
        return (int64_t)(diff >= 0.5 ? x + (1 - diff) : x - diff);
    }
}

static int64_t floor(float x)
{
    if (x == 0)
        return 0;
    float xcopy = (x < 0) ? (x * -1) : x;
    intptr_t zeros = 0;
    float n = 1;
    for (n = 1; xcopy > (n * 10); n *= 10, ++zeros)
        ;
    for (xcopy -= n; zeros != -1; xcopy -= n)
    {
        if (xcopy < 0)
        {
            xcopy += n;
            n /= 10;
            --zeros;
        }
    }
    xcopy += n;
    if (x < 0)
        return ((int64_t)((xcopy == 0) ? x : (x + xcopy - 1)));
    else
        return ((int64_t)(x - xcopy));
}

static float sqrt(int64_t number)
{
    int64_t start = 0, end = number;
    int64_t mid;
    float ans;
    while (start <= end)
    {
        mid = (start + end) / 2;
        if (mid * mid == number)
        {
            ans = mid;
            break;
        }
        if (mid * mid < number)
        {
            ans = start;
            start = mid + 1;
        }
        else
        {
            end = mid - 1;
        }
    }
    float increment = 0.1;
    for (int i = 0; i < 5; i++)
    {
        while (ans * ans <= number)
        {
            ans += increment;
        }
        ans = ans - increment;
        increment = increment / 10;
    }
    return ans;
}

static float clamp(float d, float min, float max)
{
    const float t = d < min ? min : d;
    return t > max ? max : t;
}

static int64_t clamp_int(int64_t d, int64_t min, int64_t max)
{
    const int64_t t = d < min ? min : d;
    return t > max ? max : t;
}

typedef struct Point
{
    int64_t x, y;
} Point_t;

typedef struct FPoint
{
    float x, y;
} FPoint_t;

typedef struct DPoint
{
    double x, y;
} DPoint_t;
#endif