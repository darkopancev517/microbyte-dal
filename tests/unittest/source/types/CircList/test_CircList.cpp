#include "gtest/gtest.h"

#include "CircList.h"

class TestCircList : public testing::Test
{
    protected:

    CircList *obj;

    virtual void SetUp()
    {
        obj = new CircList();
    }

    virtual void TearDown()
    {
        delete obj;
    }
};

TEST_F(TestCircList, constructorTest)
{
    EXPECT_TRUE(obj);
}

TEST_F(TestCircList, rightPushTest)
{
    CircList node1;
    CircList node2;
    CircList node3;
    CircList node4;

    obj->rightPush(&node1);

    /* obj->1->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node1);

    obj->rightPush(&node2);

    /* obj->2->1->2 */

    EXPECT_EQ(obj->next, &node2);
    EXPECT_EQ(obj->next->next, &node1);
    EXPECT_EQ(obj->next->next->next, &node2);

    obj->rightPush(&node3);

    /* obj->3->1->2->3 */

    EXPECT_EQ(obj->next, &node3);
    EXPECT_EQ(obj->next->next, &node1);
    EXPECT_EQ(obj->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next, &node3);

    obj->rightPush(&node4);

    /* obj->4->1->2->3->4 */

    EXPECT_EQ(obj->next, &node4);
    EXPECT_EQ(obj->next->next, &node1);
    EXPECT_EQ(obj->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next->next, &node4);
}

TEST_F(TestCircList, leftPushTest)
{
    CircList node1;
    CircList node2;
    CircList node3;
    CircList node4;

    obj->leftPush(&node1);

    /* obj->1->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node1);

    obj->leftPush(&node2);

    /* obj->1->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node2);
    EXPECT_EQ(obj->next->next->next, &node1);

    obj->leftPush(&node3);

    /* obj->1->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node3);
    EXPECT_EQ(obj->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next, &node1);

    obj->leftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);
}

TEST_F(TestCircList, leftPopTest)
{
    CircList node1;
    CircList node2;
    CircList node3;
    CircList node4;

    obj->leftPush(&node1);
    obj->leftPush(&node2);
    obj->leftPush(&node3);
    obj->leftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    EXPECT_EQ(obj->leftPop(), &node4);

    /* obj->1->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node3);
    EXPECT_EQ(obj->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next, &node1);

    EXPECT_EQ(obj->leftPop(), &node3);
    EXPECT_EQ(obj->leftPop(), &node2);

    /* obj->1->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node1);

    EXPECT_EQ(obj->leftPop(), &node1);

    /* obj->null */

    EXPECT_EQ(obj->next, nullptr);
}

TEST_F(TestCircList, leftPopRightPushTest)
{
    CircList node1;
    CircList node2;
    CircList node3;
    CircList node4;

    obj->leftPush(&node1);
    obj->leftPush(&node2);
    obj->leftPush(&node3);
    obj->leftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    obj->leftPopRightPush();

    /* obj->4->3->2->1->4 */

    EXPECT_EQ(obj->next, &node4);
    EXPECT_EQ(obj->next->next, &node3);
    EXPECT_EQ(obj->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next, &node1);
    EXPECT_EQ(obj->next->next->next->next->next, &node4);

    obj->leftPopRightPush();

    /* obj->3->2->1->4->3 */

    EXPECT_EQ(obj->next, &node3);
    EXPECT_EQ(obj->next->next, &node2);
    EXPECT_EQ(obj->next->next->next, &node1);
    EXPECT_EQ(obj->next->next->next->next, &node4);
    EXPECT_EQ(obj->next->next->next->next->next, &node3);

    /* obj->2->1->4->3->2 */

    obj->leftPopRightPush();

    EXPECT_EQ(obj->next, &node2);
    EXPECT_EQ(obj->next->next, &node1);
    EXPECT_EQ(obj->next->next->next, &node4);
    EXPECT_EQ(obj->next->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next->next, &node2);

    obj->leftPopRightPush();

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);
}

TEST_F(TestCircList, countTest)
{
    CircList node1;
    CircList node2;
    CircList node3;
    CircList node4;

    obj->leftPush(&node1);
    obj->leftPush(&node2);
    obj->leftPush(&node3);
    obj->leftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    EXPECT_EQ(obj->count(), 4);
}

