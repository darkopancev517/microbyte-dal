#include "MicroByteDevice.h"
#include "MicroByteEvent.h"

MicroByteEvent::MicroByteEvent()
{
    this->node.next = NULL;
}

MicroByteEventQueue::MicroByteEventQueue()
{
    this->queue.next = NULL;
}

void MicroByteEventQueue::post(MicroByteEvent *event, MicroByteThread *thread)
{
    if (event == NULL || thread == NULL)
        return;

    MicroByteScheduler *scheduler = &MicroByteScheduler::get();

    uint32_t irqmask = microbyte_disable_irq();

    if (!event->node.next)
        queue.rightPush(&event->node);

    microbyte_restore_irq(irqmask);
    scheduler->setThreadFlags(thread, MICROBYTE_EVENT_THREAD_FLAG);
}

void MicroByteEventQueue::cancel(MicroByteEvent *event)
{
    if (event == NULL)
        return;

    uint32_t irqmask = microbyte_disable_irq();
    queue.remove(&event->node);
    event->node.next = NULL;
    microbyte_restore_irq(irqmask);
}

MicroByteEvent *MicroByteEventQueue::get()
{
    uint32_t irqmask = microbyte_disable_irq();
    MicroByteEvent *result = reinterpret_cast<MicroByteEvent *>(queue.leftPop());
    microbyte_restore_irq(irqmask);
    if (result)
        result->node.next = NULL;
    return result;
}

MicroByteEvent *MicroByteEventQueue::wait()
{
    MicroByteEvent *result = NULL;
    MicroByteScheduler *scheduler = &MicroByteScheduler::get();
#ifdef UNITTEST
    uint32_t irqmask = microbyte_disable_irq();
    result = reinterpret_cast<MicroByteEvent *>(queue.leftPop());
    microbyte_restore_irq(irqmask);
    if (result == NULL)
        scheduler->waitAnyThreadFlags(MICROBYTE_EVENT_THREAD_FLAG);
#else
    do
    {
        uint32_t irqmask = microbyte_disable_irq();
        result = reinterpret_cast<MicroByteEvent *>(queue.leftPop());
        microbyte_restore_irq(irqmask);
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
