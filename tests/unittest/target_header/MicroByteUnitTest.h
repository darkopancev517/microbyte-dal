#ifndef MICROBYTE_UNITTEST_H
#define MICROBYTE_UNITTEST_H

#include "MicroByteConfig.h"

class MicroByteCpuTest : public MicroByteCpu
{
    int inIsrState;
    int contextSwitchState;

    public:

    MicroByteCpuTest()
    {
        inIsrState = 0;
        contextSwitchState = 0;
    }

    unsigned disableIrq(void);

    unsigned enableIrq(void);

    void restoreIrq(unsigned state);

    int inIsr(void);

    void endOfIsr(void);

    void triggerContextSwitch(void);

    void contextExit(void);

    void sleep(int deep);

    void sleepUntilEvent(void);

    void *getMsp(void);

    char *stackInit(void *(*handler)(void*), void *arg, void *stack, int size);

    // Test helper functions

    void setInIsr(int state)
    {
        inIsrState = state;
    }

    int contextSwitchTriggered(void)
    {
        return contextSwitchState;
    }

    void resetContextSwitchState(void)
    {
        contextSwitchState = 0;
    }
};

#endif
