#include <stddef.h>

#include "List.h"

List::List()
{
    next = nullptr;
}

void List::add(List *node)
{
    node->next = this->next;
    this->next = node;
}

List *List::removeHead()
{
    List *head = this->next;

    if (head != nullptr)
    {
        this->next = head->next;
    }

    return head;
}

List *List::remove(List *list, List *node)
{
    while (list->next)
    {
        if (list->next == node)
        {
            list->next = node->next;
            return node;
        }
        list = list->next;
    }

    return list->next;
}
