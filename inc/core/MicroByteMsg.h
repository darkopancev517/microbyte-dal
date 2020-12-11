#ifndef MICROBYTE_MSG_H
#define MICROBYTE_MSG_H

#include <stddef.h>
#include <stdint.h>

#include "MicroByteConfig.h"
#include "MicroByteCpu.h"

class Msg
{
    MicroByteCpu *cpu;
    ThreadScheduler *scheduler;

    int send(ThreadPid targetPid, int blocking, unsigned state);
    int receive(int blocking);

    public:

    ThreadPid senderPid;
    uint16_t type;
    union {
        void *ptr;
        uint32_t value;
    } content;

    Msg();

    int send(ThreadPid targetPid);

    int trySend(ThreadPid targetPid);

    int sendToSelf();

    int sendInIsr(ThreadPid targetPid);

    int sentByIsr();

    int receive();

    int tryReceive();

    int sendReceive(Msg *reply, ThreadPid targetPid);

    int reply(Msg *reply);

    int replyInIsr(Msg *reply);
};

#endif
