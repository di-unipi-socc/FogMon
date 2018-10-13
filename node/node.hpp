#ifndef NODE_HPP_
#define NODE_HPP_
#include <string>
#include <thread>

#include "connections.hpp"
#include "storage.hpp"

class Node {
private:
    int timerReport;
    int lastReport;

    //ip:port of the server node 
    string ipS;
    
    //port of the listener
    uint16_t portC;

    int timerPing;
    int timerbandwidth;

    int lastPing;
    int lastBandwidth;

    bool running;

    thread listenerThread;
    thread timerThread;

    Connections connections;
    
    int numThreads;

    //fd to wake up the listener thread during poll
    int efd;

public:

    Storage storage;


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

    //listener for incoming connections
    void listener();
};

#endif















































































