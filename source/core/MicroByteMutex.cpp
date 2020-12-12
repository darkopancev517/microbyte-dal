#include "MicroByteDevice.h"
#include "MicroByteMutex.h"

MicroByteMutex::MicroByteMutex()
    : queue()
{
    this->scheduler = &MicroByteScheduler::get();
}

MicroByteMutex::MicroByteMutex(CircList *locked)
    : queue()
{
    this->queue.next = locked;
    this->scheduler = &MicroByteScheduler::get();
}

int MicroByteMutex::setLock(int blocking)
{
    unsigned state = microbyte_disable_irq();
    if (queue.next == NULL)
    {
        queue.next = MICROBYTE_MUTEX_LOCKED;
        microbyte_restore_irq(state);
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
        microbyte_restore_irq(state);
        microbyte_trigger_context_switch();
        return 1;
    }
    else
    {
        microbyte_restore_irq(state);
        return 0;
    }
}

MicroBytePid MicroByteMutex::peek()
{
    unsigned state = microbyte_disable_irq();
    if (queue.next == NULL || queue.next == MICROBYTE_MUTEX_LOCKED)
    {
        microbyte_restore_irq(state);
        return MICROBYTE_THREAD_PID_UNDEF;
    }
    MicroByteThread *thread = MicroByteThread::get(queue.next);
    microbyte_restore_irq(state);
    return thread->pid;
}

void MicroByteMutex::unlock()
{
    unsigned state = microbyte_disable_irq();
    if (queue.next == NULL)
    {
        microbyte_restore_irq(state);
        return;
    }
    if (queue.next == MICROBYTE_MUTEX_LOCKED)
    {
        queue.next = NULL;
        microbyte_restore_irq(state);
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
    microbyte_restore_irq(state);
    scheduler->contextSwitch(thread->priority);
}

void MicroByteMutex::unlockAndSleep()
{
    unsigned state = microbyte_disable_irq();
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
    microbyte_restore_irq(state);
    scheduler->sleep();
}
