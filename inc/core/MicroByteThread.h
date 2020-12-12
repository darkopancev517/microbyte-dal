#ifndef MICROBYTE_THREAD_H
#define MICROBYTE_THREAD_H

#include <stdint.h>

#include "MicroByteConfig.h"
#include "CircList.h"
#include "List.h"
#include "Cib.h"

#define MICROBYTE_THREAD_STATUS_NOT_FOUND ((MicroByteThreadStatus)-1)

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

typedef int16_t MicroBytePid;

typedef void *(*MicroByteThreadHandler)(void *arg);

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
} MicroByteThreadStatus;

class MicroByteCpu;
class MicroByteScheduler;
class MicroByteMsg;

class MicroByteThread
{
    friend class MicroByteScheduler;
    friend class MicroByteMutex;
    friend class MicroByteMsg;

    protected:

    char *stackPointer;
    MicroByteThreadStatus status;
    uint8_t priority;
    MicroBytePid pid;
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
    MicroByteMsg *msgArray;

    public:

    MicroByteThread();

    static MicroByteThread *init(char *stack, int size, uint8_t prio, int flags,
                                 MicroByteThreadHandler func, void *arg, const char *name);

    uint8_t getPriority() { return priority; }

    MicroByteThreadStatus getStatus() { return status; }

    void setStatus(MicroByteThreadStatus newStatus) { status = newStatus; }

    MicroBytePid getPid() { return pid; }

    const char *getName() { return name; }

    void addTo(CircList *queue);

    static MicroByteThread *get(CircList *entry);

    void setMsgQueue(MicroByteMsg *msg, unsigned int size);

    int queuedMsg(MicroByteMsg *msg);

    int numOfMsgInQueue();

    int hasMsgQueue();
};

class MicroByteScheduler
{
    int numOfThreadsInContainer;
    unsigned int contextSwitchRequest;
    MicroByteThread *currentActiveThread;
    MicroBytePid currentActivePid;
    uint32_t runQueueBitCache;
    MicroByteThread *threadsContainer[MICROBYTE_THREAD_PID_LAST + 1];
    CircList runQueue[MICROBYTE_CONFIG_THREAD_PRIO_LEVELS];
    MicroByteCpu *cpu;

    static unsigned bitArithmLsb(unsigned v);
    MicroByteThread *nextThreadFromRunQueue();

    uint16_t clearThreadFlagsAtomic(MicroByteThread *thread, uint16_t mask);
    void waitThreadFlags(uint16_t mask, MicroByteThread *thread, MicroByteThreadStatus newStatus, unsigned state);
    void waitAnyThreadFlagsBlocked(uint16_t mask);

    public:

    static MicroByteScheduler &init();
    static MicroByteScheduler &get();

    MicroByteScheduler();

    void setThreadStatus(MicroByteThread *thread, MicroByteThreadStatus status);

    void contextSwitch(uint8_t priority);

    MicroByteThread *threadFromContainer(MicroBytePid pid) { return threadsContainer[pid]; }

    void addThread(MicroByteThread *thread, MicroBytePid pid) { threadsContainer[pid] = thread; }

    void addNumOfThreads() { numOfThreadsInContainer += 1; }

    int numOfThreads() { return numOfThreadsInContainer; }

    MicroByteThread *activeThread() { return currentActiveThread; }

    MicroBytePid activePid() { return currentActivePid; }

    int requestedContextSwitch() { return contextSwitchRequest; }

    void requestContextSwitch() { contextSwitchRequest = 1; }

    void sleep();

    void yield();

    void exit();

    int wakeUpThread(MicroBytePid pid);

    void run();

    void setThreadFlags(MicroByteThread *thread, uint16_t mask);

    uint16_t clearThreadFlags(uint16_t mask);

    uint16_t waitAnyThreadFlags(uint16_t mask);

    uint16_t waitAllThreadFlags(uint16_t mask);

    uint16_t waitOneThreadFlags(uint16_t mask);

    int wakeThreadFlags(MicroByteThread *thread);
};

#endif
