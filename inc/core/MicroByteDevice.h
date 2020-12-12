#ifndef MICROBYTE_DEVICE_H
#define MICROBYTE_DEVICE_H

#include <stdint.h>
#include <stddef.h>

#include "MicroByteConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

unsigned microbyte_disable_irq();

unsigned microbyte_enable_irq();

void microbyte_restore_irq(unsigned state);

int microbyte_in_isr();

void microbyte_end_of_isr();

void microbyte_trigger_context_switch();

void microbyte_context_exit();

void microbyte_sleep(int deep);

void microbyte_sleep_until_event();

void *microbyte_get_msp();

char *microbyte_stack_init(void *(*handler)(void *), void *arg, void *stack, int size);

uint32_t microbyte_serial_number();

char *microbyte_friendly_name();

void microbyte_reset();

const char *microbyte_dal_version();

void microbyte_panic(int status_code);

void microbyte_panic_timeout(int iterations);

int microbyte_random(int max);

void microbyte_seed_random(uint32_t seed);

#ifdef __cplusplus
}
#endif

#endif
