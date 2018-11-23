#include "queue.hpp"
#include <gtest/gtest.h>


TEST(QueueTest,PushPop) {
    Queue<int> queue;
    queue.startqueue();
    int a=3;
    queue.push(a);
    queue.pop(&a);
    EXPECT_EQ(3,a);
    queue.stopqueue();
};

TEST(QueueTest,PushPopOrder) {
    Queue<int> queue;
    queue.startqueue();
    for(int i=0; i<10; i++) {
        queue.push(i);
    }
    int a = 99;
    for(int i=0; i<10; i++) {
        queue.pop(&a);
        EXPECT_EQ(i,a);
    }
    queue.stopqueue();
};