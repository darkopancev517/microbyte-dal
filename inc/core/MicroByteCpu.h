#ifndef MICROBYTE_CPU_H
#define MICROBYTE_CPU_H

#include <stdint.h>

#include "MicroByteConfig.h"
#include "MicroByteThread.h"

class MicroByteCpu
{
    public:

    MicroByteCpu();

    virtual unsigned disableIrq()
    {
        return 0;
    }

    virtual unsigned enableIrq()
    {
        return 0;
    }

    virtual void restoreIrq(unsigned state)
    {
        (void)state;
    }

    virtual int inIsr()
    {
        return 0;
    }

    virtual void endOfIsr()
    {
    }

    virtual void triggerContextSwitch()
    {
    }

    virtual void contextExit()
    {
    }

    virtual void sleep(int deep)
    {
        (void)deep;
    }

    virtual void sleepUntilEvent()
    {
    }

    virtual void *getMsp()
    {
        return NULL;
    }

    virtual char *stackInit(MicroByteThreadHandler func, void *arg, void *stack, int size)
    {
        (void)func;
        (void)arg;
        (void)stack;
        (void)size;

        return NULL;
    }
};

extern MicroByteCpu *uByteCpu;

#endif
