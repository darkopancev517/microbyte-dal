#include "MicroByteConfig.h"
#include "MicroByteHeapAllocator.h"
#include "MicroByteDevice.h"
#include "MicroByteCompat.h"
#include "ErrorNo.h"

extern "C" uint32_t __end__;
extern "C" uint32_t __StackLimit;

static HeapDefinition heap[MICROBYTE_MAXIMUM_HEAPS] = { };
static uint8_t heap_count = 0;

static uint32_t microbyte_heap_start = (uint32_t)&__end__;
static uint32_t microbyte_heap_end = (uint32_t)&__StackLimit;

void microbyte_heap_print(HeapDefinition &aHeap)
{
    (void)aHeap;
}

void microbyte_heap_print()
{
    for (int i = 0; i < heap_count; i++) {
        // if (SERIAL_DEBUG) SERIAL_DEBUG->printf("\nHEAP %d: \n", i);
        microbyte_heap_print(heap[i]);
    }
}

int microbyte_create_heap(uint32_t start, uint32_t end)
{
    HeapDefinition *h = &heap[heap_count];

    if (heap_count == MICROBYTE_MAXIMUM_HEAPS)
        return MICROBYTE_NO_RESOURCES;

    if (end <= start ||
        end - start < MICROBYTE_HEAP_BLOCK_SIZE*2 ||
        end % MICROBYTE_HEAP_BLOCK_SIZE != 0 || start % MICROBYTE_HEAP_BLOCK_SIZE != 0) {
        return MICROBYTE_INVALID_PARAMETER;
    }

    uint32_t irqmask = microbyte_disable_irq();

    h->heap_start = (uint32_t *)start;
    h->heap_end = (uint32_t *)end;

    *h->heap_start = MICROBYTE_HEAP_BLOCK_FREE | (((uint32_t) h->heap_end - (uint32_t) h->heap_start) / MICROBYTE_HEAP_BLOCK_SIZE);
    heap_count++;

    microbyte_restore_irq(irqmask);

#if CONFIG_ENABLED(MICROBYTE_DBG) && CONFIG_ENABLED(MICROBYTE_HEAP_DBG)
    microbyte_heap_print();
#endif

    return MICROBYTE_OK;
}

void *microbyte_malloc(size_t size, HeapDefinition &aHeap)
{
    uint32_t blockSize = 0;
    uint32_t blocksNeeded = size % MICROBYTE_HEAP_BLOCK_SIZE == 0 ? size / MICROBYTE_HEAP_BLOCK_SIZE : size / MICROBYTE_HEAP_BLOCK_SIZE + 1;
    uint32_t *block;
    uint32_t *next;

    if (size <= 0)
        return nullptr;

    // Account for the index block;
    blocksNeeded++;

    // Disable IRQ temporarily to ensure no race conditions!
    uint32_t irqmask = microbyte_disable_irq();

    // We implement a first fit algorithm with cache to handle rapid churn...
    // We also defragment free blocks as we search, to optimise this and future searches.
    block = aHeap.heap_start;
    while (block < aHeap.heap_end) {
        // If the block is used, then keep looking.
        if (!(*block & MICROBYTE_HEAP_BLOCK_FREE)) {
            block += *block;
            continue;
        }

        blockSize = *block & ~MICROBYTE_HEAP_BLOCK_FREE;

        // We have a free block. Let's see if the subsequent ones are too. If so, we can merge...
        next = block + blockSize;

        while (*next & MICROBYTE_HEAP_BLOCK_FREE) {
            if (next >= aHeap.heap_end)
                break;
            // We can merge!
            blockSize += (*next & ~MICROBYTE_HEAP_BLOCK_FREE);
            *block = blockSize | MICROBYTE_HEAP_BLOCK_FREE;

            next = block + blockSize;
        }

        // We have a free block. Let's see if it's big enough.
        // If so, we have a winner.
        if (blockSize >= blocksNeeded)
            break;

        // Otherwise, keep looking...
        block += blockSize;
    }

    // We're full!
    if (block >= aHeap.heap_end) {
        microbyte_restore_irq(irqmask);
        return nullptr;
    }

    // If we're at the end of memory or have very near match then mark the whole segment as in use.
    if (blockSize <= blocksNeeded+1 || block+blocksNeeded+1 >= aHeap.heap_end) {
        // Just mark the whole block as used.
        *block &= ~MICROBYTE_HEAP_BLOCK_FREE;
    } else {
        // We need to split the block.
        uint32_t *splitBlock = block + blocksNeeded;
        *splitBlock = blockSize - blocksNeeded;
        *splitBlock |= MICROBYTE_HEAP_BLOCK_FREE;
        *block = blocksNeeded;
    }

    // Enable Interrupts
    microbyte_restore_irq(irqmask);

    return block+1;
}

