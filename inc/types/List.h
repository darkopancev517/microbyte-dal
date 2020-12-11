#ifndef LIST_H
#define LIST_H

class List
{
    public:

    List *next;

    List();

    void add(List *node);

    List *removeHead();

    static List *remove(List *list, List *node);
};

#endif /* LIST_H */
