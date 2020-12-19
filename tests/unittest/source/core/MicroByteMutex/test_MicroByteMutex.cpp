#include "gtest/gtest.h"

#include "MicroByteUnitTest.h"

#include "MicroByteMutex.h"
#include "MicroByteThread.h"

class TestMicroByteMutex : public testing::Test
{
    protected:

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(TestMicroByteMutex, mutexFunctionsTest)
{
    MicroByteScheduler *scheduler = &MicroByteScheduler::init();

    EXPECT_NE(scheduler, nullptr);
    EXPECT_EQ(scheduler->numOfThreads(), 0);
    EXPECT_EQ(sched_active_thread, nullptr);
    EXPECT_EQ(sched_active_pid, MICROBYTE_THREAD_PID_UNDEF);

    char idleStack[128];

    MicroByteThread *idleThread = MicroByteThread::init(idleStack, sizeof(idleStack), nullptr, "idle", MICROBYTE_THREAD_PRIORITY_IDLE);

    EXPECT_NE(idleThread, nullptr);
    EXPECT_EQ(idleThread->getPid(), 1);
    EXPECT_EQ(idleThread->getPriority(), MICROBYTE_THREAD_PRIORITY_IDLE);
    EXPECT_EQ(idleThread->getName(), "idle");
    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    char mainStack[128];

    MicroByteThread *mainThread = MicroByteThread::init(mainStack, sizeof(mainStack), nullptr, "main");

    EXPECT_NE(mainThread, nullptr);
    EXPECT_EQ(mainThread->getPid(), 2);
    EXPECT_EQ(mainThread->getPriority(), MICROBYTE_THREAD_PRIORITY_MAIN);
    EXPECT_EQ(mainThread->getName(), "main");
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    char stack1[128];

    MicroByteThread *thread1 = MicroByteThread::init(stack1, sizeof(stack1), nullptr, "thread1", MICROBYTE_THREAD_PRIORITY_MAIN - 1);

    EXPECT_NE(thread1, nullptr);
    EXPECT_EQ(thread1->getPid(), 3);
    EXPECT_EQ(thread1->getPriority(), MICROBYTE_THREAD_PRIORITY_MAIN - 1);
    EXPECT_EQ(thread1->getName(), "thread1");
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    char stack2[128];

    MicroByteThread *thread2 = MicroByteThread::init(stack2, sizeof(stack2), nullptr, "thread2", MICROBYTE_THREAD_PRIORITY_MAIN - 1);

    EXPECT_NE(thread2, nullptr);
    EXPECT_EQ(thread2->getPid(), 4);
    EXPECT_EQ(thread2->getPriority(), MICROBYTE_THREAD_PRIORITY_MAIN - 1);
    EXPECT_EQ(thread2->getName(), "thread2");
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    EXPECT_EQ(scheduler->numOfThreads(), 4);
    EXPECT_EQ(scheduler->threadFromContainer(idleThread->getPid()), idleThread);
    EXPECT_EQ(scheduler->threadFromContainer(mainThread->getPid()), mainThread);
    EXPECT_EQ(scheduler->threadFromContainer(thread1->getPid()), thread1);
    EXPECT_EQ(scheduler->threadFromContainer(thread2->getPid()), thread2);
    EXPECT_EQ(scheduler->requestedContextSwitch(), 0);
    EXPECT_EQ(sched_active_thread, nullptr);
    EXPECT_EQ(sched_active_pid, MICROBYTE_THREAD_PID_UNDEF);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();
    scheduler->run();
    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] single mutex
     * -------------------------------------------------------------------------
     **/


    MicroByteMutex mutex = MicroByteMutex(MICROBYTE_MUTEX_INIT_UNLOCKED);

    mutex.lock();

    // MicroByteMutex was unlocked when set lock for the first time, therefore current
    // thread (thread1) expected still running state

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    mutex.tryLock();
    mutex.tryLock();
    mutex.tryLock();
    mutex.tryLock();
    mutex.tryLock();
    mutex.tryLock();

    // MicroByteMutex was set to lock, nothing will happen when calling tryLock()

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    mutex.lock(); // this will blocked thread1

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    mutex.lock(); // this will blocked thread2

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    mutex.lock(); // this will blocked mainThread

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    EXPECT_EQ(mutex.peek(), thread1->getPid()); // thread1 is in the HEAD of mutex list

    mutex.unlock(); // this expected to unlock thread1

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    mutex.unlock(); // this will unlock thread2
    mutex.unlock(); // this will unlock mainThread

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    EXPECT_EQ(mutex.peek(), MICROBYTE_THREAD_PID_UNDEF);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] multiple mutexes with LOCKED initial value
     * -------------------------------------------------------------------------
     **/

    MicroByteMutex mutex1 = MicroByteMutex();
    MicroByteMutex mutex2 = MicroByteMutex();
    MicroByteMutex mutex3 = MicroByteMutex();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    mutex1.lock(); // mutex1 locked thread1

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    mutex2.lock();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    mutex3.lock();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    mutex2.unlock(); // this will unlocked thread2

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    mutex3.unlock();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    mutex1.unlock();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] unlock mutex in ISR
     * -------------------------------------------------------------------------
     **/

    mutex3.lock();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    mutex3.unlock();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    mutex1.lock(); // this will lock thread1

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    mutex2.lock(); // this will lock thread2

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    mutex3.lock(); // this will lock mainThread

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    microbyte_set_in_isr(1);

    EXPECT_EQ(microbyte_in_isr(), 1);

    mutex3.unlock();

    microbyte_set_in_isr(0);

    EXPECT_EQ(microbyte_in_isr(), 0);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);

    mutex1.unlock();
    mutex2.unlock();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    mutex1.lock();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_MUTEX_BLOCKED);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    mutex1.unlockAndSleep();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    mutex1.unlock();
    mutex1.unlock();
    mutex1.unlock();
    mutex1.unlock();
    mutex1.unlock();
    mutex1.unlock();
    mutex1.unlock();
    mutex1.unlock();

    // mutex1 wasn't locked so nothing happen

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread2->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);
}
