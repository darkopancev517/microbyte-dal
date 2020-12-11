#ifndef MICROBYTE_MUTEX_H
#define MICROBYTE_MUTEX_H

#include <stddef.h>
#include <stdint.h>

#include "MicroByteConfig.h"
#include "MicroByteCpu.h"
#include "MicroByteThread.h"
#include "CircList.h"

#define MICROBYTE_MUTEX_LOCKED ((CircList *)-1)

class Mutex
{
    CircList queue;

    MicroByteCpu *cpu;
    ThreadScheduler *scheduler;

    int setLock(int blocking);
    template <typename Type> inline Type &get() const;

    public:

    Mutex();

    Mutex(CircList *locked);

    int tryLock() { return setLock(0); }

    void lock() { setLock(1); }

    ThreadPid peek();

    void unlock();

    void unlockAndSleep();
};

#endif
