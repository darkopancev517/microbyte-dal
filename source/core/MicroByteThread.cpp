#include "MicroByteCpu.h"
#include "MicroByteThread.h"
#include "MicroByteMsg.h"
#include "New.h"
#include "Utils.h"

DEFINE_ALIGNED_VAR(threadSchedulerRaw, sizeof(ThreadScheduler), uint64_t);

ThreadScheduler &ThreadScheduler::init()
{
    ThreadScheduler *scheduler = &get();
    scheduler = new (&threadSchedulerRaw) ThreadScheduler();
    return *scheduler;
}

ThreadScheduler &ThreadScheduler::get()
{
    void *scheduler = &threadSchedulerRaw;
    return *static_cast<ThreadScheduler *>(scheduler);
}

ThreadScheduler::ThreadScheduler()
    : numOfThreadsInContainer(0)
    , contextSwitchRequest(0)
    , currentActiveThread(NULL)
    , currentActivePid(MICROBYTE_THREAD_PID_UNDEF)
    , runQueueBitCache(0)
{
    for (ThreadPid i = MICROBYTE_THREAD_PID_FIRST; i <= MICROBYTE_THREAD_PID_LAST; ++i)
    {
        this->threadsContainer[i] = NULL;
    }
    for (uint8_t prio = 0; prio < MICROBYTE_CONFIG_THREAD_PRIO_LEVELS; prio++)
    {
        this->runQueue[prio].next = NULL;
    }
    this->cpu = uByteCpu;
}

Thread::Thread()
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
    this->cpu = uByteCpu;
}

Thread *Thread::init(char *stack, int size, uint8_t prio, int flags,
                     ThreadFunc func, void *arg, const char *name)
{
    if (prio >= MICROBYTE_CONFIG_THREAD_PRIO_LEVELS)
    {
        return NULL;
    }

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
    size -= sizeof(Thread);

    // Round down the stacksize to multiple of Thread aligments (usually 16/32 bit)
    size -= size % 8;

    if (size < 0)
    {
        // Stack size too small
        return NULL;
    }

    Thread *thread = new (stack + size) Thread();

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

    unsigned state = thread->cpu->disableIrq();

    ThreadPid pid = MICROBYTE_THREAD_PID_UNDEF;

    ThreadScheduler &scheduler = ThreadScheduler::get();

    for (ThreadPid i = MICROBYTE_THREAD_PID_FIRST; i <= MICROBYTE_THREAD_PID_LAST; ++i)
    {
        if (scheduler.threadFromContainer(i) == NULL)
        {
            pid = i;
            break;
        }
    }

    if (pid == MICROBYTE_THREAD_PID_UNDEF)
    {
        thread->cpu->restoreIrq(state);
        return NULL;
    }

    scheduler.addThread(thread, pid);

    thread->pid = pid;
    thread->stackPointer = thread->cpu->stackInit(func, arg, stack, size);
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
            thread->cpu->restoreIrq(state);
            scheduler.contextSwitch(prio);
            return thread;
        }
    }

    thread->cpu->restoreIrq(state);

    return thread;
}

void ThreadScheduler::setThreadStatus(Thread *thread, ThreadStatus newStatus)
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
            {
                runQueueBitCache &= ~(1 << priority);
            }
        }
    }

    thread->status = newStatus;
}

void ThreadScheduler::contextSwitch(uint8_t priority)
{
    Thread *curThread = currentActiveThread;
    uint8_t curPriority = curThread->getPriority();
    int isInRunQueue = (curThread->getStatus() >= MICROBYTE_THREAD_STATUS_RUNNING);
    // The lowest priority number is the highest priority thread
    if (!isInRunQueue || (curPriority > priority))
    {
        if (cpu->inIsr())
        {
            contextSwitchRequest = 1;
        }
        else
        {
            cpu->triggerContextSwitch();
        }
    }
}

