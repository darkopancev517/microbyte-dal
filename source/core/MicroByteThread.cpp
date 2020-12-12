#include "MicroByteDevice.h"
#include "MicroByteThread.h"
#include "MicroByteMsg.h"
#include "New.h"
#include "Utils.h"

DEFINE_ALIGNED_VAR(uByteScheduler, sizeof(MicroByteScheduler), uint64_t);

MicroByteScheduler &MicroByteScheduler::init()
{
    MicroByteScheduler *scheduler = &get();
    scheduler = new (&uByteScheduler) MicroByteScheduler();
    return *scheduler;
}

MicroByteScheduler &MicroByteScheduler::get()
{
    void *scheduler = &uByteScheduler;
    return *static_cast<MicroByteScheduler *>(scheduler);
}

MicroByteScheduler::MicroByteScheduler()
    : numOfThreadsInContainer(0)
    , contextSwitchRequest(0)
    , currentActiveThread(NULL)
    , currentActivePid(MICROBYTE_THREAD_PID_UNDEF)
    , runQueueBitCache(0)
{
    for (MicroBytePid i = MICROBYTE_THREAD_PID_FIRST; i <= MICROBYTE_THREAD_PID_LAST; ++i)
    {
        this->threadsContainer[i] = NULL;
    }
    for (uint8_t prio = 0; prio < MICROBYTE_THREAD_PRIO_LEVELS; prio++)
    {
        this->runQueue[prio].next = NULL;
    }
}

MicroByteThread::MicroByteThread()
    : stackPointer(NULL)
    , status(MICROBYTE_THREAD_STATUS_NOT_FOUND)
    , priority(MICROBYTE_THREAD_PRIORITY_IDLE)
    , pid(MICROBYTE_THREAD_PID_UNDEF)
    , runQueueEntry()
    , stackStart(NULL)
    , name(NULL)
    , stackSize(0)
    , flags(0)
    , waitFlags(0)
    , waitData(NULL)
    , msgWaiters()
    , msgQueue()
    , msgArray(NULL)
{
}

MicroByteThread *MicroByteThread::init(char *stack, int size, uint8_t prio, int flags,
                                       MicroByteThreadHandler func, void *arg, const char *name)
{
    if (prio >= MICROBYTE_THREAD_PRIO_LEVELS)
        return NULL;

    int totalStackSize = size;

    // Aligned the stack on 16/32 bit boundary
    uintptr_t misalignment = reinterpret_cast<uintptr_t>(stack) % 8;

    if (misalignment)
    {
        misalignment = 8 - misalignment;
        stack += misalignment;
        size -= misalignment;
    }

    // Make room for TCB
    size -= sizeof(MicroByteThread);

    // Round down the stacksize to multiple of MicroByteThread aligments (usually 16/32 bit)
    size -= size % 8;

    if (size < 0)
        return NULL;

    MicroByteThread *thread = new (stack + size) MicroByteThread();

    if (flags & MICROBYTE_THREAD_FLAGS_STACKMARKER)
    {
        // Assign each int of the stack the value of it's address, for test purposes
        uintptr_t *stackmax = reinterpret_cast<uintptr_t *>(stack + size);
        uintptr_t *stackp = reinterpret_cast<uintptr_t *>(stack);
        while (stackp < stackmax)
        {
            *stackp = reinterpret_cast<uintptr_t>(stackp);
            stackp++;
        }
    }
    else
    {
        // Create stack guard
        *(uintptr_t *)stack = reinterpret_cast<uintptr_t>(stack);
    }

    unsigned state = microbyte_disable_irq();

    MicroBytePid pid = MICROBYTE_THREAD_PID_UNDEF;

    MicroByteScheduler &scheduler = MicroByteScheduler::get();

    for (MicroBytePid i = MICROBYTE_THREAD_PID_FIRST; i <= MICROBYTE_THREAD_PID_LAST; ++i)
    {
        if (scheduler.threadFromContainer(i) == NULL)
        {
            pid = i;
            break;
        }
    }

    if (pid == MICROBYTE_THREAD_PID_UNDEF)
    {
        microbyte_restore_irq(state);
        return NULL;
    }

    scheduler.addThread(thread, pid);

    thread->pid = pid;
    thread->stackPointer = microbyte_stack_init(func, arg, stack, size);
    thread->stackStart = stack;
    thread->stackSize = totalStackSize;
    thread->name = name;
    thread->priority = prio;
    thread->status = MICROBYTE_THREAD_STATUS_STOPPED;

    scheduler.addNumOfThreads();

    if (flags & MICROBYTE_THREAD_FLAGS_SLEEP)
    {
        scheduler.setThreadStatus(thread, MICROBYTE_THREAD_STATUS_SLEEPING);
    }
    else
    {
        scheduler.setThreadStatus(thread, MICROBYTE_THREAD_STATUS_PENDING);

        if (!(flags & MICROBYTE_THREAD_FLAGS_WOUT_YIELD))
        {
            microbyte_restore_irq(state);
            scheduler.contextSwitch(prio);
            return thread;
        }
    }

    microbyte_restore_irq(state);

    return thread;
}

