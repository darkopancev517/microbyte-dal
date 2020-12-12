#include "gtest/gtest.h"

#include "MicroByteMsg.h"
#include "MicroByteThread.h"
#include "MicroByteUnitTest.h"
#include "Utils.h"

class TestMicroByteMsg : public testing::Test
{
    protected:

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(TestMicroByteMsg, singleMsgTest)
{
    MicroByteCpuTest cpuTest;

    MicroByteScheduler *scheduler = &MicroByteScheduler::init();

    EXPECT_NE(scheduler, nullptr);
    EXPECT_EQ(scheduler->numOfThreads(), 0);
    EXPECT_EQ(scheduler->activeThread(), nullptr);
    EXPECT_EQ(scheduler->activePid(), MICROBYTE_THREAD_PID_UNDEF);

    MicroByteMsg mainThreadMsgQueue[MICROBYTE_CONFIG_MSG_QUEUE_SIZE];
    MicroByteMsg thread1MsgQueue[MICROBYTE_CONFIG_MSG_QUEUE_SIZE];

    char idleStack[128];

    MicroByteThread *idleThread = MicroByteThread::init(idleStack, sizeof(idleStack),
                                      MICROBYTE_THREAD_PRIORITY_IDLE,
                                      MICROBYTE_THREAD_FLAGS_WOUT_YIELD |
                                      MICROBYTE_THREAD_FLAGS_STACKMARKER,
                                      NULL, NULL, "idle");

    EXPECT_EQ(idleThread->hasMsgQueue(), 0);
    EXPECT_EQ(idleThread->numOfMsgInQueue(), -1);

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

    mainThread->setMsgQueue(mainThreadMsgQueue, ARRAY_LENGTH(mainThreadMsgQueue));

    EXPECT_EQ(mainThread->hasMsgQueue(), 1);
    EXPECT_EQ(mainThread->numOfMsgInQueue(), 0);

    EXPECT_NE(mainThread, nullptr);
    EXPECT_EQ(mainThread->getPid(), 2);
    EXPECT_EQ(mainThread->getPriority(), MICROBYTE_THREAD_PRIORITY_MAIN);
    EXPECT_EQ(mainThread->getName(), "main");
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    char stack1[128];

    MicroByteThread *thread1 = MicroByteThread::init(stack1, sizeof(stack1),
                                   MICROBYTE_THREAD_PRIORITY_MAIN - 1,
                                   MICROBYTE_THREAD_FLAGS_WOUT_YIELD |
                                   MICROBYTE_THREAD_FLAGS_STACKMARKER,
                                   NULL, NULL, "thread1");

    thread1->setMsgQueue(thread1MsgQueue, ARRAY_LENGTH(thread1MsgQueue));

    EXPECT_EQ(thread1->hasMsgQueue(), 1);
    EXPECT_EQ(thread1->numOfMsgInQueue(), 0);

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
    EXPECT_EQ(scheduler->activeThread(), nullptr);
    EXPECT_EQ(scheduler->activePid(), MICROBYTE_THREAD_PID_UNDEF);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] send message to a thread without msgQueue
     * -------------------------------------------------------------------------
     **/

    MicroByteMsg msg = MicroByteMsg();

    EXPECT_EQ(msg.send(idleThread->getPid()), -1);
    EXPECT_EQ(msg.send(idleThread->getPid()), -1);
    EXPECT_EQ(msg.send(idleThread->getPid()), -1);
    EXPECT_EQ(msg.send(idleThread->getPid()), -1);

