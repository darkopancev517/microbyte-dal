#ifndef MICROBYTE_COMPONENT_H
#define MICROBYTE_COMPONENT_H

#include "MicroByteConfig.h"

class MicroByteComponent
{
    protected:

    uint16_t id;
    uint8_t status;

    public:

    MicroByteComponent()
    {
        this->id = 0;
        this->status = 0;
    }

    virtual void systemTick()
    {
    }

    virtual void idleTick()
    {
    }

    virtual ~MicroByteComponent()
    {
    }
};

#endif
