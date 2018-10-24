#ifndef NODE_HPP_
#define NODE_HPP_

#include "inode.hpp"
#include "connections.hpp"
#include "server.hpp"

class Node : public INode{
private:
    int timerReport;

    //ip:port of the server node 
    std::string ipS;

    int timeTimerTest;

    bool running;

    std::thread timerThread;
    std::thread timerTestThread;
    Server server;

    Connections connections;
public:
    Node(std::string ip, int nThreads);
    ~Node();
    //start listener for incoming ping and directions
    void start();
    //stop everything
    void stop();

    //test the Bandwidth with another node
    void testBandwidth(std::string ip);
    //test the latency with another node
    void testPing(std::string ip);

    //get the hardware of this node
    void getHardware();

    //timer for hearthbeat
    void timer();

    //timer for latency and bandwidth tests
    void TestTimer();

    IConnections* getConnections();
};

#endif















































































