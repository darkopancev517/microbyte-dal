#include "MicroByteUtil.h"

KeyValueTableEntry *KeyValueTable::find(const uint32_t key) const
{
    // Now find the nearest sample range to that specified.
    KeyValueTableEntry *p = (KeyValueTableEntry *)data + (length - 1);
    KeyValueTableEntry *result = p;

    while (p >= (KeyValueTableEntry *)data) {
        if (p->key < key)
            break;
        result = p;
        p--;
    }

    return result;
}

uint32_t KeyValueTable::get(const uint32_t key) const
{
    return find(key)->value;
}

uint32_t KeyValueTable::getKey(const uint32_t key) const
{
    return find(key)->key;
}

bool KeyValueTable::hasKey(const uint32_t key) const
{
    return (find(key)->key == key);
}
