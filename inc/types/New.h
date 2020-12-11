#ifndef NEW_H
#define NEW_H

#include <stddef.h>

inline void *operator new(size_t, void *p) throw()
{
    return p;
}

#endif /* NEW_H */
