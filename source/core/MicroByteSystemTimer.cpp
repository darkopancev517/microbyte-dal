#include "MicroByteConfig.h"
#include "MicroByteSystemTimer.h"
#include "ErrorNo.h"

static uint64_t time_us = 0;
static unsigned int tick_period = 0;
static MicroByteComponent *systemTickComponents[MICROBYTE_SYSTEM_COMPONENTS];
static Ticker *ticker = nullptr;
static Timer *timer = nullptr;

int system_timer_init(int period)
{
    if (ticker == nullptr)
        ticker = new Ticker();

    if (timer == nullptr)
    {
        timer = new Timer();
        timer->start();
    }

    return system_timer_set_period(period);
}

int system_timer_set_period(int period)
{
    if (period < 1)
        return MICROBYTE_INVALID_PARAMETER;

    if (tick_period)
        ticker->detach();

    tick_period = period;
    ticker->attach_us(system_timer_tick, period * 1000);

    return MICROBYTE_OK;
}

int system_timer_get_period()
{
    return tick_period;
}

void update_time()
{
    if (timer == nullptr || ticker == nullptr)
        system_timer_init(MICROBYTE_SYSTEM_TICK_PERIOD_MS);

    time_us += timer->read_us();
    timer->reset();
}

uint64_t system_timer_current_time()
{
    return system_timer_current_time_us() / 1000;
}

uint64_t system_timer_current_time_us()
{
    update_time();
    return time_us;
}

void system_timer_tick()
{
    update_time();

    for (int i = 0; i < MICROBYTE_SYSTEM_COMPONENTS; i++)
        if (systemTickComponents[i] != nullptr)
            systemTickComponents[i]->systemTick();
}

int system_timer_add_component(MicroByteComponent *component)
{
    int i = 0;

    if (timer == nullptr || ticker == nullptr)
        system_timer_init(MICROBYTE_SYSTEM_TICK_PERIOD_MS);

    while (systemTickComponents[i] != nullptr && i < MICROBYTE_SYSTEM_COMPONENTS)
        i++;

    if (i == MICROBYTE_SYSTEM_COMPONENTS)
        return MICROBYTE_NO_RESOURCES;

    systemTickComponents[i] = component;
    return MICROBYTE_OK;
}

int system_timer_remove_component(MicroByteComponent *component)
{
    int i = 0;

    while (systemTickComponents[i] != component && i < MICROBYTE_SYSTEM_COMPONENTS)
        i++;

    if (i == MICROBYTE_SYSTEM_COMPONENTS)
        return MICROBYTE_INVALID_PARAMETER;

    systemTickComponents[i] = nullptr;

    return MICROBYTE_OK;
}
