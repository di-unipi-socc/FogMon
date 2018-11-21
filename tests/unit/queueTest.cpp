#include "queue.hpp"
#include <gtest/gtest.h>


TEST(QueueTest2,PushPop2){
    Queue<int> queue;
    queue.startqueue();
    int a=3;
    queue.push(a);
    queue.pop(&a);
EXPECT_EQ(3,a);
};

TEST(QueueTest2,PushPop3){
    Queue<int> queue;
    queue.startqueue();
    int a=4;
    queue.push(a);
    queue.pop(&a);
EXPECT_EQ(4,a);
};