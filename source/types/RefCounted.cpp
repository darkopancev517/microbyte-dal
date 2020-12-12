#include "mbed.h"
#include "MicroByteConfig.h"
#include "MicroByteDevice.h"
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

bool RefCounted::isReadOnly()
{
    return isReadOnlyInline(this);
}

void RefCounted::incr()
{
    if (!isReadOnlyInline(this))
        refCount += 2;
}

void RefCounted::decr()
{
    if (isReadOnlyInline(this))
        return;

    refCount -= 2;

    if (refCount == 1)
        free(this);
}
