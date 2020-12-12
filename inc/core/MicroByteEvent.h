#ifndef MICROBYTE_EVENT_H
#define MICROBYTE_EVENT_H

#include <stddef.h>
#include <stdint.h>

#include "MicroByteConfig.h"
#include "MicroByteThread.h"

#define MICROBYTE_EVENT_THREAD_FLAG (0x1)

class MicroByteEvent
{
    public:

    CircList node;

    MicroByteEvent();
};

class MicroByteEventQueue
{
    CircList queue;

    public:

    MicroByteEventQueue();

    void post(MicroByteEvent *event, MicroByteThread *thread);

    void cancel(MicroByteEvent *event);

    MicroByteEvent *get();

    MicroByteEvent *wait();

    int release(MicroByteEvent *event);

    int pending();

    MicroByteEvent *peek();
};

#endif