    // idleThread doesn't have  msgQueue, send message to idleThread will failed

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] basic send and receive message
     * -------------------------------------------------------------------------
     **/

    MicroByteMsg msg1 = MicroByteMsg();

    msg1.receive();

    EXPECT_EQ(msg1.senderPid, MICROBYTE_THREAD_PID_UNDEF);
    EXPECT_EQ(msg1.type, 0);
    EXPECT_EQ(msg1.content.ptr, nullptr);
    EXPECT_EQ(msg1.content.value, 0);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED);

    MicroByteMsg msg2 = MicroByteMsg();

    EXPECT_EQ(msg2.senderPid, MICROBYTE_THREAD_PID_UNDEF);
    EXPECT_EQ(msg2.type, 0);
    EXPECT_EQ(msg2.content.ptr, nullptr);
    EXPECT_EQ(msg2.content.value, 0);

    uint32_t msgValue = 0xdeadbeef;

    msg2.type = 0x20;
    msg2.content.ptr = static_cast<void *>(&msgValue);

    EXPECT_EQ(msg2.send(thread1->getPid()), 1);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    // msg2 immediately received because thread1 was in RECEIVE BLOCKED status

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    EXPECT_EQ(msg1.senderPid, mainThread->getPid());
    EXPECT_EQ(msg1.type, 0x20);
    EXPECT_NE(msg1.content.ptr, nullptr);
    EXPECT_EQ(*static_cast<uint32_t *>(msg1.content.ptr), 0xdeadbeef);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] send message (blocking)
     * -------------------------------------------------------------------------
     **/

    MicroByteMsg msg3 = MicroByteMsg();

    EXPECT_EQ(msg3.senderPid, MICROBYTE_THREAD_PID_UNDEF);
    EXPECT_EQ(msg3.type, 0);
    EXPECT_EQ(msg3.content.ptr, nullptr);
    EXPECT_EQ(msg3.content.value, 0);

    msg3.type = 0x21;
    msg3.content.value = 0x12345678;

    EXPECT_EQ(msg3.send(mainThread->getPid()), 1);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    EXPECT_EQ(mainThread->numOfMsgInQueue(), 1);

    scheduler->sleep();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    MicroByteMsg msg4 = MicroByteMsg();

    EXPECT_EQ(msg4.senderPid, MICROBYTE_THREAD_PID_UNDEF);
    EXPECT_EQ(msg4.type, 0);
    EXPECT_EQ(msg4.content.ptr, nullptr);
    EXPECT_EQ(msg4.content.value, 0);

    EXPECT_EQ(msg4.receive(), 1);

    // Expected to receive msg3 content that was sent by thread1

    EXPECT_EQ(msg4.senderPid, thread1->getPid());
    EXPECT_EQ(msg4.type, 0x21);
    EXPECT_EQ(msg4.content.value, 0x12345678);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    EXPECT_EQ(mainThread->numOfMsgInQueue(), 0);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    EXPECT_EQ(scheduler->wakeUpThread(thread1->getPid()), 1);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] try send message (non-blocking)
     * -------------------------------------------------------------------------
     **/

    msg3.type = 0x22;
    msg3.content.value = 0xaaaaaaaa;

    EXPECT_EQ(msg3.trySend(mainThread->getPid()), 1);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    EXPECT_EQ(mainThread->numOfMsgInQueue(), 1);

    scheduler->sleep();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    EXPECT_EQ(msg4.receive(), 1);

    // Expected to receive msg3 content that was sent by thread1

    EXPECT_EQ(msg4.senderPid, thread1->getPid());
    EXPECT_EQ(msg4.type, 0x22);
    EXPECT_EQ(msg4.content.value, 0xaaaaaaaa);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    EXPECT_EQ(mainThread->numOfMsgInQueue(), 0);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    EXPECT_EQ(scheduler->wakeUpThread(thread1->getPid()), 1);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] send message from ISR
     * -------------------------------------------------------------------------
     **/
    msg3.type = 0x23;
    msg3.content.value = 0xbbbbbbbb;

    cpuTest.setInIsr(1);
    EXPECT_EQ(cpuTest.inIsr(), 1);
    EXPECT_EQ(msg3.send(thread1->getPid()), 1);
    cpuTest.setInIsr(0);
    EXPECT_EQ(cpuTest.inIsr(), 0);

    EXPECT_EQ(thread1->numOfMsgInQueue(), 1);

    EXPECT_EQ(msg4.receive(), 1);

    EXPECT_EQ(thread1->numOfMsgInQueue(), 0);

    EXPECT_EQ(msg4.senderPid, MICROBYTE_THREAD_PID_ISR);
    EXPECT_EQ(msg4.type, 0x23);
    EXPECT_EQ(msg4.content.value, 0xbbbbbbbb);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] send and then immediately set to receive state by calling
     * sendReceive function.
     * -------------------------------------------------------------------------
     **/

    msg3.type = 0x24;
    msg3.content.value = 0xcccccccc;

    EXPECT_EQ(msg3.sendReceive(&msg4, mainThread->getPid()), 1);

    // Note: replied message will be update on msg4

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_REPLY_BLOCKED);

    EXPECT_EQ(mainThread->numOfMsgInQueue(), 1);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_REPLY_BLOCKED);

    MicroByteMsg msg5 = MicroByteMsg();

    EXPECT_EQ(msg5.receive(), 1);

    EXPECT_EQ(msg5.type, 0x24);
    EXPECT_EQ(msg5.content.value, 0xcccccccc);
    EXPECT_EQ(msg5.senderPid, thread1->getPid());

    EXPECT_EQ(msg5.tryReceive(), -1);
    EXPECT_EQ(mainThread->numOfMsgInQueue(), 0);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_REPLY_BLOCKED);

    MicroByteMsg msg6 = MicroByteMsg();

    msg6.type = 0xff;
    msg6.content.value = 0xaaaacccc;

    EXPECT_EQ(msg5.reply(&msg6), 1);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();    

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    // replied message received by msg4 as expected

    EXPECT_EQ(msg4.type, 0xff);
    EXPECT_EQ(msg4.content.value, 0xaaaacccc);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] send and then immediately set to receive state by calling
     * sendReceive function. reply function would be in Isr.
     * -------------------------------------------------------------------------
     **/

    msg3.type = 0x24;
    msg3.content.value = 0xddddcccc;

    EXPECT_EQ(msg3.sendReceive(&msg4, mainThread->getPid()), 1);

    // Note: replied message will be update on msg4

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_REPLY_BLOCKED);

    EXPECT_EQ(mainThread->numOfMsgInQueue(), 1);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_REPLY_BLOCKED);

    EXPECT_EQ(msg5.receive(), 1);

    EXPECT_EQ(msg5.type, 0x24);
    EXPECT_EQ(msg5.content.value, 0xddddcccc);
    EXPECT_EQ(msg5.senderPid, thread1->getPid());

    EXPECT_EQ(msg5.tryReceive(), -1);
    EXPECT_EQ(mainThread->numOfMsgInQueue(), 0);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_REPLY_BLOCKED);

    msg6.type = 0xee;
    msg6.content.value = 0xccccdddd;

    cpuTest.setInIsr(1);
    EXPECT_EQ(cpuTest.inIsr(), 1);

    EXPECT_EQ(msg5.replyInIsr(&msg6), 1);

    cpuTest.setInIsr(0);
    EXPECT_EQ(cpuTest.inIsr(), 0);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    EXPECT_EQ(scheduler->requestedContextSwitch(), 1);

    scheduler->run();    

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    // replied message received by msg4 as expected

    EXPECT_EQ(msg4.type, 0xee);
    EXPECT_EQ(msg4.content.value, 0xccccdddd);
}

