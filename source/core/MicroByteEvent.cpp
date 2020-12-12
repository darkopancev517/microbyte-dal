#include "MicroByteEvent.h"

MicroByteEvent::MicroByteEvent()
{
    this->node.next = NULL;
}

MicroByteEventQueue::MicroByteEventQueue()
{
    this->queue.next = NULL;
    this->cpu = uByteCpu;
    this->scheduler = &MicroByteScheduler::get();
}

void MicroByteEventQueue::post(MicroByteEvent *event, MicroByteThread *thread)
{
    if (event == NULL || thread == NULL)
        return;

    unsigned state = cpu->disableIrq();

    if (!event->node.next)
        queue.rightPush(&event->node);

    cpu->restoreIrq(state);
    scheduler->setThreadFlags(thread, MICROBYTE_EVENT_THREAD_FLAG);
}

void MicroByteEventQueue::cancel(MicroByteEvent *event)
{
    if (event == NULL)
        return;

    unsigned state = cpu->disableIrq();
    queue.remove(&event->node);
    event->node.next = NULL;
    cpu->restoreIrq(state);
}

MicroByteEvent *MicroByteEventQueue::get()
{
    unsigned state = cpu->disableIrq();
    MicroByteEvent *result = reinterpret_cast<MicroByteEvent *>(queue.leftPop());
    cpu->restoreIrq(state);
    if (result)
        result->node.next = NULL;
    return result;
}

MicroByteEvent *MicroByteEventQueue::wait()
{
    MicroByteEvent *result = NULL;
#ifdef UNITTEST
    unsigned state = cpu->disableIrq();
    result = reinterpret_cast<MicroByteEvent *>(queue.leftPop());
    cpu->restoreIrq(state);
    if (result == NULL)
        scheduler->waitAnyThreadFlags(MICROBYTE_EVENT_THREAD_FLAG);
#else
    do
    {
        unsigned state = cpu->disableIrq();
        result = reinterpret_cast<MicroByteEvent *>(queue.leftPop());
        cpu->restoreIrq(state);
        if (result == NULL)
            scheduler->waitAnyThreadFlags(MICROBYTE_EVENT_THREAD_FLAG);
    } while (result == NULL);
#endif
    return result;
}

int MicroByteEventQueue::release(MicroByteEvent *event)
{
    // Before releasing the event, make sure it's no longer in the event queue
    if (queue.find(reinterpret_cast<CircList *>(event)) == NULL)
    {
        event->node.next = NULL;
        return 1;
    }
    else
    {
        return -1;
    }
}

int MicroByteEventQueue::pending()
{
    return queue.count();
}

MicroByteEvent *MicroByteEventQueue::peek()
{
    return reinterpret_cast<MicroByteEvent *>(queue.leftPeek());
}
