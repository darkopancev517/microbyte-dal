#include "MicroByteDevice.h"
#include "MicroByteMsg.h"

MicroByteMsg::MicroByteMsg()
    : senderPid(MICROBYTE_THREAD_PID_UNDEF)
    , type(0)
{
    this->content.ptr = nullptr;
    this->content.value = 0;
}

int MicroByteMsg::send(MicroBytePid targetPid, int blocking, uint32_t irqmask)
{
    MicroByteScheduler *scheduler = &MicroByteScheduler::get();
    MicroByteThread *targetThread = scheduler->threadFromContainer(targetPid);
    senderPid = sched_active_pid;
    if (targetThread == nullptr || !targetThread->hasMsgQueue()) {
        microbyte_restore_irq(irqmask);
        return -1;
    }
    MicroByteThread *curThread = (MicroByteThread *)sched_active_thread;
    if (targetThread->status != MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED) {
        if (targetThread->queuedMsg(this)) {
            microbyte_restore_irq(irqmask);
            if (curThread->getStatus() == MICROBYTE_THREAD_STATUS_REPLY_BLOCKED)
                microbyte_trigger_context_switch();
            return 1;
        }
        if (!blocking) {
            microbyte_restore_irq(irqmask);
            return 0;
        }
        curThread->waitData = static_cast<void *>(this);
        MicroByteThreadStatus newStatus;
        if (curThread->status == MICROBYTE_THREAD_STATUS_REPLY_BLOCKED) {
            newStatus = MICROBYTE_THREAD_STATUS_REPLY_BLOCKED;
        } else {
            newStatus = MICROBYTE_THREAD_STATUS_SEND_BLOCKED;
        }
        scheduler->setThreadStatus(curThread, newStatus);
        curThread->addTo(reinterpret_cast<CircList *>(&targetThread->msgWaiters));
        microbyte_restore_irq(irqmask);
        microbyte_trigger_context_switch();
    } else {
        MicroByteMsg *targetMsg = static_cast<MicroByteMsg *>(targetThread->waitData);
        *targetMsg = *this;
        scheduler->setThreadStatus(targetThread, MICROBYTE_THREAD_STATUS_PENDING);
        microbyte_restore_irq(irqmask);
        microbyte_trigger_context_switch();
    }
    return 1;
}

int MicroByteMsg::receive(int blocking)
{
    uint32_t irqmask = microbyte_disable_irq();
    MicroByteScheduler *scheduler = &MicroByteScheduler::get();
    MicroByteThread *curThread = (MicroByteThread *)sched_active_thread;

    if (curThread == nullptr || !curThread->hasMsgQueue()) {
        microbyte_restore_irq(irqmask);
        return -1;
    }

    int queueIndex = -1;

    if (curThread->msgArray != nullptr)
        queueIndex = curThread->msgQueue.get();

    if (!blocking && (!curThread->msgWaiters.next && queueIndex == -1)) {
        microbyte_restore_irq(irqmask);
        return -1;
    }

    if (queueIndex >= 0) {
        *this = *static_cast<MicroByteMsg *>(&curThread->msgArray[queueIndex]);
        microbyte_restore_irq(irqmask);
        return 1;
    } else {
        curThread->waitData = static_cast<void *>(this);
    }

    List *next = curThread->msgWaiters.removeHead();

    if (next == nullptr) {
        if (queueIndex < 0) {
            scheduler->setThreadStatus(curThread, MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED);
            microbyte_restore_irq(irqmask);
            microbyte_trigger_context_switch();
        } else {
            microbyte_restore_irq(irqmask);
        }
        return 1;
    } else {
        MicroByteThread *senderThread = MicroByteThread::get(reinterpret_cast<CircList *>(next));
        MicroByteMsg *tmp = nullptr;

        if (queueIndex >= 0)
            tmp = &curThread->msgArray[curThread->msgQueue.put()];

        MicroByteMsg *senderMsg = static_cast<MicroByteMsg *>(senderThread->waitData);
        if (tmp != nullptr) {
            *tmp = *senderMsg;
            *this = *tmp;
        } else {
            *this = *senderMsg;
        }

        uint8_t senderPrio = MICROBYTE_THREAD_PRIORITY_IDLE;

        if (senderThread->status != MICROBYTE_THREAD_STATUS_REPLY_BLOCKED) {
            senderThread->waitData = nullptr;
            scheduler->setThreadStatus(senderThread, MICROBYTE_THREAD_STATUS_PENDING);
            senderPrio = senderThread->priority;
        }

        microbyte_restore_irq(irqmask);

        if (senderPrio < MICROBYTE_THREAD_PRIORITY_IDLE)
            scheduler->contextSwitch(senderPrio);

        return 1;
    }
}