void MicroByteScheduler::setThreadStatus(MicroByteThread *thread, MicroByteThreadStatus newStatus)
{
    uint8_t priority = thread->priority;

    if (newStatus >= MICROBYTE_THREAD_STATUS_RUNNING)
    {
        if (thread->status < MICROBYTE_THREAD_STATUS_RUNNING)
        {
            runQueue[priority].rightPush(&thread->runQueueEntry);
            runQueueBitCache |= 1 << priority;
        }
    }
    else
    {
        if (thread->status >= MICROBYTE_THREAD_STATUS_RUNNING)
        {
            runQueue[priority].leftPop();

            if (runQueue[priority].next == NULL)
                runQueueBitCache &= ~(1 << priority);
        }
    }

    thread->status = newStatus;
}

void MicroByteScheduler::contextSwitch(uint8_t priority)
{
    MicroByteThread *curThread = currentActiveThread;
    uint8_t curPriority = curThread->getPriority();
    int isInRunQueue = (curThread->getStatus() >= MICROBYTE_THREAD_STATUS_RUNNING);
    // The lowest priority number is the highest priority thread
    if (!isInRunQueue || (curPriority > priority))
    {
        if (microbyte_in_isr())
        {
            contextSwitchRequest = 1;
        }
        else
        {
            microbyte_trigger_context_switch();
        }
    }
}

