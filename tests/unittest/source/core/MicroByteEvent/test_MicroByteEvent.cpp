#include "gtest/gtest.h"

#include "MicroByteUnitTest.h"

#include "MicroByteEvent.h"
#include "MicroByteThread.h"
#include "Utils.h"

class TestMicroByteEvent : public testing::Test
{
    protected:

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

class CustomEvent
{
    public:
    CustomEvent()
        : super()
        , data(0)
    {
    }
    MicroByteEvent super;
    uint32_t data;
};

TEST_F(TestMicroByteEvent, eventTest)
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

    EXPECT_EQ(scheduler->numOfThreads(), 3);
    EXPECT_EQ(scheduler->threadFromContainer(idleThread->getPid()), idleThread);
    EXPECT_EQ(scheduler->threadFromContainer(mainThread->getPid()), mainThread);
    EXPECT_EQ(scheduler->threadFromContainer(thread1->getPid()), thread1);
    EXPECT_EQ(scheduler->requestedContextSwitch(), 0);
    EXPECT_EQ(sched_active_thread, nullptr);
    EXPECT_EQ(sched_active_pid, MICROBYTE_THREAD_PID_UNDEF);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] wait event and post
     * -------------------------------------------------------------------------
     **/

    MicroByteEventQueue eventQueue = MicroByteEventQueue();

    EXPECT_EQ(eventQueue.wait(), nullptr);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ANY);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ANY);

    MicroByteEvent event1 = MicroByteEvent();

    eventQueue.post(&event1, thread1);
    eventQueue.post(&event1, thread1);
    eventQueue.post(&event1, thread1);
    eventQueue.post(&event1, thread1);

    // posting same event multiple times will not increse eventQueue

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    EXPECT_EQ(eventQueue.release(&event1), -1);

    // failed to release event that is still on the queue

    EXPECT_EQ(eventQueue.wait(), &event1);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    // successfully receive the event

    EXPECT_EQ(eventQueue.release(&event1), 1);

    // event wasn't in the queue, so we can release it now

    EXPECT_EQ(eventQueue.wait(), nullptr); // this will clear thread flags
    EXPECT_EQ(eventQueue.wait(), nullptr); // this will set current thread state to FLAG BLOCKED

    // nothing on the queue, thread1 will be in FLAG BLOCKED state

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ANY);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ANY);

    eventQueue.post(&event1, thread1);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    MicroByteEvent *ev = eventQueue.get();

    // event get() will automatically release the event so no need to call event
    // release

    EXPECT_NE(ev, nullptr);
    EXPECT_EQ(ev, &event1);

    EXPECT_EQ(eventQueue.wait(), nullptr);
    EXPECT_EQ(eventQueue.wait(), nullptr);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ANY);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ANY);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] post multiple different events
     * -------------------------------------------------------------------------
     **/

    MicroByteEvent event2 = MicroByteEvent();
    MicroByteEvent event3 = MicroByteEvent();
    MicroByteEvent event4 = MicroByteEvent();
    MicroByteEvent event5 = MicroByteEvent();

    eventQueue.post(&event1, thread1);
    eventQueue.post(&event2, thread1);
    eventQueue.post(&event3, thread1);
    eventQueue.post(&event4, thread1);
    eventQueue.post(&event5, thread1);

    EXPECT_EQ(eventQueue.pending(), 5);
    EXPECT_EQ(eventQueue.peek(), &event1);

    eventQueue.cancel(&event3);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    ev = eventQueue.get();
    EXPECT_NE(ev, nullptr);
    EXPECT_EQ(ev, &event1);

    ev = eventQueue.get();
    EXPECT_NE(ev, nullptr);
    EXPECT_EQ(ev, &event2);

    ev = eventQueue.get();
    EXPECT_NE(ev, nullptr);
    EXPECT_EQ(ev, &event4);

    EXPECT_EQ(eventQueue.wait(), &event5);
    EXPECT_EQ(eventQueue.release(&event5), 1);

    ev = eventQueue.get();

    EXPECT_EQ(ev, nullptr);

    EXPECT_EQ(eventQueue.wait(), nullptr);
    EXPECT_EQ(eventQueue.wait(), nullptr);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ANY);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ANY);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] cancel event and post it again
     * -------------------------------------------------------------------------
     **/

    eventQueue.post(&event3, thread1);
    eventQueue.post(&event4, thread1);
    eventQueue.post(&event5, thread1);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    eventQueue.cancel(&event3);

    eventQueue.post(&event3, thread1);

    // after cancel it, event3 position will be at the end of the queue

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    EXPECT_EQ(eventQueue.wait(), &event4);
    EXPECT_EQ(eventQueue.release(&event4), 1);

    EXPECT_EQ(eventQueue.wait(), &event5);
    EXPECT_EQ(eventQueue.release(&event5), 1);

    EXPECT_EQ(eventQueue.wait(), &event3);
    EXPECT_EQ(eventQueue.release(&event3), 1);

    ev = eventQueue.get();

    EXPECT_EQ(ev, nullptr);

    EXPECT_EQ(eventQueue.wait(), nullptr);
    EXPECT_EQ(eventQueue.wait(), nullptr);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ANY);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_FLAG_BLOCKED_ANY);

    CustomEvent customEv = CustomEvent();

    customEv.data = 0xdeadbeef;

    eventQueue.post(reinterpret_cast<MicroByteEvent *>(&customEv), thread1);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    CustomEvent *event = (CustomEvent *)eventQueue.wait();
    eventQueue.release(reinterpret_cast<MicroByteEvent *>(event));

    EXPECT_NE(event, nullptr);
    EXPECT_EQ(event->data, 0xdeadbeef);
}
