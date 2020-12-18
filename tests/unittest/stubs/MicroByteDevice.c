#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#include "MicroByteUnitTest.h"

static int testDeviceInIrqState = 0;
static int testDeviceContextSwitchState = 0;

uint32_t microbyte_disable_irq()
{
    return 0;
}

uint32_t microbyte_enable_irq()
{
    return 0;
}

void microbyte_restore_irq(uint32_t irqmask)
{
    (void)irqmask;
}

int microbyte_in_isr()
{
    return testDeviceInIrqState;
}

void microbyte_end_of_isr()
{
}

void microbyte_trigger_context_switch()
{
    testDeviceContextSwitchState = 1;
}

void microbyte_context_exit()
{
}

void microbyte_sleep(int deep)
{
    (void)deep;
}

void microbyte_sleep_until_event()
{
}

void *microbyte_get_msp()
{
    return NULL;
}

char *microbyte_stack_init(void *(*handler)(void *), void *arg, void *stack, int size)
{
    (void)handler;
    (void)arg;
    (void)stack;
    (void)size;
    return NULL;
}

uint32_t microbyte_serial_number()
{
    return 0xdeadbeef;
}

char *microbyte_friendly_name()
{
    return "microbyte";
}

void microbyte_reset()
{
}

const char *microbyte_dal_version()
{
    return "0xdeadbeef";
}

void microbyte_panic(int status_code)
{
    (void)status_code;
}

void microbyte_panic_timeout(int iterations)
{
    (void)iterations;
}

int microbyte_random(int max)
{
    (void)max;
    return 0;
}

void microbyte_seed_random(uint32_t seed)
{
    (void)seed;
}

// test helper functions

void microbyte_set_in_isr(int state)
{
    testDeviceInIrqState = state;
}

int microbyte_context_switch_triggered()
{
    return testDeviceContextSwitchState;
}

void microbyte_reset_context_switch_state()
{
    testDeviceContextSwitchState = 0;
}
