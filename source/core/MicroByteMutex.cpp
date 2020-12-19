#include "MicroByteDevice.h"
#include "MicroByteMutex.h"

MicroByteMutex::MicroByteMutex(int initLocked)
    : queue()
{
    if (initLocked)
    {
        this->queue.next = MICROBYTE_MUTEX_LOCKED;
    }
}

int MicroByteMutex::setLock(int blocking)
{
    uint32_t irqmask = microbyte_disable_irq();
    MicroByteScheduler *scheduler = &MicroByteScheduler::get();
    if (queue.next == nullptr)
    {
        queue.next = MICROBYTE_MUTEX_LOCKED;
        microbyte_restore_irq(irqmask);
        return 1;
    }
    else if (blocking)
    {
        MicroByteThread *curThread = (MicroByteThread *)sched_active_thread;
        scheduler->setThreadStatus(curThread, MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
        if (queue.next == MICROBYTE_MUTEX_LOCKED)
        {
            queue.next = &curThread->runQueueEntry;
            queue.next->next = nullptr;
        }
        else
        {
            curThread->addTo(&queue);
        }
        microbyte_restore_irq(irqmask);
        microbyte_trigger_context_switch();
        return 1;
    }
    else
    {
        microbyte_restore_irq(irqmask);
        return 0;
    }
}

MicroBytePid MicroByteMutex::peek()
{
    uint32_t irqmask = microbyte_disable_irq();
    if (queue.next == nullptr || queue.next == MICROBYTE_MUTEX_LOCKED)
    {
        microbyte_restore_irq(irqmask);
        return MICROBYTE_THREAD_PID_UNDEF;
    }
    MicroByteThread *thread = MicroByteThread::get(queue.next);
    microbyte_restore_irq(irqmask);
    return thread->pid;
}

void MicroByteMutex::unlock()
{
    uint32_t irqmask = microbyte_disable_irq();
    MicroByteScheduler *scheduler = &MicroByteScheduler::get();
    if (queue.next == nullptr)
    {
        microbyte_restore_irq(irqmask);
        return;
    }
    if (queue.next == MICROBYTE_MUTEX_LOCKED)
    {
        queue.next = nullptr;
        microbyte_restore_irq(irqmask);
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
    microbyte_restore_irq(irqmask);
    scheduler->contextSwitch(thread->priority);
}

void MicroByteMutex::unlockAndSleep()
{
    uint32_t irqmask = microbyte_disable_irq();
    MicroByteScheduler *scheduler = &MicroByteScheduler::get();
    if (queue.next)
    {
        if (queue.next == MICROBYTE_MUTEX_LOCKED)
        {
            queue.next = nullptr;
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
    microbyte_restore_irq(irqmask);
    scheduler->sleep();
}
