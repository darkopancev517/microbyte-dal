#include "MicroByteConfig.h"
#include "PacketBuffer.h"
#include "ErrorNo.h"

PacketBuffer PacketBuffer::EmptyPacket = PacketBuffer(1);

PacketBuffer::PacketBuffer()
{
    this->init(nullptr, 0, 0);
}

PacketBuffer::PacketBuffer(int length)
{
    this->init(nullptr, length, 0);
}

PacketBuffer::PacketBuffer(uint8_t *data, int length, int rssi)
{
    this->init(data, length, rssi);
}

PacketBuffer::PacketBuffer(const PacketBuffer &buffer)
{
    ptr = buffer.ptr;
    ptr->incr();
}

void PacketBuffer::init(uint8_t *data, int length, int rssi)
{
    if (length < 0)
        length = 0;

    ptr = (PacketData *) malloc(sizeof(PacketData) + length);
    ptr->init();

    ptr->length = length;
    ptr->rssi = rssi;

    if (data)
        memcpy(ptr->payload, data, length);
}

PacketBuffer::~PacketBuffer()
{
    ptr->decr();
}

PacketBuffer& PacketBuffer::operator = (const PacketBuffer &p)
{
    if (ptr == p.ptr)
        return *this;

    ptr->decr();
    ptr = p.ptr;
    ptr->incr();

    return *this;
}

uint8_t PacketBuffer::operator [] (int i) const
{
    return ptr->payload[i];
}

uint8_t& PacketBuffer::operator [] (int i)
{
    return ptr->payload[i];
}

bool PacketBuffer::operator== (const PacketBuffer& p)
{
    if (ptr == p.ptr)
        return true;
    else
        return (ptr->length == p.ptr->length && (memcpy(ptr->payload, p.ptr->payload, ptr->length)==0));
}

int PacketBuffer::setByte(int position, uint8_t value)
{
    if (position < ptr->length)
    {
        ptr->payload[position] = value;
        return MICROBYTE_OK;
    }
    else
    {
        return MICROBYTE_INVALID_PARAMETER;
    }
}

int PacketBuffer::getByte(int position)
{
    if (position < ptr->length)
        return ptr->payload[position];
    else
        return MICROBYTE_INVALID_PARAMETER;
}

uint8_t *PacketBuffer::getBytes()
{
    return ptr->payload;
}

int PacketBuffer::length()
{
    return ptr->length;
}

int PacketBuffer::getRSSI()
{
    return->ptr->rssi;
}

void PacketBuffer::setRSSI(uint8_t rssi)
{
    ptr->rssi = rssi;
}
