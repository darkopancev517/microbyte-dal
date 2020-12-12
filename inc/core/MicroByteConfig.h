#ifndef MICROBYTE_CONFIG_H
#define MICROBYTE_CONFIG_H

#include "mbed.h"

#ifndef MICROBYTE_DBG
#define MICROBYTE_DBG 0
#endif

#ifndef MICROBYTE_HEAP_DBG
#define MICROBYTE_HEAP_DBG 0
#endif

#ifndef MICROBYTE_THREAD_PRIO_LEVELS
#define MICROBYTE_THREAD_PRIO_LEVELS (16)
#endif

#ifndef MICROBYTE_THREAD_MAX
#define MICROBYTE_THREAD_MAX (32)
#endif

#ifndef MICROBYTE_MSG_QUEUE_SIZE
#define MICROBYTE_MSG_QUEUE_SIZE (4)
#endif

#define CONFIG_ENABLED(X) (X == 1)
#define CONFIG_DISABLED(X) (X != 1)

#if CONFIG_ENABLED(MICROBYTE_HEAP_ALLOCATOR)
#include "MicroByteHeapAllocator.h"
#endif

#endif
