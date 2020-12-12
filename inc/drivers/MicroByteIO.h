#ifndef MICROBYTE_IO_H
#define MICROBYTE_IO_H

#include "mbed.h"
#include "MicroByteConfig.h"
#include "MicroByteComponent.h"
#include "MicroBytePin.h"

class MicroByteIO
{
    public:

    //TODO:
    //MicroBytePin pin[0];
    //MicroBytePin P0;
    //MicroBytePin P1;
    //MicroBytePin P2;

    MicroByteIO(int ID_P0, int ID_P1, int ID_P2);
};

#endif
