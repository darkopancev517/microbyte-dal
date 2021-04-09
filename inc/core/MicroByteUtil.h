#ifndef MICROBYTE_UTIL_H
#define MICROBYTE_UTIL_H

#include "MicroByteConfig.h"

#define CREATE_KEY_VALUE_TABLE(NAME, PAIRS) const KeyValueTable NAME { PAIRS, sizeof(PAIRS) / sizeof(KeyValueTableEntry) };

struct KeyValueTableEntry
{
    const uint32_t key;
    const uint32_t value;
};

struct KeyValueTable
{
    const KeyValueTableEntry *data;
    const int length;

    KeyValueTableEntry *find(const uint32_t key) const;
    uint32_t get(const uint32_t key) const;
    uint32_t getKey(const uint32_t key) const;
    bool hasKey(const uint32_t key) const;
};

#endif
