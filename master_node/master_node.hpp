#ifndef MASTER_NODE_HPP_
#define MASTER_NODE_HPP_

#include "inode.hpp"
#include "master_connections.hpp"
#include "server.hpp"

class MasterNode : public INode{
public:
    MasterNode(int nThreads);
    ~MasterNode();

    void start();
    void stop();

    Storage* getStorage();
    IConnections* getConnections();
private:

    void timerFun();

    std::thread timerThread;

    unsigned int timer;

    bool running;

    Storage storage;
    MasterConnections connections;
    Server server;
};

#endif