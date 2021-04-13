#include <gtest/gtest.h>

#include "follower.hpp"
#include "leader.hpp"



TEST(NodeTest, LeaderTest) {

    bool leader = true;
    string myPort = "5555";
    int threads = 2;

    Node node(myPort, leader, threads);

    EXPECT_EQ(node.isFollower(), false);  
}

TEST(NodeTest, GetMNodesTest) {

    bool leader = true;
    string myPort = "5555";
    int threads = 2;

    Node node(myPort, leader, threads);
    EXPECT_EQ(node.getMNodes().size(), 0);

    std::vector<Message::node> nodes;
    nodes.push_back(Message::node("a","b","c"));
    node.setMNodes(nodes);

    std::vector<Message::node> nodes2 = node.getMNodes();

    EXPECT_EQ(nodes2.size(), 1);
    EXPECT_EQ(nodes2[0], Message::node("a","b","c"));
}

TEST(NodeTest, SetParametersTest) {

    bool leader = true;
    string myPort = "5555";
    int threads = 2;

    Node node(myPort, leader, threads);

    int A[9];
    A[0] = 30;
    A[1] = 30;
    A[2] = 30;
    A[3] = 100;
    A[4] = 60000;
    A[5] = 1;
    A[6] = 10;
    A[7] = 20;
    A[8] = 120;

    auto fun = [&A,&node]() {
        EXPECT_EQ(node.timeReport, A[0]);
        EXPECT_EQ(node.timeTests, A[1]);
        EXPECT_EQ(node.timeLatency, A[2]);
        EXPECT_EQ(node.maxPerLatency, A[3]);
        EXPECT_EQ(node.timeBandwidth, A[4]);
        EXPECT_EQ(node.maxPerBandwidth, A[5]);
        EXPECT_EQ(node.leaderCheck , A[6]);
        EXPECT_EQ(node.timePropagation, A[7]);
        EXPECT_EQ(node.timeheartbeat  , A[8]);
    };
    fun();

    node.setParam("time-report", 31);
    A[0] = 31;
    fun();

    node.setParam("time-tests", 0);
    A[1] = 0;
    fun();

    node.setParam("time-latency", 87);
    A[2] = 87;
    fun();

    node.setParam("max-per-latency", 17);
    A[3] = 17;
    fun();

    node.setParam("time-bandwidth", 67);
    A[4] = 67;
    fun();

    node.setParam("max-per-bandwidth", 12);
    A[5] = 12;
    fun();

    node.setParam("leader-check", 3121);
    A[6] = 3121;
    fun();

    node.setParam("time-propagation", 432);
    A[7] = 432;
    fun();

    node.setParam("heartbeat", 32);
    A[8] = 32;
    fun();
}