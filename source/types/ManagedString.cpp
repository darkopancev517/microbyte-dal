#include <string.h>
#include <stdlib.h>

#include "mbed.h"
#include "MicroByteConfig.h"
#include "ManagedString.h"
#include "MicroByteCompat.h"

static const char empty[] __attribute__ ((aligned (4))) = "\xff\xff\0\0\0";

void ManagedString::initEmpty()
{
    ptr = (StringData *)(void*)empty;
}

void ManagedString::initString(const char *str)
{
    int len = strlen(str);
    ptr = (StringData *)malloc(4+len+1);
    ptr->init();
    ptr->len = len;
    memcpy(ptr->data, str, len+1);
}

ManagedString::ManagedString(StringData *p)
{
    if (p == nullptr) {
        initEmpty();
        return;
    }
    ptr = p;
    ptr->incr();
}

StringData* ManagedString::leakData()
{
    StringData *res = ptr;
    initEmpty();
    return res;
}

ManagedString::ManagedString(const int value)
{
    char str[12];
    itoa(value, str);
    initString(str);
}

ManagedString::ManagedString(const char value)
{
    char str[2] = {value, 0};
    initString(str);
}

ManagedString::ManagedString(const char *str)
{
    // Sanity check. Return EmptyString for anything distasteful
    if (str == NULL || *str == 0) {
        initEmpty();
        return;
    }
    initString(str);
}

ManagedString::ManagedString(const ManagedString &s1, const ManagedString &s2)
{
    // Calculate length of new string.
    int len = s1.length() + s2.length();
    // Create a new buffer for holding the new string data.
    ptr = (StringData*) malloc(4+len+1);
    ptr->init();
    ptr->len = len;
    // Enter the data, and terminate the string.
    memcpy(ptr->data, s1.toCharArray(), s1.length());
    memcpy(ptr->data + s1.length(), s2.toCharArray(), s2.length());
    ptr->data[len] = 0;
}

ManagedString::ManagedString(PacketBuffer buffer)
{
    if (buffer.length() == 0) {
        initEmpty();
        return;
    }
    // Allocate a new buffer ( just in case the data is not NULL terminated).
    ptr = (StringData*) malloc(4+buffer.length()+1);
    ptr->init();

    // Store the length of the new string
    ptr->len = buffer.length();
    memcpy(ptr->data, buffer.getBytes(), buffer.length());
    ptr->data[buffer.length()] = 0;
}

ManagedString::ManagedString(const char *str, const int16_t length)
{
    // Sanity check. Return EmptyString for anything distasteful
    if (str == NULL || *str == 0 || length == 0 || (uint16_t)length > strlen(str)) {
        initEmpty();
        return;
    }
    // Allocate a new buffer, and create a NULL terminated string.
    ptr = (StringData*) malloc(4+length+1);
    ptr->init();
    // Store the length of the new string
    ptr->len = length;
    memcpy(ptr->data, str, length);
    ptr->data[length] = 0;
}

ManagedString::ManagedString(const ManagedString &s)
{
    ptr = s.ptr;
    ptr->incr();
}

ManagedString::ManagedString()
{
    initEmpty();
}

ManagedString::~ManagedString()
{
    ptr->decr();
}

ManagedString& ManagedString::operator = (const ManagedString& s)
{
    if (this->ptr == s.ptr)
        return *this;

    ptr->decr();
    ptr = s.ptr;
    ptr->incr();

    return *this;
}

bool ManagedString::operator== (const ManagedString& s)
{
    return ((length() == s.length()) && (strcmp(toCharArray(),s.toCharArray())==0));
}

bool ManagedString::operator< (const ManagedString& s)
{
    return (strcmp(toCharArray(), s.toCharArray())<0);
}

bool ManagedString::operator> (const ManagedString& s)
{
    return (strcmp(toCharArray(), s.toCharArray())>0);
}

ManagedString ManagedString::substring(int16_t start, int16_t length)
{
    // If the parameters are illegal, just return a reference to the empty string.
    if (start >= this->length())
        return ManagedString(ManagedString::EmptyString);

    // Compute a safe copy length;
    length = min(this->length()-start, length);

    // Build a ManagedString from this.
    return ManagedString(toCharArray()+start, length);
}

ManagedString operator+ (const ManagedString& lhs, const ManagedString& rhs)
{

    // If the either string is empty, nothing to do!
    if (rhs.length() == 0)
        return lhs;

    if (lhs.length() == 0)
        return rhs;

    return ManagedString(lhs, rhs);
}

char ManagedString::charAt(int16_t index)
{
    return (index >=0 && index < length()) ? ptr->data[index] : 0;
}

ManagedString ManagedString::EmptyString((StringData*)(void*)empty);
