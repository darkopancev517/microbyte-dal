#ifndef MICROBYTE_THREAD_H
#define MICROBYTE_THREAD_H

#include <stdint.h>

#include "MicroByteConfig.h"
#include "CircList.h"
#include "List.h"
#include "Cib.h"

#define MICROBYTE_THREAD_STATUS_NOT_FOUND ((ThreadStatus)-1)

#define MICROBYTE_THREAD_FLAGS_SLEEP (0x1)
#define MICROBYTE_THREAD_FLAGS_WOUT_YIELD (0x2)
#define MICROBYTE_THREAD_FLAGS_STACKMARKER (0x4)

#define MICROBYTE_THREAD_PRIORITY_LEVELS (16)
#define MICROBYTE_THREAD_PRIORITY_MIN (MICROBYTE_THREAD_PRIORITY_LEVELS - 1)
#define MICROBYTE_THREAD_PRIORITY_IDLE MICROBYTE_THREAD_PRIORITY_MIN
#define MICROBYTE_THREAD_PRIORITY_MAIN (MICROBYTE_THREAD_PRIORITY_MIN - (MICROBYTE_THREAD_PRIORITY_LEVELS / 2))

#define MICROBYTE_THREAD_PID_UNDEF (0)
#define MICROBYTE_THREAD_PID_FIRST (MICROBYTE_THREAD_PID_UNDEF + 1)
#define MICROBYTE_THREAD_PID_LAST (MICROBYTE_THREAD_PID_FIRST + MICROBYTE_CONFIG_THREAD_MAX - 1)
#define MICROBYTE_THREAD_PID_ISR (MICROBYTE_THREAD_PID_LAST - 1)

typedef int16_t ThreadPid;

typedef void *(*ThreadFunc)(void *arg);

typedef enum
{
    MICROBYTE_THREAD_STATUS_STOPPED,
    MICROBYTE_THREAD_STATUS_SLEEPING,
    MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED,
    MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED,
    MICROBYTE_THREAD_STATUS_SEND_BLOCKED,
    MICROBYTE_THREAD_STATUS_REPLY_BLOCKED,
    MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ANY,
    MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ALL,
    MICROBYTE_THREAD_STATUS_MBOX_BLOCKED,
    MICROBYTE_THREAD_STATUS_COND_BLOCKED,
    MICROBYTE_THREAD_STATUS_RUNNING,
    MICROBYTE_THREAD_STATUS_PENDING,
    MICROBYTE_THREAD_STATUS_NUMOF,
} ThreadStatus;

class MicroByteCpu;
class ThreadScheduler;
class Msg;

class Thread
{
    friend class ThreadScheduler;
    friend class Mutex;
    friend class Msg;

    protected:

    char *stackPointer;
    ThreadStatus status;
    uint8_t priority;
    ThreadPid pid;
    CircList runQueueEntry;
    char *stackStart;
    const char *name;
    int stackSize;
    MicroByteCpu *cpu;

    uint16_t flags;
    uint16_t waitFlags;

    void *waitData;
    List msgWaiters;
    Cib msgQueue;
    Msg *msgArray;

    public:

    Thread();

    static Thread *init(char *stack, int size, uint8_t prio, int flags, ThreadFunc func, void *arg, const char *name);

    uint8_t getPriority() { return priority; }

    ThreadStatus getStatus() { return status; }

    void setStatus(ThreadStatus newStatus) { status = newStatus; }

    ThreadPid getPid() { return pid; }

    const char *getName() { return name; }

    void addTo(CircList *queue);

    static Thread *get(CircList *entry);

    void setMsgQueue(Msg *msg, unsigned int size);

    int queuedMsg(Msg *msg);

    int numOfMsgInQueue();

    int hasMsgQueue();
};

class ThreadScheduler
{
    int numOfThreadsInContainer;
    unsigned int contextSwitchRequest;
    Thread *currentActiveThread;
    ThreadPid currentActivePid;
    uint32_t runQueueBitCache;
    Thread *threadsContainer[MICROBYTE_THREAD_PID_LAST + 1];
    CircList runQueue[MICROBYTE_CONFIG_THREAD_PRIO_LEVELS];
    MicroByteCpu *cpu;

    static unsigned bitArithmLsb(unsigned v);
    Thread *nextThreadFromRunQueue();

    uint16_t clearThreadFlagsAtomic(Thread *thread, uint16_t mask);
    void waitThreadFlags(uint16_t mask, Thread *thread, ThreadStatus newStatus, unsigned state);
    void waitAnyThreadFlagsBlocked(uint16_t mask);

    public:

    static ThreadScheduler &init();
    static ThreadScheduler &get();

    ThreadScheduler();

    void setThreadStatus(Thread *thread, ThreadStatus status);

    void contextSwitch(uint8_t priority);

    Thread *threadFromContainer(ThreadPid pid) { return threadsContainer[pid]; }

    void addThread(Thread *thread, ThreadPid pid) { threadsContainer[pid] = thread; }

    void addNumOfThreads() { numOfThreadsInContainer += 1; }

    int numOfThreads() { return numOfThreadsInContainer; }

    Thread *activeThread() { return currentActiveThread; }

    ThreadPid activePid() { return currentActivePid; }

    int requestedContextSwitch() { return contextSwitchRequest; }

    void requestContextSwitch() { contextSwitchRequest = 1; }

    void sleep();

    void yield();

    void exit();

    int wakeUpThread(ThreadPid pid);

    void run();

    void setThreadFlags(Thread *thread, uint16_t mask);

    uint16_t clearThreadFlags(uint16_t mask);

    uint16_t waitAnyThreadFlags(uint16_t mask);

    uint16_t waitAllThreadFlags(uint16_t mask);

    uint16_t waitOneThreadFlags(uint16_t mask);

    int wakeThreadFlags(Thread *thread);
};

#endif
