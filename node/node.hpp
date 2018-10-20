#ifndef NODE_HPP_
#define NODE_HPP_

#include "inode.hpp"
#include "connections.hpp"
#include "server.hpp"

class Node : public INode{
private:
    int timerReport;
    int lastReport;

    //ip:port of the server node 
    std::string ipS;

    int timerPing;
    int timerbandwidth;

    int lastPing;
    int lastBandwidth;

    bool running;

    std::thread timerThread;
    Server server;

    Connections connections;
public:
    Node(int nThreads);
    ~Node();
    //start listener for incoming ping and directions
    void start();
    //stop everything
    void stop();

    //send report
    void report();

    //test the Bandwidth with another node
    void testBandwidth();
    //test the latency with another node
    void testPing();

    //get the hardware of this node
    void getHardware();

    //timer for latency and bandwidth tests
    void timer();

    IConnections* getConnections();
};

#endif















































































