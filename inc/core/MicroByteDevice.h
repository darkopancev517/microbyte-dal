#ifndef MICROBYTE_DEVICE_H
#define MICROBYTE_DEVICE_H

#include "MicroByteConfig.h"
#include "MicroByteThread.h"

unsigned microbyte_disable_irq();

unsigned microbyte_enable_irq();

void microbyte_restore_irq(unsigned state);

int microbyte_in_isr();

void microbyte_end_of_isr();

void microbyte_trigger_context_switch();

void microbyte_context_exit();

void microbyte_slee(int deep);

void microbyte_sleep_until_event();

void *microbyte_get_msp();

char *microbyte_stack_init(void *(*handler)(void *), void *arg, void *stack, int size);

#endif
