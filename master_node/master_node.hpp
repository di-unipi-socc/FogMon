#ifndef MASTER_NODE_HPP_
#define MASTER_NODE_HPP_

#include "inode.hpp"
#include "master_connections.hpp"
#include "server.hpp"

class MasterNode : public iNode{
    MasterNode(int nThreads);
    ~MasterNode();

    void start();
    void stop();

    Storage* getStorage();
    iConnections* getConnections();
private:

    Storage storage;
    MasterConnections connections;
    Server server;
};

#endif