/* Source: http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup */
static const uint8_t MultiplyDeBruijnBitPosition[32] =
{
    0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
    31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

unsigned ThreadScheduler::bitArithmLsb(unsigned v)
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

Thread *ThreadScheduler::nextThreadFromRunQueue()
{
    uint8_t nextPrio = bitArithmLsb(runQueueBitCache);
    CircList *nextThreadEntry = runQueue[nextPrio].next->next;
    return containerOf(nextThreadEntry, &Thread::runQueueEntry);
}

uint16_t ThreadScheduler::clearThreadFlagsAtomic(Thread *thread, uint16_t mask)
{
    unsigned state = cpu->disableIrq();
    mask &= thread->flags;
    thread->flags &= ~mask;
    cpu->restoreIrq(state);
    return mask;
}

void ThreadScheduler::waitThreadFlags(uint16_t mask, Thread *thread, ThreadStatus newStatus, unsigned state)
{
    thread->waitFlags = mask;
    setThreadStatus(thread, newStatus);
    cpu->restoreIrq(state);
    cpu->triggerContextSwitch();
}

void ThreadScheduler::waitAnyThreadFlagsBlocked(uint16_t mask)
{
    unsigned state = cpu->disableIrq();
    if (!(currentActiveThread->flags & mask))
    {
        waitThreadFlags(mask, currentActiveThread, MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ANY, state);
    }
    else
    {
        cpu->restoreIrq(state);
    }
}

void ThreadScheduler::sleep()
{
    if (cpu->inIsr())
    {
        return;
    }
    unsigned state = cpu->disableIrq();
    setThreadStatus(currentActiveThread, MICROBYTE_THREAD_STATUS_SLEEPING);
    cpu->restoreIrq(state);
    cpu->triggerContextSwitch();
}

void ThreadScheduler::yield()
{
    unsigned state = cpu->disableIrq();
    if (currentActiveThread->status >= MICROBYTE_THREAD_STATUS_RUNNING)
    {
        runQueue[currentActiveThread->priority].leftPopRightPush();
    }
    cpu->restoreIrq(state);
    cpu->triggerContextSwitch();
}

void ThreadScheduler::exit()
{
    (void)cpu->disableIrq();
    threadsContainer[currentActivePid] = NULL;
    numOfThreadsInContainer -= 1;
    setThreadStatus(currentActiveThread, MICROBYTE_THREAD_STATUS_STOPPED);
    currentActiveThread = NULL;
    cpu->triggerContextSwitch();
}

int ThreadScheduler::wakeUpThread(ThreadPid pid)
{
    unsigned state = cpu->disableIrq();
    Thread *threadToWake = threadFromContainer(pid);
    if (!threadToWake)
    {
        cpu->restoreIrq(state);
        return -1; // Thread wasn't in container
    }
    else if (threadToWake->status == MICROBYTE_THREAD_STATUS_SLEEPING)
    {
        setThreadStatus(threadToWake, MICROBYTE_THREAD_STATUS_PENDING);
        cpu->restoreIrq(state);
        contextSwitch(threadToWake->priority);
        return 1;
    }
    else
    {
        cpu->restoreIrq(state);
        return 0; // Thread wasn't sleep
    }
}

void ThreadScheduler::run()
{
    contextSwitchRequest = 0;
    Thread *curThread = currentActiveThread;
    Thread *nextThread = nextThreadFromRunQueue();
    if (curThread == nextThread)
    {
        return;
    }
    if (curThread != NULL)
    {
        if (curThread->status == MICROBYTE_THREAD_STATUS_RUNNING)
        {
            curThread->setStatus(MICROBYTE_THREAD_STATUS_PENDING);
        }
    }
    nextThread->setStatus(MICROBYTE_THREAD_STATUS_RUNNING);
    currentActiveThread = nextThread;
    currentActivePid = nextThread->pid;
}

void ThreadScheduler::setThreadFlags(Thread *thread, uint16_t mask)
{
    unsigned state = cpu->disableIrq();
    thread->flags |= mask;
    if (wakeThreadFlags(thread))
    {
        cpu->restoreIrq(state);
        if (!cpu->inIsr())
        {
            cpu->triggerContextSwitch();
        }
    }
    else
    {
        cpu->restoreIrq(state);
    }
}

uint16_t ThreadScheduler::clearThreadFlags(uint16_t mask)
{
    return clearThreadFlagsAtomic(currentActiveThread, mask);
}

uint16_t ThreadScheduler::waitAnyThreadFlags(uint16_t mask)
{
    waitAnyThreadFlagsBlocked(mask);
    return clearThreadFlagsAtomic(currentActiveThread, mask);
}

uint16_t ThreadScheduler::waitAllThreadFlags(uint16_t mask)
{
    unsigned state = cpu->disableIrq();
    if (!((currentActiveThread->flags & mask) == mask))
    {
        waitThreadFlags(mask, currentActiveThread, MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ALL, state);
    }
    else
    {
        cpu->restoreIrq(state);
    }
    return clearThreadFlagsAtomic(currentActiveThread, mask);
}

uint16_t ThreadScheduler::waitOneThreadFlags(uint16_t mask)
{
    waitAnyThreadFlagsBlocked(mask);
    uint16_t tmp = currentActiveThread->flags & mask;
    tmp &= (~tmp + 1);
    return clearThreadFlagsAtomic(currentActiveThread, tmp);
}

int ThreadScheduler::wakeThreadFlags(Thread *thread)
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

void Thread::addTo(CircList *queue)
{
    while (queue->next)
    {
        Thread *queuedThread = containerOf(queue->next, &Thread::runQueueEntry);
        if (queuedThread->priority > this->priority)
        {
            break;
        }
        queue = queue->next;
    }
    this->runQueueEntry.next = queue->next;
    queue->next = &this->runQueueEntry;
}

Thread *Thread::get(CircList *entry)
{
    return containerOf(entry, &Thread::runQueueEntry);
}

void Thread::setMsgQueue(Msg *msg, unsigned int size)
{
    msgArray = msg;
    msgQueue.reset(size);
}

int Thread::queuedMsg(Msg *msg)
{
    int index = msgQueue.put();
    if (index < 0)
    {
        return 0;
    }
    Msg *dest = &msgArray[index];
    *dest = *msg;
    return 1;
}

int Thread::numOfMsgInQueue()
{
    int queuedMsgs = -1;
    if (hasMsgQueue())
    {
        queuedMsgs = msgQueue.avail();
    }
    return queuedMsgs;
}

int Thread::hasMsgQueue()
{
    return msgArray != NULL;
}
