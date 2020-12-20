#ifndef MICROBYTE_HEAP_ALLOCATOR_H
#define MICROBYTE_HEAP_ALLOCATOR_H

#include "MicroByteConfig.h"

#define MICROBYTE_MAXIMUM_HEAPS     2

#define MICROBYTE_HEAP_BLOCK_FREE   0x80000000
#define MICROBYTE_HEAP_BLOCK_SIZE   4

struct HeapDefinition
{
    uint32_t *heap_start;
    uint32_t *heap_end;
};

int microbyte_create_heap(uint32_t start, uint32_t end);
void microbyte_heap_print();

#endif
