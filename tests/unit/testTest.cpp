#include <gtest/gtest.h>
#include "follower.hpp"
#include <iostream>

using namespace std;

class MFollower : public Follower {
public:
    MFollower() : Follower(Message::node("a","::1","12345"), 0) {}
    int testPing(std::string host) override { return Follower::testPing(host); }
    float testBandwidthIperf(std::string host, int port) override { return Follower::testBandwidthIperf(host, port); }
    int startIperf() override { return Follower::startIperf(); }

    float testBandwidthEstimate(std::string ip, std::string myIp, float old) override { return Follower::testBandwidthEstimate(ip, myIp, old); }
    int startEstimate() override { return Follower::startEstimate(); }
};

TEST(testTest, pingTest) {
    MFollower f;
    int ping = f.testPing("google.com");

    cout << "Ping to google.com: " << ping << "ms" << endl;
    // ping >= 0
    ASSERT_GE(ping, 0);
}

TEST(testTest, pingFailTest) {
    MFollower f;
    int ping = f.testPing("a");

    cout << "Ping to google.com: " << ping << "ms" << endl;
    // ping == -1 (failure)
    ASSERT_EQ(ping, -1);
}

TEST(testTest, iperTest) {
    MFollower f;
    // start iperf
    int r = f.startIperf();
    // r == 0 (success)
    ASSERT_EQ(r, 0);
    int port = f.getIperfPort();
    // port >= 0
    ASSERT_GE(port, 0);
    // test with iperf
    float bw = f.testBandwidthIperf("localhost", port);
    f.stop();
    // bw >= 0
    ASSERT_GE(bw, 0);
}

TEST(testTest, iperfFailTest) {
    MFollower f;
    // test with iperf
    float bw = f.testBandwidthIperf("a", 12345);
    // bw == -1 (failure)
    ASSERT_EQ(bw, -1);
}


TEST(testTest, estimateTest) {
    MFollower f;
    // start estimate
    int r = f.startEstimate();
    // r == 0 (success)
    ASSERT_EQ(r, 0);
    // test with estimate
    float bw = f.testBandwidthEstimate("localhost", "localhost", 5000);
    f.stop();
    // bw >= 0
    ASSERT_GE(bw, 0);    
}

TEST(testTest, estimateFailTest) {
    MFollower f;
    // test with estimate
    float bw = f.testBandwidthEstimate("a", "::1", 0);
    // bw == -1 (failure)
    ASSERT_EQ(bw, -1);
}
