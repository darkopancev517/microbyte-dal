#ifndef MICROBYTE_EVENT_H
#define MICROBYTE_EVENT_H

#include <stddef.h>
#include <stdint.h>

#include "MicroByteConfig.h"
#include "MicroByteCpu.h"

#define MICROBYTE_EVENT_THREAD_FLAG (0x1)

class Event
{
    public:

    CircList node;

    Event();
};

class EventQueue
{
    CircList queue;
    MicroByteCpu *cpu;
    ThreadScheduler *scheduler;

    public:

    EventQueue();

    void post(Event *event, Thread *thread);

    void cancel(Event *event);

    Event *get();

    Event *wait();

    int release(Event *event);

    int pending();

    Event *peek();
};

#endif