void *malloc(size_t size)
{
    static uint8_t initialised = 0;
    void *p = nullptr;

    if (!initialised) {
        heap_count = 0;
        if (microbyte_create_heap(microbyte_heap_start, microbyte_heap_end) == MICROBYTE_INVALID_PARAMETER) {
            // TODO: microbyte_panic(MICROBYTE_HEAP_ERROR);
        }
        initialised = 1;
    }

    // Assign the memory from the first heap created that has space.
    for (int i=0; i < heap_count; i++) {
        p = microbyte_malloc(size, heap[i]);
        if (p != nullptr)
            break;
    }

    if (p != nullptr) {
#if CONFIG_ENABLED(MICROBYTE_DBG) && CONFIG_ENABLED(MICROBYTE_HEAP_DBG)
        if(SERIAL_DEBUG) SERIAL_DEBUG->printf("malloc: ALLOCATED: %d [%p]\n", size, p);
#endif
        return p;
    }

    // We're totally out of options (and memory!).
#if CONFIG_ENABLED(MICROBYTE_DBG) && CONFIG_ENABLED(MICROBYTE_HEAP_DBG)
    if(SERIAL_DEBUG) SERIAL_DEBUG->printf("malloc: OUT OF MEMORY [%d]\n", size);
#endif

    // TODO: microbyte_panic(MICROBYTE_OOM);

    return nullptr;
}

void free(void *mem)
{
    uint32_t *memory = (uint32_t *)mem;
    uint32_t *cb = memory-1;

#if CONFIG_ENABLED(MICROBYTE_DBG) && CONFIG_ENABLED(MICROBYTE_HEAP_DBG)
    if (heap_count > 0)
        if(SERIAL_DEBUG) SERIAL_DEBUG->printf("free:   %p\n", mem);
#endif
    // Sanity check.
	  if (memory == nullptr)
        return;

    // If this memory was created from a heap registered with us, free it.
    for (int i=0; i < heap_count; i++) {
        if (memory > heap[i].heap_start && memory < heap[i].heap_end) {
            // The memory block given is part of this heap, so we can simply
            // flag that this memory area is now free, and we're done.
            if (*cb == 0 || *cb & MICROBYTE_HEAP_BLOCK_FREE) {
                // TODO: microbyte_panic(MICROBYTE_HEAP_ERROR);
            }
            *cb |= MICROBYTE_HEAP_BLOCK_FREE;
            return;
        }
    }
    // If we reach here, then the memory is not part of any registered heap.
    // TODO: microbyte_panic(MICROBYTE_HEAP_ERROR);
}

void *calloc(size_t num, size_t size)
{
    void *mem = malloc(num*size);
    if (mem) {
        memclr(mem, num*size);
    }
    return mem;
}

void *realloc(void* ptr, size_t size)
{
    void *mem = malloc(size);
    // handle the simplest case - no previous memory allocted.
    if (ptr != nullptr && mem != nullptr) {
        // Otherwise we need to copy and free up the old data.
        uint32_t *cb = ((uint32_t *)ptr) - 1;
        uint32_t blockSize = *cb & ~MICROBYTE_HEAP_BLOCK_FREE;
        memcpy(mem, ptr, min(blockSize * sizeof(uint32_t), size));
        free(ptr);
    }
    return mem;
}

// make sure the libc allocator is not pulled in
void *_malloc_r(struct _reent *, size_t len)
{
    return malloc(len);
}

void _free_r(struct _reent *, void *addr)
{
    free(addr);
}

void *_realloc_r(struct _reent *ptr, void *old, size_t newlen)
{
    (void) ptr;
    return realloc(old, newlen);
}