TEST_F(TestMicroByteMsg, multipleMsgTest)
{
    MicroByteCpuTest cpuTest;

    MicroByteScheduler *scheduler = &MicroByteScheduler::init();

    EXPECT_NE(scheduler, nullptr);
    EXPECT_EQ(scheduler->numOfThreads(), 0);
    EXPECT_EQ(scheduler->activeThread(), nullptr);
    EXPECT_EQ(scheduler->activePid(), MICROBYTE_THREAD_PID_UNDEF);

    MicroByteMsg idleThreadMsgQueue[MICROBYTE_CONFIG_MSG_QUEUE_SIZE];
    MicroByteMsg mainThreadMsgQueue[MICROBYTE_CONFIG_MSG_QUEUE_SIZE];
    MicroByteMsg thread1MsgQueue[MICROBYTE_CONFIG_MSG_QUEUE_SIZE];

    char idleStack[128];

    MicroByteThread *idleThread = MicroByteThread::init(idleStack, sizeof(idleStack),
                                      MICROBYTE_THREAD_PRIORITY_IDLE,
                                      MICROBYTE_THREAD_FLAGS_WOUT_YIELD |
                                      MICROBYTE_THREAD_FLAGS_STACKMARKER,
                                      NULL, NULL, "idle");

    idleThread->setMsgQueue(idleThreadMsgQueue, ARRAY_LENGTH(idleThreadMsgQueue));

    EXPECT_EQ(idleThread->hasMsgQueue(), 1);
    EXPECT_EQ(idleThread->numOfMsgInQueue(), 0);

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

    mainThread->setMsgQueue(mainThreadMsgQueue, ARRAY_LENGTH(mainThreadMsgQueue));

    EXPECT_EQ(mainThread->hasMsgQueue(), 1);
    EXPECT_EQ(mainThread->numOfMsgInQueue(), 0);

    EXPECT_NE(mainThread, nullptr);
    EXPECT_EQ(mainThread->getPid(), 2);
    EXPECT_EQ(mainThread->getPriority(), MICROBYTE_THREAD_PRIORITY_MAIN);
    EXPECT_EQ(mainThread->getName(), "main");
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    char stack1[128];

    MicroByteThread *thread1 = MicroByteThread::init(stack1, sizeof(stack1),
                                   MICROBYTE_THREAD_PRIORITY_MAIN - 1,
                                   MICROBYTE_THREAD_FLAGS_WOUT_YIELD |
                                   MICROBYTE_THREAD_FLAGS_STACKMARKER,
                                   NULL, NULL, "thread1");

    thread1->setMsgQueue(thread1MsgQueue, ARRAY_LENGTH(thread1MsgQueue));

    EXPECT_EQ(thread1->hasMsgQueue(), 1);
    EXPECT_EQ(thread1->numOfMsgInQueue(), 0);

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
    EXPECT_EQ(scheduler->activeThread(), nullptr);
    EXPECT_EQ(scheduler->activePid(), MICROBYTE_THREAD_PID_UNDEF);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] set current active thread to sleep status and send msg to it,
     * it expected to queued the msg.
     * -------------------------------------------------------------------------
     **/

    scheduler->sleep();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    MicroByteMsg msg1 = MicroByteMsg();
    MicroByteMsg msg2 = MicroByteMsg();
    MicroByteMsg msg3 = MicroByteMsg();
    MicroByteMsg msg4 = MicroByteMsg();

    msg1.type = 0xff;
    msg1.content.value = 0x1;

    msg2.type = 0xff;
    msg2.content.value = 0x2;

    msg3.type = 0xff;
    msg3.content.value = 0x3;

    msg4.type = 0xff;
    msg4.content.value = 0x4;

    EXPECT_EQ(msg1.send(thread1->getPid()), 1);
    EXPECT_EQ(msg2.send(thread1->getPid()), 1);
    EXPECT_EQ(msg3.send(thread1->getPid()), 1);
    EXPECT_EQ(msg4.send(thread1->getPid()), 1);

    EXPECT_EQ(thread1->numOfMsgInQueue(), 4);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    EXPECT_EQ(scheduler->wakeUpThread(thread1->getPid()), 1);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    MicroByteMsg msg5 = MicroByteMsg();

    EXPECT_EQ(msg5.receive(), 1);

    EXPECT_EQ(msg5.senderPid, mainThread->getPid());
    EXPECT_EQ(msg5.type, 0xff);
    EXPECT_EQ(msg5.content.value, 0x1);

    EXPECT_EQ(msg5.receive(), 1);

    EXPECT_EQ(msg5.senderPid, mainThread->getPid());
    EXPECT_EQ(msg5.type, 0xff);
    EXPECT_EQ(msg5.content.value, 0x2);

    EXPECT_EQ(msg5.receive(), 1);

    EXPECT_EQ(msg5.senderPid, mainThread->getPid());
    EXPECT_EQ(msg5.type, 0xff);
    EXPECT_EQ(msg5.content.value, 0x3);

    EXPECT_EQ(msg5.receive(), 1);

    EXPECT_EQ(msg5.senderPid, mainThread->getPid());
    EXPECT_EQ(msg5.type, 0xff);
    EXPECT_EQ(msg5.content.value, 0x4);

    EXPECT_EQ(thread1->numOfMsgInQueue(), 0);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    scheduler->sleep();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    // Send message to thread1 until it message queue overflow

    EXPECT_EQ(msg1.send(thread1->getPid()), 1);
    EXPECT_EQ(msg2.send(thread1->getPid()), 1);
    EXPECT_EQ(msg3.send(thread1->getPid()), 1);
    EXPECT_EQ(msg4.send(thread1->getPid()), 1);

    EXPECT_EQ(thread1->numOfMsgInQueue(), 4);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    MicroByteMsg msg6 = MicroByteMsg();

    msg6.type = 0xff;
    msg6.content.value = 0xdeadbeef;

    EXPECT_EQ(msg6.send(thread1->getPid()), 1);

    EXPECT_EQ(thread1->numOfMsgInQueue(), 4);

    // Notice that num of message in thread1 msgQueue doesn't increase, it
    // already reach it's limit

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_SEND_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    // thread1 msgQueue was full, so the mainThread will go into SEND BLOCKED
    // state to send msg6 until all messages in thread1 msgQueue is receive

    EXPECT_EQ(scheduler->wakeUpThread(thread1->getPid()), 1);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_SEND_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_SEND_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    EXPECT_EQ(msg5.receive(), 1);

    EXPECT_EQ(msg5.senderPid, mainThread->getPid());
    EXPECT_EQ(msg5.type, 0xff);
    EXPECT_EQ(msg5.content.value, 0x1); // msg1

    EXPECT_EQ(msg5.receive(), 1);

    EXPECT_EQ(msg5.senderPid, mainThread->getPid());
    EXPECT_EQ(msg5.type, 0xff);
    EXPECT_EQ(msg5.content.value, 0x2); // msg2

    EXPECT_EQ(msg5.receive(), 1);

    EXPECT_EQ(msg5.senderPid, mainThread->getPid());
    EXPECT_EQ(msg5.type, 0xff);
    EXPECT_EQ(msg5.content.value, 0x3); // msg3

    EXPECT_EQ(msg5.receive(), 1);

    EXPECT_EQ(msg5.senderPid, mainThread->getPid());
    EXPECT_EQ(msg5.type, 0xff);
    EXPECT_EQ(msg5.content.value, 0x4); // msg4

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_SEND_BLOCKED);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    EXPECT_EQ(thread1->numOfMsgInQueue(), 0);

    // All messages have been received from thread1 msgQueue, to receive pending
    // msg6 we need to call receive() once again and sender thread (mainThread)
    // should no longer in SEND BLOCKED state if msg6 receive successfully

    EXPECT_EQ(msg5.receive(), 1);

    EXPECT_EQ(msg5.senderPid, mainThread->getPid());
    EXPECT_EQ(msg5.type, 0xff);
    EXPECT_EQ(msg5.content.value, 0xdeadbeef); // msg6

    // Now msg6 was successfully sent and mainThread (sender) will no longer in
    // SEND BLOCKED state

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    msg5.type = 0;
    msg5.content.value = 0;

    EXPECT_EQ(msg5.receive(), 1);

    EXPECT_EQ(msg5.type, 0);
    EXPECT_EQ(msg5.content.value, 0);

    // Nothing was on the queue, thread1 will go into RECEIVE BLOCKED state

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RECEIVE_BLOCKED);

    EXPECT_EQ(msg1.send(thread1->getPid()), 1);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);

    EXPECT_EQ(msg5.receive(), 1);

    EXPECT_EQ(msg5.type, 0xff);
    EXPECT_EQ(msg5.content.value, 0x1);

    EXPECT_EQ(thread1->numOfMsgInQueue(), 0);

    scheduler->sleep();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    scheduler->run();

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    // Use trySend() function (non-blocking) when target msgQueue is full

    EXPECT_EQ(msg1.send(thread1->getPid()), 1);
    EXPECT_EQ(msg2.send(thread1->getPid()), 1);
    EXPECT_EQ(msg3.send(thread1->getPid()), 1);
    EXPECT_EQ(msg4.send(thread1->getPid()), 1);

    EXPECT_EQ(thread1->numOfMsgInQueue(), 4);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    EXPECT_EQ(msg6.trySend(thread1->getPid()), 0);
    EXPECT_EQ(msg6.trySend(thread1->getPid()), 0);
    EXPECT_EQ(msg6.trySend(thread1->getPid()), 0);
    EXPECT_EQ(msg6.trySend(thread1->getPid()), 0);
    EXPECT_EQ(msg6.trySend(thread1->getPid()), 0);
    EXPECT_EQ(msg6.trySend(thread1->getPid()), 0);

    // thread1 msgQueue is full, so all trySend attempt was failed as expected
    // and mainThread should still in RUNNING state

    EXPECT_EQ(thread1->numOfMsgInQueue(), 4);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    /**
     * -------------------------------------------------------------------------
     * [TEST CASE] send message to itself
     * -------------------------------------------------------------------------
     **/

    EXPECT_EQ(msg6.sendToSelf(), 1);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);

    EXPECT_EQ(mainThread->numOfMsgInQueue(), 1);

    msg5.type = 0;
    msg5.content.value = 0;

    EXPECT_EQ(msg5.receive(), 1);

    EXPECT_EQ(msg5.type, 0xff);
    EXPECT_EQ(msg5.content.value, 0xdeadbeef); // msg6

    EXPECT_EQ(mainThread->numOfMsgInQueue(), 0);

    EXPECT_EQ(idleThread->getStatus(), MICROBYTE_THREAD_STATUS_PENDING);
    EXPECT_EQ(mainThread->getStatus(), MICROBYTE_THREAD_STATUS_RUNNING);
    EXPECT_EQ(thread1->getStatus(), MICROBYTE_THREAD_STATUS_SLEEPING);
}