int MicroByteMsg::send(MicroBytePid targetPid)
{
    if (microbyte_in_isr())
        return sendInIsr(targetPid);

    if (sched_active_pid == targetPid)
        return sendToSelf();

    return send(targetPid, 1, microbyte_disable_irq());
}

int MicroByteMsg::trySend(MicroBytePid targetPid)
{
    if (microbyte_in_isr())
        return sendInIsr(targetPid);

    if (sched_active_pid == targetPid)
        return sendToSelf();

    return send(targetPid, 0, microbyte_disable_irq());
}

int MicroByteMsg::sendToSelf(void)
{
    uint32_t irqmask = microbyte_disable_irq();
    MicroByteThread *curThread = (MicroByteThread *)sched_active_thread;
    if (curThread == nullptr || !curThread->hasMsgQueue()) {
        microbyte_restore_irq(irqmask);
        return -1;
    }
    senderPid = curThread->pid;
    int result = curThread->queuedMsg(this);
    microbyte_restore_irq(irqmask);
    return result;
}

int MicroByteMsg::sendInIsr(MicroBytePid targetPid)
{
    MicroByteScheduler *scheduler = &MicroByteScheduler::get();
    MicroByteThread *targetThread = scheduler->threadFromContainer(targetPid);

    if (targetThread == nullptr || !targetThread->hasMsgQueue())
        return -1;

    senderPid = MICROBYTE_THREAD_PID_ISR;

    if (targetThread->status == MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED) {
        MicroByteMsg *targetMsg = static_cast<MicroByteMsg *>(targetThread->waitData);
        *targetMsg = *this;
        scheduler->setThreadStatus(targetThread, MICROBYTE_THREAD_STATUS_PENDING);
        scheduler->requestContextSwitch();
        return 1;
    } else {
        return targetThread->queuedMsg(this);
    }
}

int MicroByteMsg::sentByIsr()
{
    return senderPid == MICROBYTE_THREAD_PID_ISR;
}

int MicroByteMsg::receive()
{
    return receive(1);
}

int MicroByteMsg::tryReceive()
{
    return receive(0);
}

int MicroByteMsg::sendReceive(MicroByteMsg *reply, MicroBytePid targetPid)
{
    uint32_t irqmask = microbyte_disable_irq();

    MicroByteScheduler *scheduler = &MicroByteScheduler::get();
    MicroByteThread *curThread = (MicroByteThread *)sched_active_thread;

    if (curThread->pid == targetPid || !curThread->hasMsgQueue())
        return -1;

    scheduler->setThreadStatus(curThread, MICROBYTE_THREAD_STATUS_REPLY_BLOCKED);
    curThread->waitData = static_cast<void *>(reply);
    *reply = *this;
    return reply->send(targetPid, 1, irqmask);
}

int MicroByteMsg::reply(MicroByteMsg *reply)
{
    uint32_t irqmask = microbyte_disable_irq();
    MicroByteScheduler *scheduler = &MicroByteScheduler::get();
    MicroByteThread *targetThread = scheduler->threadFromContainer(senderPid);
    if (targetThread == nullptr ||
        !targetThread->hasMsgQueue() ||
        targetThread->status != MICROBYTE_THREAD_STATUS_REPLY_BLOCKED) {
        microbyte_restore_irq(irqmask);
        return -1;
    }
    MicroByteMsg *targetMsg = static_cast<MicroByteMsg *>(targetThread->waitData);
    *targetMsg = *reply;
    scheduler->setThreadStatus(targetThread, MICROBYTE_THREAD_STATUS_PENDING);
    microbyte_restore_irq(irqmask);
    scheduler->contextSwitch(targetThread->priority);
    return 1;
}

int MicroByteMsg::replyInIsr(MicroByteMsg *reply)
{
    MicroByteScheduler *scheduler = &MicroByteScheduler::get();
    MicroByteThread *targetThread = scheduler->threadFromContainer(senderPid);
    if (targetThread == nullptr ||
        !targetThread->hasMsgQueue() ||
        targetThread->status != MICROBYTE_THREAD_STATUS_REPLY_BLOCKED) {
        return -1;
    }
    MicroByteMsg *targetMsg = static_cast<MicroByteMsg *>(targetThread->waitData);
    *targetMsg = *reply;
    scheduler->setThreadStatus(targetThread, MICROBYTE_THREAD_STATUS_PENDING);
    scheduler->requestContextSwitch();
    return 1;
}
