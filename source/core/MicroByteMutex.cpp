#include "MicroByteMutex.h"

Mutex::Mutex()
    : queue()
{
    this->cpu = uByteCpu;
    this->scheduler = &ThreadScheduler::get();
}

Mutex::Mutex(CircList *locked)
    : queue()
{
    this->queue.next = locked;
    this->cpu = uByteCpu;
    this->scheduler = &ThreadScheduler::get();
}

int Mutex::setLock(int blocking)
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
        Thread *curThread = scheduler->activeThread();
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

ThreadPid Mutex::peek()
{
    unsigned state = cpu->disableIrq();
    if (queue.next == NULL || queue.next == MICROBYTE_MUTEX_LOCKED)
    {
        cpu->restoreIrq(state);
        return MICROBYTE_THREAD_PID_UNDEF;
    }
    Thread *thread = Thread::get(queue.next);
    cpu->restoreIrq(state);
    return thread->pid;
}

void Mutex::unlock()
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
    Thread *thread = Thread::get(head);
    scheduler->setThreadStatus(thread, MICROBYTE_THREAD_STATUS_PENDING);
    if (!queue.next)
    {
        queue.next = MICROBYTE_MUTEX_LOCKED;
    }
    cpu->restoreIrq(state);
    scheduler->contextSwitch(thread->priority);
}

void Mutex::unlockAndSleep()
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
            Thread *thread = Thread::get(head);
            scheduler->setThreadStatus(thread, MICROBYTE_THREAD_STATUS_PENDING);
            if (!queue.next)
            {
                queue.next = MICROBYTE_MUTEX_LOCKED;
            }
        }
    }
    cpu->restoreIrq(state);
    scheduler->sleep();
}
