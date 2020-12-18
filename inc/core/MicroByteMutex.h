#ifndef MICROBYTE_MUTEX_H
#define MICROBYTE_MUTEX_H

#include <stddef.h>
#include <stdint.h>

#include "MicroByteConfig.h"
#include "MicroByteThread.h"
#include "CircList.h"

#define MICROBYTE_MUTEX_LOCKED ((CircList *)-1)

#define MICROBYTE_MUTEX_INIT_UNLOCKED   0
#define MICROBYTE_MUTEX_INIT_LOCKED     1

class MicroByteMutex
{
    CircList queue;

    int setLock(int blocking);
    template <typename Type> inline Type &get() const;

    public:

    MicroByteMutex(int initLocked = MICROBYTE_MUTEX_INIT_LOCKED);

    int tryLock() { return setLock(0); }

    void lock() { setLock(1); }

    MicroBytePid peek();

    void unlock();

    void unlockAndSleep();
};

#endif
