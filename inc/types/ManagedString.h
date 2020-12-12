#ifndef MANAGED_STRING_H
#define MANAGED_STRING_H

#include "MicroByteConfig.h"
#include "RefCounted.h"
#include "PacketBuffer.h"

struct StringData : RefCounted
{
    uint16_t len;
    char data[0];
};

class ManagedString
{
    StringData *ptr;

    public:

    ManagedString(StringData *ptr);

    StringData *leakData();

    ManagedString(const char *str);

    ManagedString(const int value);

    ManagedString(const char value);

    ManagedString(PacketBuffer buffer);

    ManagedString(const char *str, const int16_t length);

    ManagedString(const ManagedString &s);

    ManagedString();

    ~ManagedString();

    ManagedString& operator = (const ManagedString& s);

    bool operator== (const ManagedString& s);

    bool operator< (const ManagedString& s);

    bool operator> (const ManagedString& s);

    ManagedString substring(int16_t start, int16_t length);

    friend ManagedString operator+ (const ManagedString& lhs, const ManagedString& rhs);

    char charAt(int16_t index);

    const char *toCharArray() const
    {
        return ptr->data;
    }

    int length() const
    {
        return ptr->len;
    }

    static ManagedString EmptyString;

    private:

    void initEmpty();

    void initString(const char *str);

    ManagedString(const ManagedString &s1, const ManagedString &s2);
};

#endif
