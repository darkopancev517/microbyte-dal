#ifndef REF_COUNTED_H
#define REF_COUNTED_H

#include "mbed.h"
#include "MicroByteConfig.h"

struct RefCounted
{
public:
    uint16_t refCount;

    void incr();

    void decr();

    void init();

    bool isReadOnly();
};

#endif
