#include "MicroByteDevice.h"
#include "MicroByteEvent.h"

MicroByteEvent::MicroByteEvent()
{
    this->node.next = nullptr;
}

MicroByteEventQueue::MicroByteEventQueue()
{
    this->queue.next = nullptr;
}

void MicroByteEventQueue::post(MicroByteEvent *event, MicroByteThread *thread)
{
    if (event == nullptr || thread == nullptr)
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
    if (event == nullptr)
        return;

    uint32_t irqmask = microbyte_disable_irq();
    queue.remove(&event->node);
    event->node.next = nullptr;
    microbyte_restore_irq(irqmask);
}

MicroByteEvent *MicroByteEventQueue::get()
{
    uint32_t irqmask = microbyte_disable_irq();
    MicroByteEvent *result = reinterpret_cast<MicroByteEvent *>(queue.leftPop());
    microbyte_restore_irq(irqmask);
    if (result)
        result->node.next = nullptr;
    return result;
}

MicroByteEvent *MicroByteEventQueue::wait()
{
    MicroByteEvent *result = nullptr;
    MicroByteScheduler *scheduler = &MicroByteScheduler::get();
#ifdef UNITTEST
    uint32_t irqmask = microbyte_disable_irq();
    result = reinterpret_cast<MicroByteEvent *>(queue.leftPop());
    microbyte_restore_irq(irqmask);
    if (result == nullptr)
        scheduler->waitAnyThreadFlags(MICROBYTE_EVENT_THREAD_FLAG);
#else
    do {
        uint32_t irqmask = microbyte_disable_irq();
        result = reinterpret_cast<MicroByteEvent *>(queue.leftPop());
        microbyte_restore_irq(irqmask);
        if (result == nullptr)
            scheduler->waitAnyThreadFlags(MICROBYTE_EVENT_THREAD_FLAG);
    } while (result == nullptr);
#endif
    return result;
}

int MicroByteEventQueue::release(MicroByteEvent *event)
{
    // Before releasing the event, make sure it's no longer in the event queue
    if (queue.find(reinterpret_cast<CircList *>(event)) == nullptr) {
        event->node.next = nullptr;
        return 1;
    } else {
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
