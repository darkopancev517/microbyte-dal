#include "MicroByteDevice.h"
#include "MicroByteEvent.h"

MicroByteEvent::MicroByteEvent()
{
    this->node.next = NULL;
}

MicroByteEventQueue::MicroByteEventQueue()
{
    this->queue.next = NULL;
    this->scheduler = &MicroByteScheduler::get();
}

void MicroByteEventQueue::post(MicroByteEvent *event, MicroByteThread *thread)
{
    if (event == NULL || thread == NULL)
        return;

    unsigned state = microbyte_disable_irq();

    if (!event->node.next)
        queue.rightPush(&event->node);

    microbyte_restore_irq(state);
    scheduler->setThreadFlags(thread, MICROBYTE_EVENT_THREAD_FLAG);
}

void MicroByteEventQueue::cancel(MicroByteEvent *event)
{
    if (event == NULL)
        return;

    unsigned state = microbyte_disable_irq();
    queue.remove(&event->node);
    event->node.next = NULL;
    microbyte_restore_irq(state);
}

MicroByteEvent *MicroByteEventQueue::get()
{
    unsigned state = microbyte_disable_irq();
    MicroByteEvent *result = reinterpret_cast<MicroByteEvent *>(queue.leftPop());
    microbyte_restore_irq(state);
    if (result)
        result->node.next = NULL;
    return result;
}

MicroByteEvent *MicroByteEventQueue::wait()
{
    MicroByteEvent *result = NULL;
#ifdef UNITTEST
    unsigned state = microbyte_disable_irq();
    result = reinterpret_cast<MicroByteEvent *>(queue.leftPop());
    microbyte_restore_irq(state);
    if (result == NULL)
        scheduler->waitAnyThreadFlags(MICROBYTE_EVENT_THREAD_FLAG);
#else
    do
    {
        unsigned state = microbyte_disable_irq();
        result = reinterpret_cast<MicroByteEvent *>(queue.leftPop());
        microbyte_restore_irq(state);
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
