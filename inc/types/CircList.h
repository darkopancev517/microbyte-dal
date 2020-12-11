#ifndef CIRCLIST_H
#define CIRCLIST_H

#include <stddef.h>

class CircList
{
    public:

    CircList *next;

    CircList();

    void rightPush(CircList *node);

    void leftPush(CircList *node);

    CircList *leftPop();

    void leftPopRightPush();

    CircList *leftPeek();

    CircList *rightPeek();

    CircList *rightPop();

    CircList *findBefore(const CircList *node);

    CircList *find(const CircList *node);

    CircList *remove(CircList *node);

    size_t count();
};

#endif /* CIRCLIST_H */
