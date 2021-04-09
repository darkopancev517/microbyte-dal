#ifndef MICROBYTE_SYSTEM_TIMER_H
#define MICROBYTE_SYSTEM_TIMER_H

#include "mbed.h"
#include "MicroByteConfig.h"
#include "MicroByteComponent.h"

int system_timer_init(int period);

int system_timer_set_period(int period);

int system_timer_get_period();

inline void update_time();

uint64_t system_timer_current_time();

uint64_t system_timer_current_time_us();

void system_timer_tick();

int system_timer_add_component(MicroByteComponent *component);

int system_timer_remove_component(MicroByteComponent *component);

class MicroByteSystemTimerCallback : MicroByteComponent
{
    void (*fn)(void);

    public:
    MicroByteSystemTimerCallback(void (*function)(void))
    {
        fn = function;
        system_timer_add_component(this);
    }

    void systemTick()
    {
        fn();
    }
};

#endif
