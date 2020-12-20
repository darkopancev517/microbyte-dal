#ifndef MICROBIT_COMPAT_H
#define MICROBIT_COMPAT_H

#include "mbed.h"
#include "MicroByteConfig.h"

#define PI 3.14159265359

inline int min(int a, int b)
{
    return (a < b ? a : b);
}

inline int max(int a, int b)
{
    return (a > b ? a : b);
}

inline void *memclr(void *a, size_t b)
{
    return memset(a,0,b);
}

inline bool isdigit(char c)
{
    return (c > 47 && c < 58);
}

int string_reverse(char *s);

int itoa(int n, char *s);

#endif
