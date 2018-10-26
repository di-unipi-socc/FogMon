#ifndef MASTER_NODE_HPP_
#define MASTER_NODE_HPP_

#include "inode.hpp"
#include "master_connections.hpp"
#include "master_storage.hpp"
#include "server.hpp"

class MasterNode : public INode{
public:
    MasterNode(int nThreads);
    ~MasterNode();

    void start();
    void stop();

    IConnections* getConnections();
private:

    void timerFun();

    std::thread timerThread;

    unsigned int timer;

    bool running;

    MasterConnections connections;
    Server server;

    void setMyIp(std::string ip) {}
    std::string getMyIp() { return ""; }

    Server* getServer();
};

#endif