/* Source: http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup */
static const uint8_t MultiplyDeBruijnBitPosition[32] =
{
    0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
    31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

unsigned MicroByteScheduler::bitArithmLsb(unsigned v)
{
    return MultiplyDeBruijnBitPosition[((uint32_t)((v & -v) * 0x077CB531U)) >> 27];
}

template<class P, class M>
size_t offsetOf(const M P::*member)
{
    return (size_t) &( reinterpret_cast<P*>(0)->*member);
}

template<class P, class M>
P* containerOf(M* ptr, const M P::*member)
{
    return (P*)( (char*)ptr - offsetOf(member));
}

MicroByteThread *MicroByteScheduler::nextThreadFromRunQueue()
{
    uint8_t nextPrio = bitArithmLsb(runQueueBitCache);
    CircList *nextThreadEntry = runQueue[nextPrio].next->next;
    return containerOf(nextThreadEntry, &MicroByteThread::runQueueEntry);
}

uint16_t MicroByteScheduler::clearThreadFlagsAtomic(MicroByteThread *thread, uint16_t mask)
{
    unsigned state = microbyte_disable_irq();
    mask &= thread->flags;
    thread->flags &= ~mask;
    microbyte_restore_irq(state);
    return mask;
}

void MicroByteScheduler::waitThreadFlags(uint16_t mask, MicroByteThread *thread, MicroByteThreadStatus newStatus, unsigned state)
{
    thread->waitFlags = mask;
    setThreadStatus(thread, newStatus);
    microbyte_restore_irq(state);
    microbyte_trigger_context_switch();
}

void MicroByteScheduler::waitAnyThreadFlagsBlocked(uint16_t mask)
{
    unsigned state = microbyte_disable_irq();
    if (!(currentActiveThread->flags & mask))
    {
        waitThreadFlags(mask, currentActiveThread, MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ANY, state);
    }
    else
    {
        microbyte_restore_irq(state);
    }
}

void MicroByteScheduler::sleep()
{
    if (microbyte_in_isr())
        return;

    unsigned state = microbyte_disable_irq();
    setThreadStatus(currentActiveThread, MICROBYTE_THREAD_STATUS_SLEEPING);
    microbyte_restore_irq(state);
    microbyte_trigger_context_switch();
}

void MicroByteScheduler::yield()
{
    unsigned state = microbyte_disable_irq();

    if (currentActiveThread->status >= MICROBYTE_THREAD_STATUS_RUNNING)
        runQueue[currentActiveThread->priority].leftPopRightPush();

    microbyte_restore_irq(state);
    microbyte_trigger_context_switch();
}

void MicroByteScheduler::exit()
{
    (void)microbyte_disable_irq();
    threadsContainer[currentActivePid] = NULL;
    numOfThreadsInContainer -= 1;
    setThreadStatus(currentActiveThread, MICROBYTE_THREAD_STATUS_STOPPED);
    currentActiveThread = NULL;
    microbyte_trigger_context_switch();
}

int MicroByteScheduler::wakeUpThread(MicroBytePid pid)
{
    unsigned state = microbyte_disable_irq();
    MicroByteThread *threadToWake = threadFromContainer(pid);
    if (!threadToWake)
    {
        microbyte_restore_irq(state);
        return -1; // MicroByteThread wasn't in container
    }
    else if (threadToWake->status == MICROBYTE_THREAD_STATUS_SLEEPING)
    {
        setThreadStatus(threadToWake, MICROBYTE_THREAD_STATUS_PENDING);
        microbyte_restore_irq(state);
        contextSwitch(threadToWake->priority);
        return 1;
    }
    else
    {
        microbyte_restore_irq(state);
        return 0; // MicroByteThread wasn't sleep
    }
}

void MicroByteScheduler::run()
{
    contextSwitchRequest = 0;
    MicroByteThread *curThread = currentActiveThread;
    MicroByteThread *nextThread = nextThreadFromRunQueue();

    if (curThread == nextThread)
        return;

    if (curThread != NULL)
    {
        if (curThread->status == MICROBYTE_THREAD_STATUS_RUNNING)
            curThread->setStatus(MICROBYTE_THREAD_STATUS_PENDING);
    }

    nextThread->setStatus(MICROBYTE_THREAD_STATUS_RUNNING);
    currentActiveThread = nextThread;
    currentActivePid = nextThread->pid;
}

void MicroByteScheduler::setThreadFlags(MicroByteThread *thread, uint16_t mask)
{
    unsigned state = microbyte_disable_irq();
    thread->flags |= mask;

    if (wakeThreadFlags(thread))
    {
        microbyte_restore_irq(state);

        if (!microbyte_in_isr())
            microbyte_trigger_context_switch();
    }
    else
    {
        microbyte_restore_irq(state);
    }
}

uint16_t MicroByteScheduler::clearThreadFlags(uint16_t mask)
{
    return clearThreadFlagsAtomic(currentActiveThread, mask);
}

uint16_t MicroByteScheduler::waitAnyThreadFlags(uint16_t mask)
{
    waitAnyThreadFlagsBlocked(mask);
    return clearThreadFlagsAtomic(currentActiveThread, mask);
}

uint16_t MicroByteScheduler::waitAllThreadFlags(uint16_t mask)
{
    unsigned state = microbyte_disable_irq();
    if (!((currentActiveThread->flags & mask) == mask))
    {
        waitThreadFlags(mask, currentActiveThread, MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ALL, state);
    }
    else
    {
        microbyte_restore_irq(state);
    }
    return clearThreadFlagsAtomic(currentActiveThread, mask);
}

uint16_t MicroByteScheduler::waitOneThreadFlags(uint16_t mask)
{
    waitAnyThreadFlagsBlocked(mask);
    uint16_t tmp = currentActiveThread->flags & mask;
    tmp &= (~tmp + 1);
    return clearThreadFlagsAtomic(currentActiveThread, tmp);
}

int MicroByteScheduler::wakeThreadFlags(MicroByteThread *thread)
{
    unsigned wakeup;
    uint16_t mask = thread->waitFlags;
    switch (thread->status)
    {
    case MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ANY:
        wakeup = (thread->flags & mask);
        break;
    case MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ALL:
        wakeup = ((thread->flags & mask) == mask);
        break;
    default:
        wakeup = 0;
        break;
    }
    if (wakeup)
    {
        setThreadStatus(thread, MICROBYTE_THREAD_STATUS_PENDING);
        requestContextSwitch();
    }
    return wakeup;
}

void MicroByteThread::addTo(CircList *queue)
{
    while (queue->next)
    {
        MicroByteThread *queuedThread = containerOf(queue->next, &MicroByteThread::runQueueEntry);
        if (queuedThread->priority > this->priority)
        {
            break;
        }
        queue = queue->next;
    }
    this->runQueueEntry.next = queue->next;
    queue->next = &this->runQueueEntry;
}

MicroByteThread *MicroByteThread::get(CircList *entry)
{
    return containerOf(entry, &MicroByteThread::runQueueEntry);
}

void MicroByteThread::setMsgQueue(MicroByteMsg *msg, unsigned int size)
{
    msgArray = msg;
    msgQueue.reset(size);
}

int MicroByteThread::queuedMsg(MicroByteMsg *msg)
{
    int index = msgQueue.put();

    if (index < 0)
        return 0;

    MicroByteMsg *dest = &msgArray[index];
    *dest = *msg;
    return 1;
}

int MicroByteThread::numOfMsgInQueue()
{
    int queuedMsgs = -1;

    if (hasMsgQueue())
        queuedMsgs = msgQueue.avail();

    return queuedMsgs;
}

int MicroByteThread::hasMsgQueue()
{
    return msgArray != NULL;
}
