#include "gtest/gtest.h"

#include "MicroByteUnitTest.h"

#include "MicroByteMutex.h"
#include "MicroByteThread.h"

class TestMicroByteThread : public testing::Test
{
    protected:

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(TestMicroByteThread, basicThreadTest)
{
    MicroByteScheduler *scheduler = &MicroByteScheduler::init();

    EXPECT_NE(scheduler, nullptr);
    EXPECT_EQ(scheduler->numOfThreads(), 0);
    EXPECT_EQ(scheduler->activeThread(), nullptr);
    EXPECT_EQ(scheduler->activePid(), MICROBYTE_THREAD_PID_UNDEF);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] create single thread and run the thread scheduler, that
     * thread is expected to be in running state and become current active
     * thread
     * -------------------------------------------------------------------------
     **/

    char stack1[128];

    MicroByteThread *thread1 = MicroByteThread::init(stack1, sizeof(stack1),
                                   MICROBYTE_THREAD_PRIORITY_MAIN,
                                   MICROBYTE_THREAD_FLAGS_WOUT_YIELD |
                                   MICROBYTE_THREAD_FLAGS_STACKMARKER,
                                   NULL, NULL, "thread1");

    EXPECT_NE(thread1, nullptr);

    EXPECT_EQ(thread1->getPid(), 1);
    EXPECT_EQ(thread1->getPriority(), MICROBYTE_THREAD_PRIORITY_MAIN);
    EXPECT_EQ(thread1->getName(), "thread1");
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    EXPECT_EQ(scheduler->numOfThreads(), 1);
    EXPECT_EQ(scheduler->threadFromContainer(thread1->getPid()), thread1);
    EXPECT_EQ(scheduler->requestedContextSwitch(), 0);
    EXPECT_EQ(scheduler->activeThread(), nullptr);
    EXPECT_EQ(scheduler->activePid(), MICROBYTE_THREAD_PID_UNDEF);

    scheduler->run();

    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    EXPECT_EQ(scheduler->activeThread(), thread1);
    EXPECT_EQ(scheduler->activePid(), thread1->getPid());
    EXPECT_EQ(scheduler->numOfThreads(), 1);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] create new thread and exit current active thread
     * -------------------------------------------------------------------------
     **/

    char stack2[128];

    MicroByteThread *thread2 = MicroByteThread::init(stack2, sizeof(stack2),
                                   MICROBYTE_THREAD_PRIORITY_MAIN - 1,
                                   MICROBYTE_THREAD_FLAGS_WOUT_YIELD |
                                   MICROBYTE_THREAD_FLAGS_STACKMARKER,
                                   NULL, NULL, "thread2");

    EXPECT_NE(thread2, nullptr);

    EXPECT_EQ(thread2->getPid(), 2);
    EXPECT_EQ(thread2->getPriority(), MICROBYTE_THREAD_PRIORITY_MAIN - 1);
    EXPECT_EQ(thread2->getName(), "thread2");
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    EXPECT_EQ(scheduler->activeThread(), thread2);
    EXPECT_EQ(scheduler->activePid(), thread2->getPid());
    EXPECT_EQ(scheduler->numOfThreads(), 2);

    // Exit current active thread

    scheduler->exit();

    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_STOPPED);

    scheduler->run();

    EXPECT_EQ(scheduler->activeThread(), thread1);
    EXPECT_EQ(scheduler->activePid(), thread1->getPid());
    EXPECT_EQ(scheduler->numOfThreads(), 1);

    // Try to get removed thread from scheduler

    MicroByteThread *thread = scheduler->threadFromContainer(thread2->getPid());

    EXPECT_EQ(thread, nullptr);
}

