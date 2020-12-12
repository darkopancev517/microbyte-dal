#include "mbed.h"
#include "MicroByteConfig.h"
#include "RefCounted.h"

void RefCounted::init()
{
    refCount = 3;
}

static inline bool isReadOnlyInline(RefCounted *t)
{
    uint32_t refCount = t->refCount;

    if (refCount == 0xffff)
        return true;

    if (refCount == 1 || (refCount & 1) == 0)
        microbyte_panic(MICROBYTE_HEAP_ERROR);

    return false;
}