TEST_F(TestCircList, removeTest)
{
    CircList node1;
    CircList node2;
    CircList node3;
    CircList node4;

    obj->leftPush(&node1);
    obj->leftPush(&node2);
    obj->leftPush(&node3);
    obj->leftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    EXPECT_EQ(obj->remove(&node3), &node3);

    /* obj->1->4->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next, &node1);

    EXPECT_EQ(obj->remove(&node1), &node1);

    /* obj->2->4->2 */

    EXPECT_EQ(obj->next, &node2);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node2);

    EXPECT_EQ(obj->remove(&node4), &node4);

    /* obj->2->2 */

    EXPECT_EQ(obj->next, &node2);
    EXPECT_EQ(obj->next->next, &node2);

    EXPECT_EQ(obj->remove(&node2), &node2);

    /* obj->null */

    EXPECT_EQ(obj->next, nullptr);
}

TEST_F(TestCircList, findTest)
{
    CircList node1;
    CircList node2;
    CircList node3;
    CircList node4;

    obj->leftPush(&node1);
    obj->leftPush(&node2);
    obj->leftPush(&node3);
    obj->leftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    EXPECT_EQ(obj->find(&node1), &node1);
    EXPECT_EQ(obj->find(&node3), &node3);
    EXPECT_EQ(obj->find(&node2), &node2);
    EXPECT_EQ(obj->find(&node4), &node4);

    EXPECT_EQ(obj->remove(&node3), &node3);

    EXPECT_EQ(obj->find(&node3), nullptr);

    /* obj->1->4->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next, &node1);
}

TEST_F(TestCircList, findBeforeTest)
{
    CircList node1;
    CircList node2;
    CircList node3;
    CircList node4;

    obj->leftPush(&node1);
    obj->leftPush(&node2);
    obj->leftPush(&node3);
    obj->leftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    EXPECT_EQ(obj->findBefore(&node3), &node4);
    EXPECT_EQ(obj->findBefore(&node4), &node1);
    EXPECT_EQ(obj->findBefore(&node1), &node2);
    EXPECT_EQ(obj->findBefore(&node2), &node3);
}

TEST_F(TestCircList, rightPopTest)
{
    CircList node1;
    CircList node2;
    CircList node3;
    CircList node4;

    obj->leftPush(&node1);
    obj->leftPush(&node2);
    obj->leftPush(&node3);
    obj->leftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    EXPECT_EQ(obj->rightPop(), &node1);

    /* obj->2->4->3->2 */

    EXPECT_EQ(obj->next, &node2);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);

    EXPECT_EQ(obj->rightPop(), &node2);

    /* obj->3->4->3 */

    EXPECT_EQ(obj->next, &node3);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);

    EXPECT_EQ(obj->rightPop(), &node3);

    /* obj->4->4 */

    EXPECT_EQ(obj->next, &node4);
    EXPECT_EQ(obj->next->next, &node4);

    EXPECT_EQ(obj->rightPop(), &node4);

    /* obj->null */

    EXPECT_EQ(obj->next, nullptr);
}

TEST_F(TestCircList, peekTest)
{
    CircList node1;
    CircList node2;
    CircList node3;
    CircList node4;

    obj->leftPush(&node1);
    obj->leftPush(&node2);
    obj->leftPush(&node3);
    obj->leftPush(&node4);

    /* obj->1->4->3->2->1 */

    EXPECT_EQ(obj->next, &node1);
    EXPECT_EQ(obj->next->next, &node4);
    EXPECT_EQ(obj->next->next->next, &node3);
    EXPECT_EQ(obj->next->next->next->next, &node2);
    EXPECT_EQ(obj->next->next->next->next->next, &node1);

    EXPECT_EQ(obj->leftPeek(), &node4);
    EXPECT_EQ(obj->rightPeek(), &node1);

    /* obj->1->3->2->1 */

    EXPECT_EQ(obj->leftPop(), &node4);

    EXPECT_EQ(obj->leftPeek(), &node3);
    EXPECT_EQ(obj->rightPeek(), &node1);

    /* obj->2->3->2 */

    EXPECT_EQ(obj->rightPop(), &node1);

    EXPECT_EQ(obj->next, &node2);
    EXPECT_EQ(obj->next->next, &node3);
    EXPECT_EQ(obj->next->next->next, &node2);

    EXPECT_EQ(obj->leftPeek(), &node3);
    EXPECT_EQ(obj->rightPeek(), &node2);

    EXPECT_EQ(obj->leftPop(), &node3);

    /* obj->2->2 */

    EXPECT_EQ(obj->next, &node2);
    EXPECT_EQ(obj->next->next, &node2);

    EXPECT_EQ(obj->leftPeek(), &node2);
    EXPECT_EQ(obj->rightPeek(), &node2);

    EXPECT_EQ(obj->rightPop(), &node2);

    /* obj->null */

    EXPECT_EQ(obj->next, nullptr);

    EXPECT_EQ(obj->leftPeek(), nullptr);
    EXPECT_EQ(obj->rightPeek(), nullptr);
}