TEST_F(TestMicroByteThread, multipleThreadTest)
{
    MicroByteScheduler *scheduler = &MicroByteScheduler::init();

    EXPECT_NE(scheduler, nullptr);
    EXPECT_EQ(scheduler->numOfThreads(), 0);
    EXPECT_EQ(scheduler->activeThread(), nullptr);
    EXPECT_EQ(scheduler->activePid(), MICROBYTE_THREAD_PID_UNDEF);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] create multiple thread ("idle" and "main" thread) and make
     * sure the thread with higher priority will be in running state and the
     * thread with lower priority ("idle" thread) is in pending state
     * -------------------------------------------------------------------------
     **/

    char idleStack[128];

    MicroByteThread *idleThread = MicroByteThread::init(idleStack, sizeof(idleStack),
                                      MICROBYTE_THREAD_PRIORITY_IDLE,
                                      MICROBYTE_THREAD_FLAGS_WOUT_YIELD |
                                      MICROBYTE_THREAD_FLAGS_STACKMARKER,
                                      NULL, NULL, "idle");

    EXPECT_NE(idleThread, nullptr);
    EXPECT_EQ(idleThread->getPid(), 1);
    EXPECT_EQ(idleThread->getPriority(), MICROBYTE_THREAD_PRIORITY_IDLE);
    EXPECT_EQ(idleThread->getName(), "idle");
    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    char mainStack[128];

    MicroByteThread *mainThread = MicroByteThread::init(mainStack, sizeof(mainStack),
                                      MICROBYTE_THREAD_PRIORITY_MAIN,
                                      MICROBYTE_THREAD_FLAGS_WOUT_YIELD |
                                      MICROBYTE_THREAD_FLAGS_STACKMARKER,
                                      NULL, NULL, "main");

    EXPECT_NE(mainThread, nullptr);
    EXPECT_EQ(mainThread->getPid(), 2);
    EXPECT_EQ(mainThread->getPriority(), MICROBYTE_THREAD_PRIORITY_MAIN);
    EXPECT_EQ(mainThread->getName(), "main");
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    EXPECT_EQ(scheduler->numOfThreads(), 2);
    EXPECT_EQ(scheduler->threadFromContainer(idleThread->getPid()), idleThread);
    EXPECT_EQ(scheduler->threadFromContainer(mainThread->getPid()), mainThread);
    EXPECT_EQ(scheduler->requestedContextSwitch(), 0);
    EXPECT_EQ(scheduler->activeThread(), nullptr);
    EXPECT_EQ(scheduler->activePid(), MICROBYTE_THREAD_PID_UNDEF);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    EXPECT_EQ(scheduler->activeThread(), mainThread);
    EXPECT_EQ(scheduler->activePid(), mainThread->getPid());
    EXPECT_EQ(scheduler->numOfThreads(), 2);

    // At this point "main" thread alread in running state as expected

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] set the higher priority thread ("main" thread) to blocked
     * state and lower priority thread ("idle" thread) should be in in running
     * state
     * -------------------------------------------------------------------------
     **/

    scheduler->setThreadStatus(mainThread, MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(scheduler->activeThread(), mainThread);

    scheduler->run();

    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(scheduler->activeThread(), idleThread);
    EXPECT_EQ(scheduler->activePid(), idleThread->getPid());
    EXPECT_EQ(scheduler->numOfThreads(), 2);

    // At this point "idle" thread in running state as expected after "main"
    // thread set to blocked state

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] create new thread with higher priority than main and idle
     * thread and yield immediately
     * -------------------------------------------------------------------------
     **/

    char stack1[128];

    MicroByteThread *thread1 = MicroByteThread::init(stack1, sizeof(stack1),
                                   MICROBYTE_THREAD_PRIORITY_MAIN - 1,
                                   MICROBYTE_THREAD_FLAGS_STACKMARKER,
                                   NULL, NULL, "thread1");

    EXPECT_NE(thread1, nullptr);
    EXPECT_EQ(thread1->getPid(), 3);
    EXPECT_EQ(thread1->getPriority(), MICROBYTE_THREAD_PRIORITY_MAIN - 1);
    EXPECT_EQ(thread1->getName(), "thread1");
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    // At this point cpu should immediately yield the "thread1" by triggering
    // PendSV interrupt and context switch from Isr is not requested */

    EXPECT_EQ(microbyte_context_switch_triggered(), 1);
    EXPECT_EQ(scheduler->requestedContextSwitch(), 0);

    scheduler->run();

    // After run the scheduler current active thread is expected to be "thread1"

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    EXPECT_EQ(scheduler->activeThread(), thread1);
    EXPECT_EQ(scheduler->activePid(), thread1->getPid());
    EXPECT_EQ(scheduler->numOfThreads(), 3);

    scheduler->yield();

    // "thread1" alread the highest priority, nothing to be change is expected

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] mutexes
     * -------------------------------------------------------------------------
     **/

    MicroByteMutex mutex = MicroByteMutex();

    mutex.lock();

    // Note: mutex was unlocked, set to locked for the first time,
    // current active thread (thread1) will not change it's status until
    // second time `mutex.lock()` is call

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    mutex.lock();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    mutex.unlock();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    scheduler->setThreadStatus(mainThread, MICROBYTE_THREAD_STATUS_PENDING);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    scheduler->run();

    mutex.lock();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    mutex.lock();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    mutex.unlock();

    // thread1 was in HEAD of the "mutex" queue, it will unlock thread1 first

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    mutex.unlock();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    EXPECT_EQ(mutex.peek(), MICROBYTE_THREAD_PID_UNDEF);

    // No thread being locked by this mutex so peek() should return PID UNDEF

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    scheduler->setThreadStatus(mainThread, MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] try to set current active thread to sleep and wakeup
     * -------------------------------------------------------------------------
     **/

    scheduler->sleep();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    // Note: at this point both mainThread and thread1 are in blocking
    // status, so the next expected thread to run is idleThread because
    // idleThread was in pending status

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    EXPECT_EQ(scheduler->activeThread(), idleThread);
    EXPECT_EQ(scheduler->activePid(), idleThread->getPid());
    EXPECT_EQ(scheduler->numOfThreads(), 3);

    EXPECT_EQ(scheduler->wakeUpThread(mainThread->getPid()), 0);

    // mainThread wasn't in sleeping state

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    // Wake up thread1 which was on sleeping status

    EXPECT_EQ(scheduler->wakeUpThread(thread1->getPid()), 1);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] try to set sleep in Isr
     * -------------------------------------------------------------------------
     **/

    microbyte_set_in_isr(1);

    scheduler->sleep();

    // We can't sleep in ISR function, nothing is expected to happen

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    microbyte_set_in_isr(0);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] try to context swithing to lower priority thread than current
     * running thread
     * -------------------------------------------------------------------------
     **/

    microbyte_reset_context_switch_state();

    EXPECT_EQ(microbyte_context_switch_triggered(), 0);

    EXPECT_EQ(idleThread->getPriority(), MICROBYTE_THREAD_PRIORITY_IDLE);
    EXPECT_EQ(mainThread->getPriority(), MICROBYTE_THREAD_PRIORITY_MAIN);
    EXPECT_EQ(thread1->getPriority(), MICROBYTE_THREAD_PRIORITY_MAIN - 1);

    scheduler->contextSwitch(mainThread->getPriority());

    // Note: because "mainThread" priority is lower than current running thread
    // and current running thread is still in running status, nothing should
    // happened

    EXPECT_EQ(microbyte_context_switch_triggered(), 0);

    EXPECT_EQ(scheduler->requestedContextSwitch(), 0);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    EXPECT_EQ(scheduler->activeThread(), thread1);
    EXPECT_EQ(scheduler->activePid(), thread1->getPid());
    EXPECT_EQ(scheduler->numOfThreads(), 3);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] request context swicth inside Isr (Interrupt Service Routine)
     * and current running thread is not in running status, it expected to see
     * context switch is requested instead of yielding immediately to the next
     * thread
     * -------------------------------------------------------------------------
     **/

    // Set "thread1" at blocked state first and is expected to be the next
    // thread to run is "idle" thread

    scheduler->setThreadStatus(thread1, MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED);

    EXPECT_EQ(scheduler->activeThread(), idleThread);
    EXPECT_EQ(scheduler->activePid(), idleThread->getPid());
    EXPECT_EQ(scheduler->numOfThreads(), 3);

    // At this point idle thread is run as expected, because other
    // higher priority threads is in blocked state

    microbyte_set_in_isr(1);

    EXPECT_EQ(microbyte_in_isr(), 1);

    scheduler->contextSwitch(thread1->getPriority());

    EXPECT_EQ(microbyte_context_switch_triggered(), 0);
    EXPECT_EQ(scheduler->requestedContextSwitch(), 1);

    // Because it is in ISR at this point context switch is requested instead
    // of immediatelly yield to "thread1"

    // In real cpu implementation, before exiting the Isr function it will
    // check this flag and trigger the PendSV interrupt if context switch is
    // requested, therefore after exiting Isr function PendSV interrupt will be
    // triggered and run thread scheduler

    // This equal to cpu->endOfIsr();

    if (scheduler->requestedContextSwitch())
        microbyte_trigger_context_switch();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED);

    microbyte_set_in_isr(0);

    EXPECT_EQ(microbyte_in_isr(), 0);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED);

    // At this point the current active thread is still "idleThread" because
    // "thread1" is still in receive blocked state even though it was try to
    // context switch to "thread1", now try set "thread1" to pending state and
    // context switch to "thread1" priority */

    scheduler->setThreadStatus(thread1, MICROBYTE_THREAD_STATUS_PENDING);

    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->contextSwitch(thread1->getPriority());

    EXPECT_EQ(microbyte_context_switch_triggered(), 1);
    EXPECT_EQ(scheduler->requestedContextSwitch(), 0);

    scheduler->run();

    microbyte_reset_context_switch_state();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    // At this point it succesfully switched to "thread1"

    EXPECT_EQ(scheduler->activeThread(), thread1);
    EXPECT_EQ(scheduler->activePid(), thread1->getPid());
    EXPECT_EQ(scheduler->numOfThreads(), 3);

    /**
     * ------------------------------------------------------------------------------
     * [TEST CASE] create a thread with highest priority but with THREAD_FLAGS
     * sleeping and not using THREAD_FLAGS create stack marker.
     * ------------------------------------------------------------------------------
     **/

    char stack2[128];

    // Intentionally create thread with misaligment stack boundary on
    // 16/32 bit boundary (&stack2[1]) will do the job

    MicroByteThread *thread2 = MicroByteThread::init(&stack2[1], sizeof(stack2) - 4,
                                   MICROBYTE_THREAD_PRIORITY_MAIN - 2,
                                   MICROBYTE_THREAD_FLAGS_SLEEP,
                                   NULL, NULL, "thread2");

    EXPECT_NE(thread2, nullptr);

    EXPECT_EQ(thread2->getPid(), 4);
    EXPECT_EQ(thread2->getPriority(), MICROBYTE_THREAD_PRIORITY_MAIN - 2);
    EXPECT_EQ(thread2->getName(), "thread2");
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    scheduler->wakeUpThread(thread2->getPid());

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
}
