#include <stddef.h>
#include <stdint.h>

#include "CircList.h"

CircList::CircList()
{
    next = nullptr;
}

void CircList::rightPush(CircList *node)
{
    if (this->next) {
        node->next = this->next->next;
        this->next->next = node;
    } else {
        node->next = node;
    }
    this->next = node;
}

void CircList::leftPush(CircList *node)
{
    if (this->next) {
        node->next = this->next->next;
        this->next->next = node;
    } else {
        node->next = node;
        this->next = node;
    }
}

CircList *CircList::leftPop()
{
    if (this->next) {
        CircList *first = this->next->next;
        if (this->next == first) {
            this->next = nullptr;
        } else {
            this->next->next = first->next;
        }
        return first;
    } else {
        return nullptr;
    }
}

void CircList::leftPopRightPush()
{
    if (this->next) {
        this->next = this->next->next;
    }
}

CircList *CircList::leftPeek()
{
    if (this->next) {
        return this->next->next;
    }

    return nullptr;
}

CircList *CircList::rightPeek()
{
    return this->next;
}

CircList *CircList::rightPop()
{
    if (this->next) {
        CircList *last = this->next;
        while (this->next->next != last) {
            this->leftPopRightPush();
        }
        return this->leftPop();
    } else {
        return nullptr;
    }
}

CircList *CircList::findBefore(const CircList *node)
{
    CircList *pos = this->next;
    if (!pos) {
        return nullptr;
    }

    do {
        pos = pos->next;
        if (pos->next == node) {
            return pos;
        }
    } while (pos != this->next);

    return nullptr;
}

CircList *CircList::find(const CircList *node)
{
    CircList *tmp = this->findBefore(node);
    if (tmp) {
        return tmp->next;
    } else {
        return nullptr;
    }
}

CircList *CircList::remove(CircList *node)
{
    if (this->next) {
        if (this->next->next == node) {
            return this->leftPop();
        } else {
            CircList *tmp = this->findBefore(node);
            if (tmp) {
                tmp->next = tmp->next->next;
                if (node == this->next) {
                    this->next = tmp;
                }
                return node;
            }
        }
    }
    return nullptr;
}

size_t CircList::count()
{
    CircList *node = this->next;
    size_t cnt = 0;
    if (node) {
        do {
            node = node->next;
            ++cnt;
        } while (node != this->next);
    }
    return cnt;
}
