#ifndef MICROBYTE_MSG_H
#define MICROBYTE_MSG_H

#include <stddef.h>
#include <stdint.h>

#include "MicroByteConfig.h"
#include "MicroByteCpu.h"
#include "MicroByteThread.h"

class MicroByteMsg
{
    MicroByteCpu *cpu;
    MicroByteScheduler *scheduler;

    int send(MicroBytePid targetPid, int blocking, unsigned state);
    int receive(int blocking);

    public:

    MicroBytePid senderPid;
    uint16_t type;
    union {
        void *ptr;
        uint32_t value;
    } content;

    MicroByteMsg();

    int send(MicroBytePid targetPid);

    int trySend(MicroBytePid targetPid);

    int sendToSelf();

    int sendInIsr(MicroBytePid targetPid);

    int sentByIsr();

    int receive();

    int tryReceive();

    int sendReceive(MicroByteMsg *reply, MicroBytePid targetPid);

    int reply(MicroByteMsg *reply);

    int replyInIsr(MicroByteMsg *reply);
};

#endif
