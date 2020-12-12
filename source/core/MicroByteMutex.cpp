#include "MicroByteMutex.h"

MicroByteMutex::MicroByteMutex()
    : queue()
{
    this->cpu = uByteCpu;
    this->scheduler = &MicroByteScheduler::get();
}

MicroByteMutex::MicroByteMutex(CircList *locked)
    : queue()
{
    this->queue.next = locked;
    this->cpu = uByteCpu;
    this->scheduler = &MicroByteScheduler::get();
}

int MicroByteMutex::setLock(int blocking)
{
    unsigned state = cpu->disableIrq();
    if (queue.next == NULL)
    {
        queue.next = MICROBYTE_MUTEX_LOCKED;
        cpu->restoreIrq(state);
        return 1;
    }
    else if (blocking)
    {
        MicroByteThread *curThread = scheduler->activeThread();
        scheduler->setThreadStatus(curThread, MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
        if (queue.next == MICROBYTE_MUTEX_LOCKED)
        {
            queue.next = &curThread->runQueueEntry;
            queue.next->next = NULL;
        }
        else
        {
            curThread->addTo(&queue);
        }
        cpu->restoreIrq(state);
        cpu->triggerContextSwitch();
        return 1;
    }
    else
    {
        cpu->restoreIrq(state);
        return 0;
    }
}

MicroBytePid MicroByteMutex::peek()
{
    unsigned state = cpu->disableIrq();
    if (queue.next == NULL || queue.next == MICROBYTE_MUTEX_LOCKED)
    {
        cpu->restoreIrq(state);
        return MICROBYTE_THREAD_PID_UNDEF;
    }
    MicroByteThread *thread = MicroByteThread::get(queue.next);
    cpu->restoreIrq(state);
    return thread->pid;
}

void MicroByteMutex::unlock()
{
    unsigned state = cpu->disableIrq();
    if (queue.next == NULL)
    {
        cpu->restoreIrq(state);
        return;
    }
    if (queue.next == MICROBYTE_MUTEX_LOCKED)
    {
        queue.next = NULL;
        cpu->restoreIrq(state);
        return;
    }
    CircList *head = queue.next;
    queue.next = head->next;    
    MicroByteThread *thread = MicroByteThread::get(head);
    scheduler->setThreadStatus(thread, MICROBYTE_THREAD_STATUS_PENDING);
    if (!queue.next)
    {
        queue.next = MICROBYTE_MUTEX_LOCKED;
    }
    cpu->restoreIrq(state);
    scheduler->contextSwitch(thread->priority);
}

void MicroByteMutex::unlockAndSleep()
{
    unsigned state = cpu->disableIrq();
    if (queue.next)
    {
        if (queue.next == MICROBYTE_MUTEX_LOCKED)
        {
            queue.next = NULL;
        }
        else
        {
            CircList *head = queue.next;
            queue.next = head->next;
            MicroByteThread *thread = MicroByteThread::get(head);
            scheduler->setThreadStatus(thread, MICROBYTE_THREAD_STATUS_PENDING);

            if (!queue.next)
                queue.next = MICROBYTE_MUTEX_LOCKED;
        }
    }
    cpu->restoreIrq(state);
    scheduler->sleep();
}
