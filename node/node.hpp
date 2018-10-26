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

    //ip to differentiate between other nodes in the list and self
    std::string myIp;

    int timeTimerTest;

    bool running;

    std::thread timerThread;
    std::thread timerTestThread;
    Server server;

    Connections connections;

    //timer for hearthbeat
    void timer();

    //timer for latency and bandwidth tests
    void TestTimer();
    
    //test the Bandwidth with another node
    void testBandwidth(std::string ip, int port= -1);
    //test the latency with another node
    void testPing(std::string ip);

public:
    Node(std::string ip, int nThreads);
    ~Node();

    //start listener for incoming ping and directions
    void start();
    //stop everything
    void stop();

    //get the hardware of this node
    void getHardware();

    IConnections* getConnections();

    void setMyIp(std::string ip);
    std::string getMyIp();

    Server* getServer();

    int startIperf();
};

#endif















































































