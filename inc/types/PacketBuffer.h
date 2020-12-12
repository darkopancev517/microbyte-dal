#ifndef MICROBYTE_PACKET_BUFFER_H
#define MICROBYTE_PACKET_BUFFER_H

#include "mbed.h"
#include "MicroByteConfig.h"
#include "RefCounted.h"

struct PacketData : RefCounted
{
    int rssi;
    uint8_t length;
    uint8_t payload[0];
};

class PacketBuffer
{
    PacketData *ptr;

    public:

    uint8_t *getBytes();

    PacketBuffer();

    PacketBuffer(int length);

    PacketBuffer(uint8_t *data, int length, int rssi = 0);

    PacketBuffer(const PacketBuffer &buffer);

    void init(uint8_t *data, int length, int rssi);

    ~PacketBuffer();

    PacketBuffer& operator = (const PacketBuffer& p);

    uint8_t operator [] (int i) const;

    uint8_t& operator [] (int i);

    bool operator== (const PacketBuffer& p);

    int setByte(int position, uint8_t value);

    int getByte(int position);

    int length();

    int getRSSI();

    void setRSSI(uint8_t rssi);

    static PacketBuffer EmptyPacket;
};

#endif
