#ifndef MICROBYTE_UNITTEST_H
#define MICROBYTE_UNITTEST_H

#include "MicroByteConfig.h"
#include "MicroByteDevice.h"

#ifdef __cplusplus
extern "C" {
#endif

void microbyte_set_in_isr(int state);

int microbyte_context_switch_triggered();

void microbyte_reset_context_switch_state();

#ifdef __cplusplus
}
#endif

#endif
